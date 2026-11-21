/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_CPU_GPU_H_
#define LEGION_WMI_CPU_GPU_H_

#include "legion-common.h"
#include "legion-wmi-mode-defaults.h"

#include <linux/mutex.h>
#include <linux/types.h>

struct attribute_group;
struct device;
struct ida;
struct kset;
struct wmi_device;

/* Field of a result not present */
#define LEGION_WMI_CPU_GPU_NONE -1

/**
 * struct legion_wmi_cpu_gpu_limit - A limit and the layout of its get method (binary MOF of the firmware)
 * @name: Name of the attribute (the same as of the other mode).
 * @display_name: Description.
 * @get_id: WMI method reading the limit.
 * @set_id: WMI method writing the limit.
 * @item_size: Size of a value of the result and of the argument of the set method (1 or 4 bytes).
 * @current_index: Index of the current value in the result.
 * @default_index: Index of the default value in the result, or LEGION_WMI_CPU_GPU_NONE.
 * @step_index: Index of the step in the result.
 * @min_index: Index of the minimum in the result.
 * @max_index: Index of the maximum in the result.
 * @default_method_index: Index of the default value in the result of the default method of the class (uint32 values),
 *                        or LEGION_WMI_CPU_GPU_NONE.
 * @event: EVENT_TYPE of the uevent after a write.
 * @mode_default: Default value of the power modes (LENOVO_DEFAULT_VALUE_IN_DIFFERENT_MODE_DATA), or
 *                LEGION_WMI_MODE_DEFAULT_NONE.
 */
struct legion_wmi_cpu_gpu_limit {
	const char *name;
	const char *display_name;
	u32 get_id;
	u32 set_id;
	u8 item_size;
	s8 current_index;
	s8 default_index;
	s8 step_index;
	s8 min_index;
	s8 max_index;
	s8 default_method_index;
	enum legion_events_type event;
	enum legion_wmi_mode_default mode_default;
};

/**
 * struct legion_wmi_cpu_gpu_class - A WMI class with limits
 * @base_path: Name of the firmware attributes device.
 * @limits: The limits.
 * @count: Number of the limits.
 * @default_method: WMI method reading the default values of the limits without them in their get method.
 * @ida: Numbers of the devices of the class.
 */
struct legion_wmi_cpu_gpu_class {
	const char *base_path;
	const struct legion_wmi_cpu_gpu_limit *limits;
	size_t count;
	u32 default_method;
	struct ida *ida;
};

/*
 * Values of a limit read from the firmware (CPU_GPU_FIELD_CURRENT ... _STEP) and the static files of the
 * firmware attributes
 */
enum legion_wmi_cpu_gpu_field {
	CPU_GPU_FIELD_CURRENT,
	CPU_GPU_FIELD_DEFAULT,
	CPU_GPU_FIELD_MIN,
	CPU_GPU_FIELD_MAX,
	CPU_GPU_FIELD_STEP,
	CPU_GPU_FIELD_SUPPORTED,
	CPU_GPU_FIELD_TYPE,
	CPU_GPU_FIELD_DISPLAY_NAME,
	CPU_GPU_FIELDS
};

/**
 * struct legion_wmi_cpu_gpu_priv - The driver data
 * @wdev: The WMI device.
 * @wmi_class: The class of the device.
 * @lock: Serializes the WMI methods.
 * @fw_attr_dev: The firmware attributes device.
 * @fw_attr_kset: Directory attributes of the device.
 * @groups: Groups of the supported limits (NULL name: not supported).
 * @ida_id: Number of the device.
 */
struct legion_wmi_cpu_gpu_priv {
	struct wmi_device *wdev;
	const struct legion_wmi_cpu_gpu_class *wmi_class;
	struct mutex lock; /* serializes the WMI methods */

	//SysFs
	struct device *fw_attr_dev;
	struct kset *fw_attr_kset;
	struct attribute_group *groups;
	int ida_id;
};

int legion_wmi_cpu_gpu_read(struct legion_wmi_cpu_gpu_priv *priv,
			   const struct legion_wmi_cpu_gpu_limit *limit,
			   enum legion_wmi_cpu_gpu_field field, u32 *value);
int legion_wmi_cpu_gpu_write(struct legion_wmi_cpu_gpu_priv *priv,
			    const struct legion_wmi_cpu_gpu_limit *limit,
			    u32 value);
int legion_wmi_cpu_gpu_mode_default(struct legion_wmi_cpu_gpu_priv *priv,
				    const struct legion_wmi_cpu_gpu_limit *limit,
				    u32 mode, u32 *value);
bool legion_wmi_cpu_gpu_supported(struct legion_wmi_cpu_gpu_priv *priv,
				 const struct legion_wmi_cpu_gpu_limit *limit);

int legion_wmi_cpu_gpu_driver_init(void);
void legion_wmi_cpu_gpu_driver_exit(void);

#endif /* LEGION_WMI_CPU_GPU_H_ */
