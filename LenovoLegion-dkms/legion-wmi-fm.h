// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_FM_H_
#define LEGION_WMI_FM_H_

#include "legion-wmi-gamezone.h"

#include <linux/notifier.h>
#include <linux/hashtable.h>

struct component_master_ops;
struct wmi_device;
struct kset;

#define FAN_MAX 20

enum fm_events_type {
	LEGION_WMI_FM_GET_CPU_FAN_MAX_SPEED = 0,
	LEGION_WMI_FM_GET_GPU_FAN_MAX_SPEED = 1,
	LEGION_WMI_FM_GET_SYS_FAN_MAX_SPEED = 2
};

struct fan_max_speed {
	int fan_id;
	int sensor_id;
	int max_speed;
};

struct legion_wmi_fm_priv {
	struct component_master_ops 	*ops;
	struct wmi_device 				*wdev;

	struct notifier_block 			hwmon_nb;   /* hwmon*/

	struct legion_wmi_ftable_list 	*ft_list; 					/* only valid after ft bind */
	struct fan_max_speed			 fan_max_speeds[FAN_MAX]; 	/* only valid after ft bind */

	//SysFs
	struct device *fw_attr_dev;
	struct kset   *fw_attr_kset;
	int 		   ida_id;
	int 		   smart_fan_version;

	int 		   fan_cpu_max_speed;
	int 		   fan_gpu_max_speed;
	int 		   fan_sys_max_speed;

	/* Completion to signal when binding is done */
	struct completion bind_complete;
};

int legion_wmi_fm_notifier_call(void *data,enum gamezone_events_type gamezone_events_type);
int devm_lenovo_wmi_fm_register_notifier(struct device *dev,struct notifier_block *nb);

int  legion_wmi_fm_driver_init(struct device *parent);
void legion_wmi_fm_driver_exit(void);

#endif /* LEGION_WMI_FM_H_ */
