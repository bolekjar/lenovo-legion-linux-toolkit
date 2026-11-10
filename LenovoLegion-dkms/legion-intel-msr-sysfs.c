// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-kernel-version.h"
#include "legion-intel-msr-sysfs.h"

#include <linux/device.h>
#include <linux/module.h>
#include <linux/sysfs.h>

/*
 * The device /sys/class/legion-intel-msr/intel-msr-0 (one instance, independent of the notebook)
 */
#define LEGION_INTEL_MSR_DEVICE_NAME "intel-msr-0"

static struct legion_intel_msr_data legion_intel_msr_data;

/*
 * Notify user space about an applied offset (EVENT_TYPE: plane, EVENT_VALUE: offset in uV)
 */
static void send_event(struct device *dev,
		       const enum legion_intel_msr_events_type type,
		       const int value)
{
	char event_type[32];
	char event_value[32];
	char *envp[] = { event_type, event_value, NULL };

	snprintf(event_type, sizeof(event_type), "EVENT_TYPE=%u",
		 (unsigned int)type);
	snprintf(event_value, sizeof(event_value), "EVENT_VALUE=%d", value);

	if (kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp))
		dev_err(dev, "Failed to send uevent for attribute change\n");
}

static ssize_t offset_show(struct device *dev, char *buf, const int plane)
{
	struct legion_intel_msr_data *priv = dev_get_drvdata(dev);
	int offset_uv = 0;
	const ssize_t ret = legion_intel_msr_offset_read_show(
		&priv->intel_msr_private, plane, &offset_uv);

	if (ret)
		return ret;

	return sysfs_emit(buf, "%d\n", offset_uv);
}

static ssize_t offset_store(struct device *dev, const char *buf,
			    const size_t count, const int plane,
			    const enum legion_intel_msr_events_type event_type)
{
	struct legion_intel_msr_data *priv = dev_get_drvdata(dev);
	int offset_uv = 0;
	ssize_t ret = kstrtoint(buf, 10, &offset_uv);

	if (ret)
		return ret;

	ret = legion_intel_msr_apply_voltage_offset(&priv->intel_msr_private,
						    plane, offset_uv);
	if (ret)
		return ret;

	/* Notify only about really applied value */
	send_event(dev, event_type, offset_uv);

	return count;
}

static const struct legion_intel_voltage_limits *
plane_limits(struct device *dev, const int plane)
{
	const struct legion_intel_msr_data *priv = dev_get_drvdata(dev);

	return &priv->intel_msr_private.plane_limits[plane];
}

/*
 * Attributes of a plane: <plane>_offset (uV, read/write), <plane>_max_undervolt, <plane>_max_overvolt (uV),
 * <plane>_offset_ctrl_supported (0/1)
 */
#define LEGION_INTEL_MSR_PLANE_ATTRS(_name, _plane, _event)                    \
	static ssize_t _name##_offset_show(                                    \
		struct device *dev, struct device_attribute *attr, char *buf)  \
	{                                                                      \
		return offset_show(dev, buf, _plane);                          \
	}                                                                      \
	static ssize_t _name##_offset_store(struct device *dev,                \
					    struct device_attribute *attr,     \
					    const char *buf, size_t count)     \
	{                                                                      \
		return offset_store(dev, buf, count, _plane, _event);          \
	}                                                                      \
	static DEVICE_ATTR_RW(_name##_offset);                                 \
	static ssize_t _name##_max_undervolt_show(                             \
		struct device *dev, struct device_attribute *attr, char *buf)  \
	{                                                                      \
		return sysfs_emit(                                             \
			buf, "%d\n",                                           \
			plane_limits(dev, _plane)->max_undervolt_uv);          \
	}                                                                      \
	static DEVICE_ATTR_RO(_name##_max_undervolt);                          \
	static ssize_t _name##_max_overvolt_show(                              \
		struct device *dev, struct device_attribute *attr, char *buf)  \
	{                                                                      \
		return sysfs_emit(buf, "%d\n",                                 \
				  plane_limits(dev, _plane)->max_overvolt_uv); \
	}                                                                      \
	static DEVICE_ATTR_RO(_name##_max_overvolt);                           \
	static ssize_t _name##_offset_ctrl_supported_show(                     \
		struct device *dev, struct device_attribute *attr, char *buf)  \
	{                                                                      \
		return sysfs_emit(buf, "%d\n",                                 \
				  plane_limits(dev, _plane)->write_supported); \
	}                                                                      \
	static DEVICE_ATTR_RO(_name##_offset_ctrl_supported)

LEGION_INTEL_MSR_PLANE_ATTRS(cpu, PLANE_CPU, LENOVO_INTEL_MSR_PLANE_CPU);
LEGION_INTEL_MSR_PLANE_ATTRS(cache, PLANE_CACHE, LENOVO_INTEL_MSR_PLANE_CACHE);
LEGION_INTEL_MSR_PLANE_ATTRS(gpu, PLANE_GPU, LENOVO_INTEL_MSR_PLANE_GPU);
LEGION_INTEL_MSR_PLANE_ATTRS(uncore, PLANE_UNCORE,
			     LENOVO_INTEL_MSR_PLANE_UNCORE);
LEGION_INTEL_MSR_PLANE_ATTRS(analogio, PLANE_ANALOGIO,
			     LENOVO_INTEL_MSR_PLANE_ANALOGIO);

static struct attribute *legion_intel_msr_attrs[] = {
	&dev_attr_cpu_offset.attr,
	&dev_attr_cache_offset.attr,
	&dev_attr_gpu_offset.attr,
	&dev_attr_uncore_offset.attr,
	&dev_attr_analogio_offset.attr,

	&dev_attr_cpu_max_undervolt.attr,
	&dev_attr_cpu_max_overvolt.attr,
	&dev_attr_cache_max_undervolt.attr,
	&dev_attr_cache_max_overvolt.attr,
	&dev_attr_gpu_max_undervolt.attr,
	&dev_attr_gpu_max_overvolt.attr,
	&dev_attr_uncore_max_undervolt.attr,
	&dev_attr_uncore_max_overvolt.attr,
	&dev_attr_analogio_max_undervolt.attr,
	&dev_attr_analogio_max_overvolt.attr,

	&dev_attr_cpu_offset_ctrl_supported.attr,
	&dev_attr_cache_offset_ctrl_supported.attr,
	&dev_attr_gpu_offset_ctrl_supported.attr,
	&dev_attr_uncore_offset_ctrl_supported.attr,
	&dev_attr_analogio_offset_ctrl_supported.attr,

	NULL
};
ATTRIBUTE_GROUPS(legion_intel_msr);

/*
 * The attributes are created with the device, before its uevent
 */
static const struct class legion_intel_msr_class = {
	.name = "legion-intel-msr",
	.dev_groups = legion_intel_msr_groups,
};

/*
 * Module: the device without a parent device, independent of the notebook (loaded by
 * /etc/modules-load.d/lenovo-legion.conf), refused on a CPU without the OC mailbox (MSR 0x150)
 */
static int __init legion_intel_msr_module_init(void)
{
	struct legion_intel_msr_data *data = &legion_intel_msr_data;
	int ret;

	ret = legion_intel_msr_init(&data->intel_msr_private);
	if (ret) {
		pr_info("legion-intel-msr: Intel OC mailbox (MSR 0x150) not available: %d\n",
			ret);
		goto err_msr_exit;
	}

	ret = class_register(&legion_intel_msr_class);
	if (ret)
		goto err_msr_exit;

	data->dev = device_create(&legion_intel_msr_class, NULL, MKDEV(0, 0),
				  data, LEGION_INTEL_MSR_DEVICE_NAME);
	if (IS_ERR(data->dev)) {
		ret = PTR_ERR(data->dev);
		goto err_unregister_class;
	}

	return 0;

err_unregister_class:
	class_unregister(&legion_intel_msr_class);
err_msr_exit:
	legion_intel_msr_exit(&data->intel_msr_private);
	return ret;
}

static void __exit legion_intel_msr_module_exit(void)
{
	struct legion_intel_msr_data *data = &legion_intel_msr_data;

	/* Waits for the running sysfs operations, the mutex is not used afterwards */
	device_unregister(data->dev);
	class_unregister(&legion_intel_msr_class);
	legion_intel_msr_exit(&data->intel_msr_private);
}

module_init(legion_intel_msr_module_init);
module_exit(legion_intel_msr_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaroslav Bolek");
MODULE_DESCRIPTION(
	"Intel CPU voltage offsets (OC mailbox MSR 0x150), part of Lenovo Legion Linux Toolkit");
MODULE_VERSION("0.9.10");
