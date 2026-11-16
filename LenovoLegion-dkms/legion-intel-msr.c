// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-intel-msr.h"

#include <asm/cpu_device_id.h>
#include <asm/intel-family.h>
#include <asm/msr.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/version.h>

/*
 * OC mailbox (MSR 0x150), common for the package: every command is done on CPU 0 from the process context (an IPI
 * per MSR access, no busy wait with the interrupts disabled), serialized by priv->lock.
 * [63] busy (set to start a command, cleared when done), [47:40] plane, [39:32] command,
 * [31:21] voltage offset (11-bit signed, 1/1024 V)
 */
#define MSR_OC_MAILBOX 0x150
#define MSR_OC_MAILBOX_BUSY BIT_ULL(63)
#define MSR_OC_MAILBOX_CMD_READ_OFFSET 0x10
#define MSR_OC_MAILBOX_CMD_WRITE_OFFSET 0x11
#define MSR_OC_MAILBOX_POLLS 100 /* 10..20 us each */

#define MSR_OC_MAILBOX_CPU 0

/*
 * Offset in uV to the format of the mailbox: 11-bit signed integer in 1/1024 V units (two's complement, truncated
 * toward zero), written to bits [31:21]
 */
static u32 uv_to_msr(int uv)
{
	const s32 offset_units = (uv * 1024) / 1000000;

	return (u32)offset_units & 0x7FF;
}

/*
 * Convert MSR format back to microvolts
 * MSR format: 11-bit signed integer in 1/1024 V units, bits [31:21] of the mailbox
 */
static int msr_to_uv(const u32 msr_value)
{
	const s32 offset_units = sign_extend32((msr_value >> 21) & 0x7FF, 10);

	/* 1/1024 V units to uV */
	return (offset_units * 1000000) / 1024;
}

/*
 * Offset within the limits of the plane
 */
static int validate_offset(const struct legion_intel_msr_private *priv,
			   enum legion_intel_msr_plane plane, int offset_uv)
{
	if (plane < 0 || plane >= LEGION_INTEL_MSR_PLANES)
		return -EINVAL;

	if (offset_uv < priv->plane_limits[plane].min_uv ||
	    offset_uv > priv->plane_limits[plane].max_uv)
		return -EINVAL;

	return 0;
}

static int mailbox_read(u64 *value)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
	return rdmsrq_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#else
	return rdmsrl_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#endif
}

static int mailbox_write(const u64 value)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
	return wrmsrq_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#else
	return wrmsrl_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#endif
}

/*
 * Wait until the mailbox clears the busy bit, the last read value is returned in value
 */
static int mailbox_wait_ready(u64 *value)
{
	for (int poll = 0; poll < MSR_OC_MAILBOX_POLLS; poll++) {
		const int err = mailbox_read(value);

		if (err)
			return err;

		if (!(*value & MSR_OC_MAILBOX_BUSY))
			return 0;

		usleep_range(10, 20);
	}

	return -ETIMEDOUT;
}

/*
 * One command: the previous command must be finished (one command at a time), the result is valid when the
 * mailbox clears the busy bit
 */
static int mailbox_command(const int plane, const u8 command,
			   const u32 offset_encoded, u64 *value)
{
	const u64 msr_val = MSR_OC_MAILBOX_BUSY | ((u64)(plane & 0xFF) << 40) |
			    ((u64)command << 32) |
			    ((u64)(offset_encoded & 0x7FF) << 21);
	int err = mailbox_wait_ready(value);

	if (err)
		return err;

	err = mailbox_write(msr_val);
	if (err)
		return err;

	return mailbox_wait_ready(value);
}

/*
 * Core Ultra CPUs (Meteor Lake, Arrow Lake) accept but silently ignore the writes of the uncore and analog I/O
 * offsets (the planes can be read), Raptor Lake supports the writes of all planes
 */
static const struct x86_cpu_id legion_intel_msr_no_uncore_write[] = {
	X86_MATCH_VFM(INTEL_METEORLAKE, NULL),
	X86_MATCH_VFM(INTEL_METEORLAKE_L, NULL),
	X86_MATCH_VFM(INTEL_ARROWLAKE_H, NULL),
	X86_MATCH_VFM(INTEL_ARROWLAKE, NULL),
	X86_MATCH_VFM(INTEL_ARROWLAKE_U, NULL),
	{}
};

static void read_voltage_limits(struct legion_intel_msr_private *priv)
{
	for (int i = 0; i < LEGION_INTEL_MSR_PLANES; i++) {
		priv->plane_limits[i].min_uv = -LEGION_INTEL_MSR_MAX_UNDERVOLT_UV;
		priv->plane_limits[i].max_uv = LEGION_INTEL_MSR_MAX_OVERVOLT_UV;
		priv->plane_limits[i].supported = true;
	}

	if (x86_match_cpu(legion_intel_msr_no_uncore_write)) {
		priv->plane_limits[LEGION_INTEL_MSR_UNCORE].supported = false;
		priv->plane_limits[LEGION_INTEL_MSR_ANALOGIO].supported = false;
	}
}

/*
 * The OC mailbox (MSR 0x150) is available on Intel CPUs only, it does not exist when its read faults
 */
static int check_msr_availability(void)
{
	u64 value = 0;

	if (boot_cpu_data.x86_vendor != X86_VENDOR_INTEL ||
	    boot_cpu_data.x86 < 6)
		return -ENODEV;

	if (mailbox_read(&value))
		return -ENODEV;

	return 0;
}

/**
 * legion_intel_msr_read_offset() - Read the voltage offset of a plane
 * @priv: State of the mailbox.
 * @plane: The plane.
 * @offset_uv: The offset in uV.
 *
 * Context: Takes priv->lock.
 * Return: 0 or an error of the mailbox.
 */
int legion_intel_msr_read_offset(struct legion_intel_msr_private *priv,
				 enum legion_intel_msr_plane plane,
				 int *offset_uv)
{
	u64 value = 0;
	int err;

	if (plane < 0 || plane >= LEGION_INTEL_MSR_PLANES)
		return -EINVAL;

	guard(mutex)(&priv->lock);

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_READ_OFFSET, 0, &value);
	if (err)
		return err;

	*offset_uv = msr_to_uv((u32)value);

	return 0;
}

/**
 * legion_intel_msr_write_offset() - Write the voltage offset of a plane and verify it
 * @priv: State of the mailbox.
 * @plane: The plane.
 * @offset_uv: The offset in uV.
 *
 * The offset of the plane is common for the whole package (the same as intel-undervolt): one command on CPU 0.
 * The mailbox accepts the command also when the offset is locked (BIOS undervolt protection, OC lock) and
 * ignores it silently: the offset is read back, a different value is an error.
 *
 * Context: Takes priv->lock.
 * Return: 0, -EINVAL out of the limits, -EOPNOTSUPP for a plane which can not be written, -EIO for an offset
 * ignored by the CPU or an error of the mailbox.
 */
int legion_intel_msr_write_offset(struct legion_intel_msr_private *priv,
				  enum legion_intel_msr_plane plane,
				  int offset_uv)
{
	const u32 offset_encoded = uv_to_msr(offset_uv);
	u64 value = 0;
	u32 read_back;
	int err;

	guard(mutex)(&priv->lock);

	/* Validate plane first, it is used as index below */
	err = validate_offset(priv, plane, offset_uv);
	if (err)
		return err;

	if (!priv->plane_limits[plane].supported)
		return -EOPNOTSUPP;

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_WRITE_OFFSET,
			      offset_encoded, &value);
	if (err)
		return err;

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_READ_OFFSET, 0, &value);
	if (err)
		return err;

	read_back = (u32)(value >> 21) & 0x7FF;
	if (read_back != (offset_encoded & 0x7FF)) {
		pr_warn_ratelimited(
			"legion-intel-msr: offset %d uV of plane %d was not applied (read back %d uV), undervolting is probably locked in the BIOS\n",
			offset_uv, plane, msr_to_uv(read_back << 21));
		return -EIO;
	}

	return 0;
}

/**
 * legion_intel_msr_init() - Check the OC mailbox and set the limits of the planes
 * @priv: State of the mailbox.
 *
 * Return: 0 or -ENODEV without the OC mailbox.
 */
int legion_intel_msr_init(struct legion_intel_msr_private *priv)
{
	int err;

	mutex_init(&priv->lock);

	read_voltage_limits(priv);

	err = check_msr_availability();
	if (err)
		mutex_destroy(&priv->lock);

	return err;
}

void legion_intel_msr_exit(struct legion_intel_msr_private *priv)
{
	mutex_destroy(&priv->lock);
}
