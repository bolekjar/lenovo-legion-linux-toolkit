// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_HWMON_H
#define LEGION_HWMON_H

#include "legion-wmi-other.h"

enum other_hwmon_events_type {
	LEGION_WMI_FM_GET_CAPABILITY_DATA,
	LEGION_WMI_FM_GET_CAPABILITY_VALUE
};

struct hwmon_capability {
	enum CapabilityID 			capability_id;
	void *	 					data;
};

struct device;
struct notifier_block;

int  legion_hwmon_other_register_notifier(struct device *dev,struct notifier_block *nb);
int  legion_hwmon_fm_register_notifier(struct device *dev,struct notifier_block *nb);

int  legion_hwmon_init(struct device *parent);
void legion_hwmon_exit(struct device *parent);

#endif // LEGION_HWMON_H
