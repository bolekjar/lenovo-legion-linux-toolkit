// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-intel-msr.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <asm/msr.h>
#include <linux/smp.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/cpu.h>
#include <asm/cpu_device_id.h>

#define MSR_VOLTAGE_OFFSET 0x150
#define MSR_OC_MAILBOX 0x150
#define MSR_OC_MAILBOX_CMD_READ_VOLTAGE_LIMIT 0x1A

#define MSR_OC_MAILBOX_BUSY ((u64)1 << 63)
#define MSR_OC_MAILBOX_TIMEOUT_US 1000

/*
 * Read voltage offset from specific plane on current CPU
 */
struct read_msr_data {
	int plane;
	u64 result;
	int error;
};

/*
 * Write voltage offset to specific plane
 */
struct write_msr_data {
	int plane;
	int offset_uv;
	int error;
	u32 read_back; // offset read after the write (11-bit MSR format)
};

/*
 * Convert microvolts offset to MSR format
 * Format: 11-bit signed integer in 1/1024 V units (two's complement), written to bits [31:21] of the mailbox
 */
static u32 uv_to_msr(int uv)
{
	// Convert uV to 1/1024V units (1 mV = 1.024 units), truncated toward zero
	const s32 offset_units = (uv * 1024) / 1000000;

	// Two's complement, 11 bits
	return (u32)offset_units & 0x7FF;
}

/*
 * Convert MSR format back to microvolts
 * MSR format: 11-bit signed integer in 1/1024 V units, bits [31:21] of the mailbox
 */
static int msr_to_uv(const u32 msr_value)
{
	s32 offset_units = 0;

	// Extract 11-bit value from bits [31:21]
	const u32 raw_value = (msr_value >> 21) & 0x7FF;

	// Sign extend from 11 bits to 32 bits
	if (raw_value & 0x400) { // Check bit 10 (sign bit)
		// Negative value - sign extend
		offset_units = (s32)(raw_value | 0xFFFFF800);
	} else {
		// Positive value
		offset_units = (s32)raw_value;
	}

	// Convert from 1/1024V units to uV
	return (offset_units * 1000000) / 1024;
}

/*
 * Validate voltage offset against per-plane limits
 */
static ssize_t
validate_offset(const struct legion_intel_msr_private *intel_msr_private,
		const int plane, const int offset_uv)
{
	if (plane < 0 || plane >= NUM_VOLTAGE_PLANES)
		return -EINVAL;

	if (offset_uv <
	    -intel_msr_private->plane_limits[plane].max_undervolt_uv) {
		return -EINVAL;
	}

	if (offset_uv > intel_msr_private->plane_limits[plane].max_overvolt_uv)
		return -EINVAL;
	return 0;
}

/*
 * Wait until the mailbox clears the busy bit, the last read value is returned in value.
 * Runs in IPI context (smp_call_function_single), busy wait only.
 */
static int wait_mailbox_ready(u64 *value)
{
	u32 low = 0, high = 0;

	for (int us = 0; us <= MSR_OC_MAILBOX_TIMEOUT_US; us++) {
		const int err = rdmsr_safe(MSR_OC_MAILBOX, &low, &high);

		if (err)
			return err;

		*value = ((u64)high << 32) | low;

		if (!(*value & MSR_OC_MAILBOX_BUSY))
			return 0;

		udelay(1);
	}

	return -ETIMEDOUT;
}

/*
 * Read the voltage offset of a plane through the mailbox (command 0x10), the offset is in bits [31:21] of value.
 * Runs in IPI context.
 */
static int read_voltage_offset_raw(const int plane, u64 *value)
{
	const u64 msr_val = MSR_OC_MAILBOX_BUSY | // Busy bit
			    ((u64)(plane & 0xFF) << 40) | // Domain
			    ((u64)0x10 << 32); // Read command

	// Previous command (e.g. write of the offset) must be finished
	int err = wait_mailbox_ready(value);

	if (err)
		return err;

	err = wrmsr_safe(MSR_OC_MAILBOX, (u32)msr_val, (u32)(msr_val >> 32));
	if (err)
		return err;

	// Result is valid when the mailbox clears the busy bit
	return wait_mailbox_ready(value);
}

/*
 * Write voltage offset to specific plane on a CPU
 */
static void write_voltage_offset_on_cpu(void *info)
{
	struct write_msr_data *data = info;
	u64 value = 0;
	int err;

	const u32 offset_encoded = uv_to_msr(data->offset_uv);

	// MSR 0x150 format for voltage offset (OC Mailbox):
	// Based on VoltageShift and intel-undervolt implementations:
	// [63]    = Busy bit (set to 1 to initiate command)
	// [47:40] = Domain/Plane ID
	// [39:32] = Command (0x11 = write voltage offset)
	// [31:21] = Voltage offset (11-bit signed, two's complement, in 1/1024V units)
	// [20:0]  = Other fields (ratio, etc - set to 0 for voltage offset)
	const u64 msr_val =
		MSR_OC_MAILBOX_BUSY | // Busy bit
		((u64)(data->plane & 0xFF) << 40) | // Domain at bits [47:40]
		((u64)0x11 << 32) | // Command at bits [39:32]
		((u64)(offset_encoded & 0x7FF)
		 << 21); // Voltage offset at bits [31:21]

	// Previous command must be finished, the mailbox is processing one command at a time
	err = wait_mailbox_ready(&value);
	if (err) {
		data->error = err;
		return;
	}

	err = wrmsr_safe(MSR_VOLTAGE_OFFSET, (const u32)msr_val,
			 (const u32)(msr_val >> 32));
	if (err) {
		data->error = err;
		return;
	}

	// Offset is applied when the mailbox clears the busy bit
	err = wait_mailbox_ready(&value);
	if (err) {
		data->error = err;
		return;
	}

	// The mailbox accepts the command also when the offset is locked (BIOS undervolt protection, OC lock):
	// the write is ignored silently. The offset is read back, a different value is an error.
	err = read_voltage_offset_raw(data->plane, &value);
	if (err) {
		data->error = err;
		return;
	}

	data->read_back = (u32)(value >> 21) & 0x7FF;
	data->error = data->read_back == (offset_encoded & 0x7FF) ? 0 : -EIO;
}

static void read_voltage_offset_on_cpu(void *info)
{
	struct read_msr_data *data = info;
	u64 value = 0;

	data->error = read_voltage_offset_raw(data->plane, &value);

	if (!data->error)
		data->result = value;
}

/*
 * Read voltage limits for all planes
 * Must be called with intel_msr_private->lock held or during initialization
 */
static void
read_voltage_limits(struct legion_intel_msr_private *intel_msr_private)
{
	for (int i = 0; i < NUM_VOLTAGE_PLANES; i++) {
		// Mark all planes as write-supported by default
		intel_msr_private->plane_limits[i].write_supported = 1;
	}

	// Core Ultra CPUs (Arrow Lake: 0xC5, 0xC6, 0xB5 and Meteor Lake: 0xAA, 0xAC) don't support
	// uncore and analogio voltage offset writes. These planes can be read but writes are
	// silently ignored by hardware.
	// Raptor Lake (0xB7, 0xBA, 0xBF) still supports all plane writes.
	if (boot_cpu_data.x86_model == 0xAA ||
	    boot_cpu_data.x86_model == 0xAC || // Meteor Lake
	    boot_cpu_data.x86_model == 0xC5 ||
	    boot_cpu_data.x86_model == 0xC6 || // Arrow Lake
	    boot_cpu_data.x86_model == 0xB5) { // Arrow Lake U
		intel_msr_private->plane_limits[PLANE_UNCORE].write_supported =
			0;
		intel_msr_private->plane_limits[PLANE_ANALOGIO].write_supported =
			0;
	}
}

/*
 * Check if MSR 0x150 (OC Mailbox) is available on this CPU
 */
static int legion_intel_msr_check_msr_availability(
	struct legion_intel_msr_private *intel_msr_private)
{
	// Check if we're on an Intel CPU
	if (boot_cpu_data.x86_vendor != X86_VENDOR_INTEL)
		return -ENODEV;

	// Check minimum CPU family (should be 6 for Core series)
	if (boot_cpu_data.x86 < 6)
		return -ENODEV;

	// Try to read MSR 0x150 to verify it exists
#if LINUX_VERSION_CODE >= KERNEL_VERSION(7, 2, 0)
	u64 val = 0;
	int err = rdmsrq_safe_on_cpu(0, MSR_OC_MAILBOX, &val);

	if (err)
		return -ENODEV;
#else
	u32 low = 0, high = 0;
	int err = rdmsr_safe_on_cpu(0, MSR_OC_MAILBOX, &low, &high);

	if (err)
		return -ENODEV;
#endif

	read_voltage_limits(intel_msr_private);

	return 0;
}

ssize_t legion_intel_msr_offset_read_show(
	struct legion_intel_msr_private *intel_msr_private, const int plane,
	int *offset_uv)
{
	struct read_msr_data data;

	data.plane = plane;
	data.error = -1;

	guard(mutex)
		(&intel_msr_private->lock);

	smp_call_function_single(0, read_voltage_offset_on_cpu, &data, 1);

	if (data.error)
		return data.error;

	*offset_uv = msr_to_uv((u32)data.result);

	return 0;
}

/*
 * Apply voltage offset to all CPUs for a given plane
 */
ssize_t legion_intel_msr_apply_voltage_offset(
	struct legion_intel_msr_private *intel_msr_private, const int plane,
	const int offset_uv)
{
	struct write_msr_data data = {
		.plane = plane,
		.offset_uv = offset_uv,
		.error = 0,
	};

	guard(mutex)
		(&intel_msr_private->lock);

	// Validate plane first, it is used as index below
	const ssize_t ret =
		validate_offset(intel_msr_private, data.plane, data.offset_uv);
	if (ret < 0)
		return ret;

	// Check if this plane supports writes
	if (!intel_msr_private->plane_limits[plane].write_supported)
		return -EOPNOTSUPP; // Operation not supported

	// Offset of the plane is common for the whole package (the same as intel-undervolt), one command on CPU 0.
	// Parallel commands from all CPUs could overwrite each other in the mailbox.
	smp_call_function_single(0, write_voltage_offset_on_cpu, &data, 1);

	if (data.error == -EIO) {
		pr_warn_ratelimited(
			"legion-intel-msr: offset %d uV of plane %d was not applied (read back %d uV), undervolting is probably locked in the BIOS\n",
			offset_uv, plane, msr_to_uv(data.read_back << 21));
	}

	return data.error;
}

int legion_intel_msr_init(struct legion_intel_msr_private *intel_msr_private)
{
	mutex_init(&intel_msr_private->lock);

	/* Initialize with defaults first to ensure safe reads if init fails */
	for (int i = 0; i < NUM_VOLTAGE_PLANES; i++) {
		intel_msr_private->plane_limits[i].max_undervolt_uv =
			DEFAULT_MAX_UNDERVOLT_UV;
		intel_msr_private->plane_limits[i].max_overvolt_uv =
			DEFAULT_MAX_OVERVOLT_UV;
	}

	return legion_intel_msr_check_msr_availability(intel_msr_private);
}

void legion_intel_msr_exit(struct legion_intel_msr_private *intel_msr_private)
{
	mutex_destroy(&intel_msr_private->lock);
}
