// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Intel CPU voltage offsets (OC mailbox MSR 0x150), a module independent of the notebook (loaded by
 * /etc/modules-load.d/lenovo-legion.conf), refused on a CPU without the OC mailbox.
 *
 * The same structure as the firmware attributes of lenovo_legion, in the own class (the module does not depend on
 * lenovo_legion): /sys/class/legion-intel-msr/legion-intel-msr-0/attributes/<plane>_offset/{current_value,
 * default_value, min_value, max_value, scalar_increment, supported, type, display_name}, values in uV; a write sends
 * a uevent of the class device (EVENT_TYPE: legion_events_type of the plane, EVENT_VALUE: offset in uV).
 *
 * After a resume the offsets written by the driver are checked and written again when the CPU / firmware reset them.
 */

#include "legion-kernel-version.h"
#include "legion-common.h"
#include "legion-intel-msr-sysfs.h"
#include "legion-wmi-helpers.h"

#include <linux/device.h>
#include <linux/idr.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#define LEGION_INTEL_MSR_FW_ATTR_BASE_PATH "legion-intel-msr"

static DEFINE_IDA(legion_intel_msr_ida);

static struct legion_intel_msr_data *legion_intel_msr_data;

/*
 * Notify user space about an applied offset (EVENT_TYPE: plane, EVENT_VALUE: offset in uV)
 */
static void legion_intel_msr_send_event(struct legion_intel_msr_data *data,
					enum legion_events_type type,
					int value)
{
	char event_type[32];
	char event_value[32];
	char *envp[] = { event_type, event_value, NULL };

	snprintf(event_type, sizeof(event_type), "EVENT_TYPE=%u",
		 (unsigned int)type);
	snprintf(event_value, sizeof(event_value), "EVENT_VALUE=%d", value);

	if (kobject_uevent_env(&data->fw_attr_dev->kobj, KOBJ_CHANGE, envp))
		dev_err(data->fw_attr_dev,
			"Failed to send uevent for attribute change\n");
}

static struct legion_intel_msr_data *legion_intel_msr_priv(struct kobject *kobj)
{
	/* kobj is the kset "attributes", its parent the class device */
	return dev_get_drvdata(kobj_to_dev(kobj->parent));
}

static ssize_t legion_intel_msr_current_show(struct kobject *kobj, char *buf,
					     enum legion_intel_msr_plane plane)
{
	struct legion_intel_msr_data *data = legion_intel_msr_priv(kobj);
	int offset_uv = 0;
	int err;

	if (!data)
		return -ENODEV;

	err = legion_intel_msr_read_offset(&data->msr, plane, &offset_uv);
	if (err)
		return err;

	return sysfs_emit(buf, "%d\n", offset_uv);
}

static ssize_t legion_intel_msr_current_store(struct kobject *kobj,
					      const char *buf, size_t count,
					      enum legion_intel_msr_plane plane,
					      enum legion_events_type event)
{
	struct legion_intel_msr_data *data = legion_intel_msr_priv(kobj);
	int offset_uv = 0;
	int err;

	if (!data)
		return -ENODEV;

	err = kstrtoint(buf, 10, &offset_uv);
	if (err)
		return err;

	err = legion_intel_msr_write_offset(&data->msr, plane, offset_uv);
	if (err)
		return err;

	/* Notify only about a really applied value */
	legion_intel_msr_send_event(data, event, offset_uv);

	return count;
}

static const struct legion_intel_msr_plane_limits *
legion_intel_msr_limits(struct kobject *kobj, enum legion_intel_msr_plane plane)
{
	const struct legion_intel_msr_data *data = legion_intel_msr_priv(kobj);

	return data ? &data->msr.plane_limits[plane] : NULL;
}

static ssize_t legion_intel_msr_limit_show(struct kobject *kobj, char *buf,
					   enum legion_intel_msr_plane plane,
					   bool max)
{
	const struct legion_intel_msr_plane_limits *limits =
		legion_intel_msr_limits(kobj, plane);

	if (!limits)
		return -ENODEV;

	return sysfs_emit(buf, "%d\n", max ? limits->max_uv : limits->min_uv);
}

static ssize_t legion_intel_msr_supported_show(struct kobject *kobj, char *buf,
					       enum legion_intel_msr_plane plane)
{
	const struct legion_intel_msr_plane_limits *limits =
		legion_intel_msr_limits(kobj, plane);

	if (!limits)
		return -ENODEV;

	return sysfs_emit(buf, "%u\n", limits->supported ? 1 : 0);
}

/*
 * Group of a plane: current_value (uV, read/write), default_value, min_value, max_value, scalar_increment (uV),
 * supported, type, display_name
 */
#define LEGION_INTEL_MSR_PLANE(_name, _plane, _event, _label)                       \
	static ssize_t func_##_name##_current_get(struct kobject *kobj,            \
						  struct kobj_attribute *kattr,    \
						  char *buf)                       \
	{                                                                          \
		return legion_intel_msr_current_show(kobj, buf, _plane);           \
	}                                                                          \
	static ssize_t func_##_name##_current_set(struct kobject *kobj,            \
						  struct kobj_attribute *kattr,    \
						  const char *buf, size_t count)   \
	{                                                                          \
		return legion_intel_msr_current_store(kobj, buf, count, _plane,    \
						      _event);                     \
	}                                                                          \
	static struct kobj_attribute kobj_attr_##_name##_current =                 \
		LEGION_WMI_ATTR_RW(current_value, _name##_current);                \
	static ssize_t func_##_name##_min_show(struct kobject *kobj,               \
					       struct kobj_attribute *kattr,       \
					       char *buf)                          \
	{                                                                          \
		return legion_intel_msr_limit_show(kobj, buf, _plane, false);      \
	}                                                                          \
	static struct kobj_attribute kobj_attr_##_name##_min =                     \
		LEGION_WMI_ATTR_RO(min_value, _name##_min);                        \
	static ssize_t func_##_name##_max_show(struct kobject *kobj,               \
					       struct kobj_attribute *kattr,       \
					       char *buf)                          \
	{                                                                          \
		return legion_intel_msr_limit_show(kobj, buf, _plane, true);       \
	}                                                                          \
	static struct kobj_attribute kobj_attr_##_name##_max =                     \
		LEGION_WMI_ATTR_RO(max_value, _name##_max);                        \
	static ssize_t func_##_name##_supported_show(struct kobject *kobj,         \
						     struct kobj_attribute *kattr, \
						     char *buf)                    \
	{                                                                          \
		return legion_intel_msr_supported_show(kobj, buf, _plane);         \
	}                                                                          \
	static struct kobj_attribute kobj_attr_##_name##_supported =               \
		LEGION_WMI_ATTR_RO(supported, _name##_supported);                  \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(default_value, "0",                  \
					      _name##_default);                    \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(                                     \
		scalar_increment, __stringify(LEGION_INTEL_MSR_STEP_UV),           \
		_name##_increment);                                                \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(type, "integer", _name##_type);      \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name, _label,                \
					      _name##_display_name);               \
	static struct attribute *legion_intel_msr_##_name##_attrs[] = {            \
		&kobj_attr_##_name##_current.attr,                                 \
		&kobj_attr_##_name##_default.attr,                                 \
		&kobj_attr_##_name##_min.attr,                                     \
		&kobj_attr_##_name##_max.attr,                                     \
		&kobj_attr_##_name##_increment.attr,                               \
		&kobj_attr_##_name##_supported.attr,                               \
		&kobj_attr_##_name##_type.attr,                                    \
		&kobj_attr_##_name##_display_name.attr,                            \
		NULL                                                               \
	}

LEGION_INTEL_MSR_PLANE(cpu, LEGION_INTEL_MSR_CPU, LEGION_INTEL_MSR_PLANE_CPU,
		       "CPU core voltage offset");
LEGION_INTEL_MSR_PLANE(gpu, LEGION_INTEL_MSR_GPU, LEGION_INTEL_MSR_PLANE_GPU,
		       "Integrated GPU voltage offset");
LEGION_INTEL_MSR_PLANE(cache, LEGION_INTEL_MSR_CACHE,
		       LEGION_INTEL_MSR_PLANE_CACHE, "CPU cache voltage offset");
LEGION_INTEL_MSR_PLANE(uncore, LEGION_INTEL_MSR_UNCORE,
		       LEGION_INTEL_MSR_PLANE_UNCORE,
		       "System agent (uncore) voltage offset");
LEGION_INTEL_MSR_PLANE(analogio, LEGION_INTEL_MSR_ANALOGIO,
		       LEGION_INTEL_MSR_PLANE_ANALOGIO, "Analog I/O voltage offset");

static const struct attribute_group legion_intel_msr_groups[] = {
	{ .name = "cpu_offset", .attrs = legion_intel_msr_cpu_attrs },
	{ .name = "gpu_offset", .attrs = legion_intel_msr_gpu_attrs },
	{ .name = "cache_offset", .attrs = legion_intel_msr_cache_attrs },
	{ .name = "uncore_offset", .attrs = legion_intel_msr_uncore_attrs },
	{ .name = "analogio_offset", .attrs = legion_intel_msr_analogio_attrs },
};

/*
 * Events of the planes (index: enum legion_intel_msr_plane)
 */
static const enum legion_events_type legion_intel_msr_events[LEGION_INTEL_MSR_PLANES] = {
	[LEGION_INTEL_MSR_CPU] = LEGION_INTEL_MSR_PLANE_CPU,
	[LEGION_INTEL_MSR_GPU] = LEGION_INTEL_MSR_PLANE_GPU,
	[LEGION_INTEL_MSR_CACHE] = LEGION_INTEL_MSR_PLANE_CACHE,
	[LEGION_INTEL_MSR_UNCORE] = LEGION_INTEL_MSR_PLANE_UNCORE,
	[LEGION_INTEL_MSR_ANALOGIO] = LEGION_INTEL_MSR_PLANE_ANALOGIO,
};

/*
 * After a resume (suspend to RAM, hibernation): the offsets written by the driver are written again when they were
 * reset, user space gets the uevent of a restored offset
 */
static int legion_intel_msr_pm_notify(struct notifier_block *nb,
				      unsigned long action, void *data)
{
	struct legion_intel_msr_data *msr_data =
		container_of(nb, struct legion_intel_msr_data, pm_notifier);
	int offset_uv = 0;
	int ret;

	if (action != PM_POST_SUSPEND && action != PM_POST_HIBERNATION &&
	    action != PM_POST_RESTORE)
		return NOTIFY_DONE;

	dev_dbg(msr_data->fw_attr_dev, "checking the offsets after the resume\n");

	for (int plane = 0; plane < LEGION_INTEL_MSR_PLANES; plane++) {
		ret = legion_intel_msr_restore_offset(&msr_data->msr, plane,
						      &offset_uv);
		if (ret < 0) {
			dev_warn(msr_data->fw_attr_dev,
				 "offset of plane %d was reset by the resume and can not be restored: %d\n",
				 plane, ret);
		} else if (ret > 0) {
			dev_info(msr_data->fw_attr_dev,
				 "offset %d uV of plane %d restored after the resume\n",
				 offset_uv, plane);
			legion_intel_msr_send_event(msr_data,
						    legion_intel_msr_events[plane],
						    offset_uv);
		}
	}

	return NOTIFY_OK;
}

static const struct class legion_intel_msr_class = {
	.name = "legion-intel-msr",
};

/**
 * legion_intel_msr_fw_attr_add() - Register the device and its attributes
 * @data: Data of the module.
 *
 * Return: Either 0, or an error code.
 */
static int legion_intel_msr_fw_attr_add(struct legion_intel_msr_data *data)
{
	unsigned int i;
	int err;

	data->ida_id = ida_alloc(&legion_intel_msr_ida, GFP_KERNEL);
	if (data->ida_id < 0)
		return data->ida_id;

	data->fw_attr_dev = device_create(&legion_intel_msr_class, NULL,
					  MKDEV(0, 0), data, "%s-%u",
					  LEGION_INTEL_MSR_FW_ATTR_BASE_PATH,
					  data->ida_id);
	if (IS_ERR(data->fw_attr_dev)) {
		err = PTR_ERR(data->fw_attr_dev);
		goto err_free_ida;
	}

	data->fw_attr_kset = kset_create_and_add(LEGION_FW_ATTR_KSET_NAME, NULL,
						 &data->fw_attr_dev->kobj);
	if (!data->fw_attr_kset) {
		err = -ENOMEM;
		goto err_unregister_fw_attr_dev;
	}

	for (i = 0; i < ARRAY_SIZE(legion_intel_msr_groups); i++) {
		err = sysfs_create_group(&data->fw_attr_kset->kobj,
					 &legion_intel_msr_groups[i]);
		if (err)
			goto err_remove_groups;
	}

	return 0;

err_remove_groups:
	while (i--)
		sysfs_remove_group(&data->fw_attr_kset->kobj,
				   &legion_intel_msr_groups[i]);
	kset_unregister(data->fw_attr_kset);
err_unregister_fw_attr_dev:
	device_unregister(data->fw_attr_dev);
err_free_ida:
	ida_free(&legion_intel_msr_ida, data->ida_id);
	return err;
}

/**
 * legion_intel_msr_fw_attr_remove() - Unregister the device and its attributes
 * @data: Data of the module.
 *
 * Waits for the running sysfs operations, the mailbox is not used afterwards.
 */
static void legion_intel_msr_fw_attr_remove(struct legion_intel_msr_data *data)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(legion_intel_msr_groups); i++)
		sysfs_remove_group(&data->fw_attr_kset->kobj,
				   &legion_intel_msr_groups[i]);

	kset_unregister(data->fw_attr_kset);
	device_unregister(data->fw_attr_dev);
	ida_free(&legion_intel_msr_ida, data->ida_id);
}

static int __init legion_intel_msr_module_init(void)
{
	struct legion_intel_msr_data *data;
	int err;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	err = legion_intel_msr_init(&data->msr);
	if (err) {
		pr_info("legion-intel-msr: Intel OC mailbox (MSR 0x150) not available: %d\n",
			err);
		goto err_free;
	}

	err = class_register(&legion_intel_msr_class);
	if (err)
		goto err_msr_exit;

	err = legion_intel_msr_fw_attr_add(data);
	if (err)
		goto err_unregister_class;

	data->pm_notifier.notifier_call = legion_intel_msr_pm_notify;
	err = register_pm_notifier(&data->pm_notifier);
	if (err)
		goto err_fw_attr_remove;

	legion_intel_msr_data = data;

	return 0;

err_fw_attr_remove:
	legion_intel_msr_fw_attr_remove(data);

err_unregister_class:
	class_unregister(&legion_intel_msr_class);
err_msr_exit:
	legion_intel_msr_exit(&data->msr);
err_free:
	kfree(data);
	return err;
}

static void __exit legion_intel_msr_module_exit(void)
{
	struct legion_intel_msr_data *data = legion_intel_msr_data;

	unregister_pm_notifier(&data->pm_notifier);
	legion_intel_msr_fw_attr_remove(data);
	class_unregister(&legion_intel_msr_class);
	legion_intel_msr_exit(&data->msr);
	kfree(data);
}

module_init(legion_intel_msr_module_init);
module_exit(legion_intel_msr_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaroslav Bolek");
MODULE_DESCRIPTION(
	"Intel CPU voltage offsets (OC mailbox MSR 0x150), part of Lenovo Legion Linux Toolkit");
MODULE_VERSION(LEGION_VERSION);
