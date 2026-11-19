/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_LEGACY_H_
#define LEGION_WMI_LEGACY_H_

#include "legion-common.h"

#include <linux/mutex.h>
#include <linux/types.h>

struct attribute_group;
struct device;
struct kset;
struct wmi_device;

/* Field of a result not present */
#define LEGION_WMI_LEGACY_NONE -1

/**
 * struct legion_wmi_legacy_limit - A limit and the layout of its get method (binary MOF of the firmware)
 * @name: Name of the attribute (the same as of the other mode).
 * @display_name: Description.
 * @get_id: WMI method reading the limit.
 * @set_id: WMI method writing the limit.
 * @item_size: Size of a value of the result and of the argument of the set method (1 or 4 bytes).
 * @current_index: Index of the current value in the result.
 * @default_index: Index of the default value in the result, or LEGION_WMI_LEGACY_NONE.
 * @step_index: Index of the step in the result.
 * @min_index: Index of the minimum in the result.
 * @max_index: Index of the maximum in the result.
 * @default_method_index: Index of the default value in the result of the default method of the class (uint32 values),
 *                        or LEGION_WMI_LEGACY_NONE.
 * @event: EVENT_TYPE of the uevent after a write.
 */
struct legion_wmi_legacy_limit {
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
};

/**
 * struct legion_wmi_legacy_class - A WMI class with limits
 * @base_path: Name of the firmware attributes device.
 * @limits: The limits.
 * @count: Number of the limits.
 * @default_method: WMI method reading the default values of the limits without them in their get method.
 */
struct legion_wmi_legacy_class {
	const char *base_path;
	const struct legion_wmi_legacy_limit *limits;
	size_t count;
	u32 default_method;
};

/*
 * Values of a limit read from the firmware (LEGACY_FIELD_CURRENT ... _STEP) and the static files of the
 * firmware attributes
 */
enum legion_wmi_legacy_field {
	LEGACY_FIELD_CURRENT,
	LEGACY_FIELD_DEFAULT,
	LEGACY_FIELD_MIN,
	LEGACY_FIELD_MAX,
	LEGACY_FIELD_STEP,
	LEGACY_FIELD_SUPPORTED,
	LEGACY_FIELD_TYPE,
	LEGACY_FIELD_DISPLAY_NAME,
	LEGACY_FIELDS
};

/**
 * struct legion_wmi_legacy_priv - The driver data
 * @wdev: The WMI device.
 * @wmi_class: The class of the device.
 * @lock: Serializes the WMI methods.
 * @fw_attr_dev: The firmware attributes device.
 * @fw_attr_kset: Directory attributes of the device.
 * @groups: Groups of the supported limits (NULL name: not supported).
 * @ida_id: Number of the device.
 */
struct legion_wmi_legacy_priv {
	struct wmi_device *wdev;
	const struct legion_wmi_legacy_class *wmi_class;
	struct mutex lock; /* serializes the WMI methods */

	//SysFs
	struct device *fw_attr_dev;
	struct kset *fw_attr_kset;
	struct attribute_group *groups;
	int ida_id;
};

int legion_wmi_legacy_read(struct legion_wmi_legacy_priv *priv,
			   const struct legion_wmi_legacy_limit *limit,
			   enum legion_wmi_legacy_field field, u32 *value);
int legion_wmi_legacy_write(struct legion_wmi_legacy_priv *priv,
			    const struct legion_wmi_legacy_limit *limit,
			    u32 value);
bool legion_wmi_legacy_supported(struct legion_wmi_legacy_priv *priv,
				 const struct legion_wmi_legacy_limit *limit);

bool legion_wmi_legacy_platform(struct device *dev, int *err);

int legion_wmi_legacy_driver_init(void);
void legion_wmi_legacy_driver_exit(void);

#endif /* LEGION_WMI_LEGACY_H_ */
