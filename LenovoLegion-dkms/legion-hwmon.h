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
 * Sources of the sensors: the other mode (temperatures, fan speeds, capability data 00) and the fan method (maximal
 * fan speeds) register their notifiers while their components are bound (not device managed: bind/unbind may repeat)
 */
int legion_hwmon_other_register_notifier(struct notifier_block *nb);
void legion_hwmon_other_unregister_notifier(struct notifier_block *nb);
int legion_hwmon_fm_register_notifier(struct notifier_block *nb);
void legion_hwmon_fm_unregister_notifier(struct notifier_block *nb);

/*
 * hwmon device "legion" of the other mode device (registered when its components are bound, ERR_PTR on error)
 */
struct legion_hwmon;

struct legion_hwmon *legion_hwmon_register(struct device *parent);
void legion_hwmon_unregister(struct legion_hwmon *hwmon);

#endif // LEGION_HWMON_H
