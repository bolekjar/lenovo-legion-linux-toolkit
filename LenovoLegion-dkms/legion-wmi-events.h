// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_EVENTS_H_
#define LEGION_WMI_EVENTS_H_

struct device;
struct notifier_block;

int legion_wmi_events_register_notifier(struct notifier_block *nb);
int legion_wmi_events_unregister_notifier(struct notifier_block *nb);
int devm_legion_wmi_events_register_notifier(struct device *dev,struct notifier_block *nb);

int  legion_wmi_events_driver_init(struct device *parent);
void legion_wmi_events_driver_exit(void);

#endif /* LEGION_WMI_EVENTS_H_ */
