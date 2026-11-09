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

/*
 * Events of the changed offsets (EVENT_TYPE of the uevent), the same values as before the module was split
 * from lenovo_legion (range 2001 - 2005 of the legion events)
 */
enum legion_intel_msr_events_type {
	LENOVO_INTEL_MSR_PLANE_CPU = 2001,
	LENOVO_INTEL_MSR_PLANE_CACHE = 2002,
	LENOVO_INTEL_MSR_PLANE_GPU = 2003,
	LENOVO_INTEL_MSR_PLANE_UNCORE = 2004,
	LENOVO_INTEL_MSR_PLANE_ANALOGIO = 2005
};

struct legion_intel_msr_sysfs_private {
	int ida_id;
	struct device *dev;
};

/*
 * Data of the module (one instance, independent of the notebook)
 */
struct legion_intel_msr_data {
	struct legion_intel_msr_private intel_msr_private;
	struct legion_intel_msr_sysfs_private intel_msr_sysfs_private;
};

#endif /* LEGION_INTEL_MSR_SYSFS_H_ */
