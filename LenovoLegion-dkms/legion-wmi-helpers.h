// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_HELPERS_H_
#define LEGION_WMI_HELPERS_H_

#include <linux/types.h>

struct wmi_device;

struct wmi_method_args_32 {
	u32 arg0;
	u32 arg1;
};

#define __LEGION_WMI_ATTR_RO(_name,_func)		          \
{                                                         \
	.attr = { .name = __stringify(_name), .mode = 0444 }, \
	.show = func_##_func##_show,                          \
}

#define __LEGION_WMI_ATTR_WO(_name,_func)		           \
{                                                          \
	.attr  = { .name = __stringify(_name), .mode = 0220 }, \
	.store = func_##_func##_store,                         \
}

#define __LEGION_WMI_ATTR_RW(_name,_func) \
	__ATTR(_name, 0644, func_##_func##_get, func_##_func##_set)


#define __LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(_name,_attr,_func) \
static ssize_t func_##_func##_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) \
{																												 \
	return sysfs_emit(buf, "%s\n", _attr);												 						 \
}																												 \
static struct kobj_attribute kobj_attr_##_func  = __LEGION_WMI_ATTR_RO(_name,_func);



int legion_wmi_dev_evaluate_int(struct wmi_device *wdev, u8 instance, u32 method_id,unsigned char *buf, size_t size, u32 *retval);
int legion_wmi_dev_evaluate_buffer(struct wmi_device *wdev, u8 instance, u32 method_id,unsigned char *buf, size_t size, u8 *retval,size_t max_retval_size);
int legion_wmi_dev_evaluate_string(struct wmi_device *wdev, u8 instance, u32 method_id, unsigned char *buf, size_t size, char *retval,size_t retval_size);

#endif /* LEGION_WMI_HELPERS_H_ */
