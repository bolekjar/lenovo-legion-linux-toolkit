/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_INTEL_MSR_SYSFS_H_
#define LEGION_INTEL_MSR_SYSFS_H_

#include "legion-intel-msr.h"

#include <linux/device.h>
#include <linux/kobject.h>

/**
 * struct legion_intel_msr_data - Data of the module (one device, independent of the notebook)
 * @msr: State of the OC mailbox.
 * @fw_attr_dev: Device /sys/class/legion-intel-msr/legion-intel-msr-<id>.
 * @fw_attr_kset: Directory attributes of the device.
 * @ida_id: Number of the device.
 */
struct legion_intel_msr_data {
	struct legion_intel_msr_private msr;
	struct device *fw_attr_dev;
	struct kset *fw_attr_kset;
	int ida_id;
};

#endif /* LEGION_INTEL_MSR_SYSFS_H_ */
