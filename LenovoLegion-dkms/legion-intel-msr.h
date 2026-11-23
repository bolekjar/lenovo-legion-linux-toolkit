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
#include <linux/notifier.h>
#include <linux/types.h>

struct device;
struct kset;

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
 * Limits of the voltage offsets in uV: conservative -150 mV for all Intel generations (12th gen to Core Ultra), the
 * BIOS allows up to +-300 mV; an overvolt (up to +150 mV) only with the module parameter allow_overvolt
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
 * @written: The offset of the plane was written by the driver (restored after a resume).
 * @written_units: The written offset (1/1024 V units, 11 bits as in the mailbox).
 * @lock: Serializes the commands of the mailbox (one command at a time).
 */
struct legion_intel_msr_private {
	struct legion_intel_msr_plane_limits plane_limits[LEGION_INTEL_MSR_PLANES];
	bool written[LEGION_INTEL_MSR_PLANES];
	u32 written_units[LEGION_INTEL_MSR_PLANES];
	struct mutex lock;
};

/**
 * struct legion_intel_msr_data - Data of the module (one device, independent of the notebook)
 * @msr: State of the OC mailbox.
 * @fw_attr_dev: Device /sys/class/legion-intel-msr/legion-intel-msr-<id>.
 * @fw_attr_kset: Directory attributes of the device.
 * @ida_id: Number of the device.
 * @pm_notifier: Restores the offsets after a resume.
 */
struct legion_intel_msr_data {
	struct legion_intel_msr_private msr;
	struct device *fw_attr_dev;
	struct kset *fw_attr_kset;
	int ida_id;
	struct notifier_block pm_notifier;
};

int legion_intel_msr_write_offset(struct legion_intel_msr_private *priv,
				  enum legion_intel_msr_plane plane,
				  int offset_uv);
int legion_intel_msr_read_offset(struct legion_intel_msr_private *priv,
				 enum legion_intel_msr_plane plane,
				 int *offset_uv);
int legion_intel_msr_restore_offset(struct legion_intel_msr_private *priv,
				    enum legion_intel_msr_plane plane,
				    int *offset_uv);

int legion_intel_msr_init(struct legion_intel_msr_private *priv);
void legion_intel_msr_exit(struct legion_intel_msr_private *priv);

#endif /* LEGION_INTEL_MSR_H_ */
