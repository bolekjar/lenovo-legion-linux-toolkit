// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_INTEL_MSR_H_
#define LEGION_INTEL_MSR_H_

#include <linux/mutex.h>

// Voltage plane offsets within MSR 0x150
#define PLANE_CPU       0
#define PLANE_GPU       1
#define PLANE_CACHE     2
#define PLANE_UNCORE    3
#define PLANE_ANALOGIO  4
#define NUM_VOLTAGE_PLANES 5


// Default voltage offset limits in uV based on Intel documented specifications
// Conservative limits: ±150mV for safety (Intel allows up to ±300mV in BIOS)
// These safe values work across all Intel generations (12th gen through Core Ultra)
#define DEFAULT_MAX_UNDERVOLT_UV 150000  // 150mV undervolt
#define DEFAULT_MAX_OVERVOLT_UV 150000   // 150mV overvolt

struct legion_intel_voltage_limits {
	int max_undervolt_uv;
	int max_overvolt_uv;
	int write_supported;  // Whether this plane supports voltage writes
};


struct legion_intel_msr_private {
	struct legion_intel_voltage_limits plane_limits[NUM_VOLTAGE_PLANES];
	struct mutex lock;
};


int  legion_intel_msr_apply_voltage_offset(struct legion_intel_msr_private *intel_msr_private,int  plane, int offset_mv);
int  legion_intel_msr_offset_read_show(struct legion_intel_msr_private *intel_msr_private,int plane, int* offset_mv);

int  legion_intel_msr_init(struct legion_intel_msr_private *intel_msr_private);
void legion_intel_msr_exit(struct legion_intel_msr_private *intel_msr_private);

#endif /* LEGION_INTEL_MSR_H_ */
