/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the lenovo-wmi-events driver of the Linux kernel (drivers/platform/x86/lenovo/wmi-events.h),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */

#ifndef LEGION_WMI_EVENTS_H_
#define LEGION_WMI_EVENTS_H_

struct device;
struct notifier_block;

int legion_wmi_events_register_notifier(struct notifier_block *nb);
int legion_wmi_events_unregister_notifier(struct notifier_block *nb);
int devm_legion_wmi_events_register_notifier(struct device *dev,
					     struct notifier_block *nb);

int legion_wmi_events_driver_init(void);
void legion_wmi_events_driver_exit(void);

#endif /* LEGION_WMI_EVENTS_H_ */
