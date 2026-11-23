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


int legion_intel_msr_sysfs_init(struct legion_intel_msr_data *data);
void legion_intel_msr_sysfs_exit(struct legion_intel_msr_data *data);
void legion_intel_msr_sysfs_offset_restored(struct legion_intel_msr_data *data,
					    enum legion_intel_msr_plane plane,
					    int offset_uv);

#endif /* LEGION_INTEL_MSR_SYSFS_H_ */
