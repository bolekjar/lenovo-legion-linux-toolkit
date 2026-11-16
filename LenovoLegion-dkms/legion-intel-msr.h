/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_INTEL_MSR_H_
#define LEGION_INTEL_MSR_H_

#include <linux/mutex.h>

/*
 * Voltage planes of the OC mailbox (MSR 0x150)
 */
enum legion_intel_msr_plane {
	LEGION_INTEL_MSR_CPU = 0,
	LEGION_INTEL_MSR_GPU = 1,
	LEGION_INTEL_MSR_CACHE = 2,
	LEGION_INTEL_MSR_UNCORE = 3,
	LEGION_INTEL_MSR_ANALOGIO = 4,
	LEGION_INTEL_MSR_PLANES
};

/*
 * Limits of the voltage offsets in uV: conservative +-150 mV for all Intel generations (12th gen to Core Ultra),
 * the BIOS allows up to +-300 mV
 */
#define LEGION_INTEL_MSR_MAX_UNDERVOLT_UV 150000
#define LEGION_INTEL_MSR_MAX_OVERVOLT_UV 150000

/*
 * Step of the offset: 1/1024 V (977 uV)
 */
#define LEGION_INTEL_MSR_STEP_UV 977

/**
 * struct legion_intel_msr_plane_limits - Limits of a voltage plane
 * @min_uv: Minimal offset in uV (maximal undervolt, negative).
 * @max_uv: Maximal offset in uV (maximal overvolt).
 * @supported: The offset of the plane can be written.
 */
struct legion_intel_msr_plane_limits {
	int min_uv;
	int max_uv;
	bool supported;
};

/**
 * struct legion_intel_msr_private - State of the OC mailbox
 * @plane_limits: Limits of the planes.
 * @lock: Serializes the commands of the mailbox (one command at a time).
 */
struct legion_intel_msr_private {
	struct legion_intel_msr_plane_limits plane_limits[LEGION_INTEL_MSR_PLANES];
	struct mutex lock;
};

int legion_intel_msr_write_offset(struct legion_intel_msr_private *priv,
				  enum legion_intel_msr_plane plane,
				  int offset_uv);
int legion_intel_msr_read_offset(struct legion_intel_msr_private *priv,
				 enum legion_intel_msr_plane plane,
				 int *offset_uv);

int legion_intel_msr_init(struct legion_intel_msr_private *priv);
void legion_intel_msr_exit(struct legion_intel_msr_private *priv);

#endif /* LEGION_INTEL_MSR_H_ */
