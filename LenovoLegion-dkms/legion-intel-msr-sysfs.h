// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_INTEL_MSR_SYSFS_H_
#define LEGION_INTEL_MSR_SYSFS_H_


struct legion_intel_msr_sysfs_private {
	int 	ida_id;
	struct 	device *dev;
};

struct legion_data;

int  legion_intel_msr_sysfs_init(struct device *parent);
void legion_intel_msr_sysfs_exit(struct device *parent);

#endif /* LEGION_INTEL_MSR_SYSFS_H_ */
