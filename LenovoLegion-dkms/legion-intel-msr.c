// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-intel-msr.h"

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


/*
 * Read voltage offset from specific plane on current CPU
 */
struct read_msr_data {
    int plane;
    u64 result;
    int error;
};


/*
 * Convert microvolts offset to MSR format
 * Format: 21-bit signed integer in 1/1024 V units (two's complement)
 */
static u32 uv_to_msr(int uv)
{
    s32 offset_units;
    u32 result;

    // Convert mV to 1/1024V units
    // 1mV = 1.024 units of 1/1024V
    offset_units = (uv * 1024) / 1000000;

    // Handle two's complement for negative values
    // Mask to 21 bits
    if (offset_units < 0) {
        // Two's complement: invert and add 1, then mask to 21 bits
        result = (u32)offset_units & 0x1FFFFF;
    } else {
        result = (u32)offset_units & 0x1FFFFF;
    }

    return result;
}


/*
 * Convert MSR format back to microvolts
 * MSR format: 11-bit signed integer in 1/1024 V units
 */
static int msr_to_uv(u32 msr_value)
{
    s32 offset_units;

    // Extract 11-bit value from bits [31:21]
    u32 raw_value = (msr_value >> 21) & 0x7FF;

    // Sign extend from 11 bits to 32 bits
    if (raw_value & 0x400) {  // Check bit 10 (sign bit)
        // Negative value - sign extend
        offset_units = (s32)(raw_value | 0xFFFFF800);
    } else {
        // Positive value
        offset_units = (s32)raw_value;
    }

    // Convert from 1/1024V units to mV
    return (offset_units * 1000000) / 1024;
}


/*
 * Validate voltage offset against per-plane limits
 */
static int validate_offset(const struct legion_intel_msr_private *intel_msr_private,int plane, int offset_mv)
{
    if (plane < 0 || plane >= NUM_VOLTAGE_PLANES) {
        return -EINVAL;
    }

    if (offset_mv < -intel_msr_private->plane_limits[plane].max_undervolt_uv) {
        return -EINVAL;
    }

    if (offset_mv > intel_msr_private->plane_limits[plane].max_overvolt_uv) {
        return -EINVAL;
    }
    return 0;
}



/*
 * Write voltage offset to specific plane on a CPU
 */
static void write_voltage_offset_on_cpu(void *info)
{
    struct {
        int plane;
        int offset_uv;
    } *data = info;

    u64 msr_val;
    u32 offset_encoded;
    int err;

    offset_encoded = uv_to_msr(data->offset_uv);

    // MSR 0x150 format for voltage offset (OC Mailbox):
    // Based on VoltageShift and intel-undervolt implementations:
    // [63]    = Busy bit (set to 1 to initiate command)
    // [47:40] = Domain/Plane ID
    // [39:32] = Command (0x11 = write voltage offset)
    // [31:21] = Voltage offset (11-bit signed, two's complement, in 1/1024V units)
    // [20:0]  = Other fields (ratio, etc - set to 0 for voltage offset)

    msr_val = ((u64)1 << 63) |                           // Busy bit
              ((u64)(data->plane & 0xFF) << 40) |        // Domain at bits [47:40]
              ((u64)0x11 << 32) |                        // Command at bits [39:32]
              ((u64)(offset_encoded & 0x7FF) << 21);     // Voltage offset at bits [31:21]

    err = wrmsr_safe(MSR_VOLTAGE_OFFSET, (u32)msr_val, (u32)(msr_val >> 32));
}

static void read_voltage_offset_on_cpu(void *info)
{
    struct read_msr_data *data = info;
    u64 msr_val;
    u32 low, high;
    int err;

    // Construct read command for MSR 0x150
    // Command 0x10 = read voltage offset
    msr_val = ((u64)1 << 63) |                      // Busy bit
              ((u64)(data->plane & 0xFF) << 40) |   // Domain
              ((u64)0x10 << 32);                    // Read command

    // Write read command
    err = wrmsr_safe(MSR_OC_MAILBOX, (u32)msr_val, (u32)(msr_val >> 32));
    if (err) {
        data->error = err;
        return;
    }

    // Small delay for mailbox to process
    udelay(10);

    // Read result
    err = rdmsr_safe(MSR_OC_MAILBOX, &low, &high);
    if (err) {
        data->error = err;
        return;
    }

    data->result = ((u64)high << 32) | low;
    data->error = 0;
}

/*
 * Read voltage offset limits from OC mailbox for a specific plane
 * Returns 0 on success, negative error code on failure
 */
static int read_voltage_limits_for_plane(struct legion_intel_msr_private *intel_msr_private, int plane)
{
    // On both old and new CPUs, the 0x1A command either doesn't work or returns 0
    // Using documented safe defaults is more reliable than trying to read limits
    // The defaults are conservative values that work across all Intel generations
    // Return success - limits are set by caller using DEFAULT values
    return 0;
}

/*
* Read voltage limits for all planes
* Must be called with intel_msr_private->lock held or during initialization
*/
static void read_voltage_limits(struct legion_intel_msr_private *intel_msr_private)
{
    int i;

    for (i = 0; i < NUM_VOLTAGE_PLANES; i++)
    {
        // Mark all planes as write-supported by default
        intel_msr_private->plane_limits[i].write_supported = 1;
        
        if (read_voltage_limits_for_plane(intel_msr_private, i)) {
        	// Fall back to defaults if reading limits fails
        	intel_msr_private->plane_limits[i].max_undervolt_uv = DEFAULT_MAX_UNDERVOLT_UV;
        	intel_msr_private->plane_limits[i].max_overvolt_uv  = DEFAULT_MAX_OVERVOLT_UV;
        }
    }
    
    // Core Ultra CPUs (Arrow Lake: 0xC5, 0xC6, 0xB5 and Meteor Lake: 0xAA, 0xAC) don't support 
    // uncore and analogio voltage offset writes. These planes can be read but writes are 
    // silently ignored by hardware.
    // Raptor Lake (0xB7, 0xBA, 0xBF) still supports all plane writes.
    if (boot_cpu_data.x86_model == 0xAA || boot_cpu_data.x86_model == 0xAC ||  // Meteor Lake
        boot_cpu_data.x86_model == 0xC5 || boot_cpu_data.x86_model == 0xC6 ||  // Arrow Lake
        boot_cpu_data.x86_model == 0xB5) {  // Arrow Lake U
        intel_msr_private->plane_limits[PLANE_UNCORE].write_supported = 0;
        intel_msr_private->plane_limits[PLANE_ANALOGIO].write_supported = 0;
    }
}

/*
 * Check if MSR 0x150 (OC Mailbox) is available on this CPU
 */
static int legion_intel_msr_check_msr_availability(struct legion_intel_msr_private *intel_msr_private)
{
    u32 low, high;
    int err;

    // Check if we're on an Intel CPU
    if (boot_cpu_data.x86_vendor != X86_VENDOR_INTEL) {
        return -ENODEV;
    }

    // Check minimum CPU family (should be 6 for Core series)
    if (boot_cpu_data.x86 < 6) {
        return -ENODEV;
    }

    // Try to read MSR 0x150 to verify it exists
    err = rdmsr_safe_on_cpu(0, MSR_OC_MAILBOX, &low, &high);
    if (err) {
        return -ENODEV;
    }

    read_voltage_limits(intel_msr_private);


    return 0;
}


int legion_intel_msr_offset_read_show(struct legion_intel_msr_private *intel_msr_private,int plane, int* offset_uv)
{
    struct read_msr_data data;

    data.plane = plane;
    data.error = -1;

    guard(mutex)(&intel_msr_private->lock);


    smp_call_function_single(0, read_voltage_offset_on_cpu, &data, 1);

    if (data.error) {
        return data.error;
    }

    *offset_uv = msr_to_uv((u32)data.result);

    return 0;
}


/*
 * Apply voltage offset to all CPUs for a given plane
 */
int legion_intel_msr_apply_voltage_offset(struct legion_intel_msr_private *intel_msr_private, int plane, int offset_uv)
{
    struct {
        int plane;
        int offset_uv;
    } data;
    int ret;

    data.plane = plane;
    data.offset_uv = offset_uv;

    guard(mutex)(&intel_msr_private->lock);

    // Check if this plane supports writes
    if (!intel_msr_private->plane_limits[plane].write_supported) {
        return -EOPNOTSUPP;  // Operation not supported
    }

    ret = validate_offset(intel_msr_private, data.plane, data.offset_uv);
    if (ret < 0)
        return ret;

    // Execute on all CPUs (including 0 to allow reset)
    on_each_cpu(write_voltage_offset_on_cpu, &data, 1);

    return 0;
}

int  legion_intel_msr_init(struct legion_intel_msr_private *intel_msr_private) {

	mutex_init(&intel_msr_private->lock);

	/* Initialize with defaults first to ensure safe reads if init fails */
	for (int i = 0; i < NUM_VOLTAGE_PLANES; i++) {
		intel_msr_private->plane_limits[i].max_undervolt_uv = DEFAULT_MAX_UNDERVOLT_UV;
		intel_msr_private->plane_limits[i].max_overvolt_uv = DEFAULT_MAX_OVERVOLT_UV;
	}

	return legion_intel_msr_check_msr_availability(intel_msr_private);
}

void legion_intel_msr_exit(struct legion_intel_msr_private *intel_msr_private) {
	mutex_destroy(&intel_msr_private->lock);
}
