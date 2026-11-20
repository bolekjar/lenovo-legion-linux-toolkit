/* SPDX-License-Identifier: GPL-2.0 */
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
	enum CapabilityID capability_id;
	void *data;
};

struct device;
struct notifier_block;

/*
 * hwmon device "legion" of the source of the sensors (temperatures, fan speeds, capability data 00), kept by the
 * source: the other mode on the newer firmware, the fan method on the older one (ERR_PTR on error)
 */
struct legion_hwmon;

struct legion_hwmon *legion_hwmon_register(struct device *parent,
					   struct notifier_block *source);
void legion_hwmon_unregister(struct legion_hwmon *hwmon);

/*
 * Maximal fan speeds: the fan method registers its notifier while its components are bound
 */
int legion_hwmon_fm_register_notifier(struct notifier_block *nb);
void legion_hwmon_fm_unregister_notifier(struct notifier_block *nb);

#endif // LEGION_HWMON_H
