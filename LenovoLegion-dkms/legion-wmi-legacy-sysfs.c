// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Firmware attributes of the CPU and GPU power limits of the older Legion firmware:
 * /sys/class/legion-firmware-attributes/legion-wmi-cpu-0 (legion-wmi-gpu-0)/attributes/<limit>/
 * {current_value, default_value, min_value, max_value, scalar_increment, supported, type, display_name}, the names of
 * the limits are the same as of the other mode; a write sends a uevent (EVENT_TYPE 4000 - 4999).
 */
#include "legion-firmware-attributes-class.h"
#include "legion-wmi-legacy-sysfs.h"
#include "legion-wmi-legacy.h"

#include <linux/device.h>
#include <linux/idr.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/wmi.h>

static const char *const legion_wmi_legacy_field_names[LEGACY_FIELDS] = {
	[LEGACY_FIELD_CURRENT] = "current_value",
	[LEGACY_FIELD_DEFAULT] = "default_value",
	[LEGACY_FIELD_MIN] = "min_value",
	[LEGACY_FIELD_MAX] = "max_value",
	[LEGACY_FIELD_STEP] = "scalar_increment",
	[LEGACY_FIELD_SUPPORTED] = "supported",
	[LEGACY_FIELD_TYPE] = "type",
	[LEGACY_FIELD_DISPLAY_NAME] = "display_name",
};

/**
 * struct legion_wmi_legacy_attr - A file of a limit
 * @kattr: The attribute.
 * @priv: The driver data.
 * @limit: The limit.
 * @field: The file.
 */
struct legion_wmi_legacy_attr {
	struct kobj_attribute kattr;
	struct legion_wmi_legacy_priv *priv;
	const struct legion_wmi_legacy_limit *limit;
	enum legion_wmi_legacy_field field;
};

static DEFINE_IDA(legion_wmi_legacy_ida);

static void legion_wmi_legacy_send_event(struct legion_wmi_legacy_priv *priv,
					 enum legion_events_type type,
					 u32 value)
{
	char event_type[32];
	char event_value[32];
	char *envp[] = { event_type, event_value, NULL };

	snprintf(event_type, sizeof(event_type), "EVENT_TYPE=%u",
		 (unsigned int)type);
	snprintf(event_value, sizeof(event_value), "EVENT_VALUE=%u", value);

	if (kobject_uevent_env(&priv->fw_attr_dev->kobj, KOBJ_CHANGE, envp))
		dev_err(priv->fw_attr_dev,
			"Failed to send uevent for attribute change\n");
}

static ssize_t legion_wmi_legacy_show(struct kobject *kobj,
				      struct kobj_attribute *kattr, char *buf)
{
	struct legion_wmi_legacy_attr *attr =
		container_of(kattr, struct legion_wmi_legacy_attr, kattr);
	u32 value;
	int err;

	switch (attr->field) {
	case LEGACY_FIELD_SUPPORTED:
		return sysfs_emit(buf, "1\n");
	case LEGACY_FIELD_TYPE:
		return sysfs_emit(buf, "integer\n");
	case LEGACY_FIELD_DISPLAY_NAME:
		return sysfs_emit(buf, "%s\n", attr->limit->display_name);
	default:
		break;
	}

	scoped_guard(mutex, &attr->priv->lock)
		err = legion_wmi_legacy_read(attr->priv, attr->limit,
					     attr->field, &value);
	if (err)
		return err;

	return sysfs_emit(buf, "%u\n", value);
}

static ssize_t legion_wmi_legacy_store(struct kobject *kobj,
				       struct kobj_attribute *kattr,
				       const char *buf, size_t count)
{
	struct legion_wmi_legacy_attr *attr =
		container_of(kattr, struct legion_wmi_legacy_attr, kattr);
	u32 value;
	int err;

	err = kstrtouint(buf, 10, &value);
	if (err)
		return err;

	err = legion_wmi_legacy_write(attr->priv, attr->limit, value);
	if (err)
		return err;

	legion_wmi_legacy_send_event(attr->priv, attr->limit->event, value);

	return count;
}

/*
 * Attributes of a limit (default_value only when the firmware provides it)
 */
static int legion_wmi_legacy_build_group(struct legion_wmi_legacy_priv *priv,
					 const struct legion_wmi_legacy_limit *limit,
					 struct attribute_group *group)
{
	struct device *dev = &priv->wdev->dev;
	struct legion_wmi_legacy_attr *attrs;
	struct attribute **list;
	unsigned int count = 0;
	u32 value;
	int err;

	attrs = devm_kcalloc(dev, LEGACY_FIELDS, sizeof(*attrs), GFP_KERNEL);
	list = devm_kcalloc(dev, LEGACY_FIELDS + 1, sizeof(*list), GFP_KERNEL);
	if (!attrs || !list)
		return -ENOMEM;

	scoped_guard(mutex, &priv->lock)
		err = legion_wmi_legacy_read(priv, limit, LEGACY_FIELD_DEFAULT,
					     &value);

	for (int field = 0; field < LEGACY_FIELDS; field++) {
		struct legion_wmi_legacy_attr *attr = &attrs[count];

		if (field == LEGACY_FIELD_DEFAULT && err)
			continue;

		sysfs_attr_init(&attr->kattr.attr);
		attr->kattr.attr.name = legion_wmi_legacy_field_names[field];
		attr->kattr.attr.mode = field == LEGACY_FIELD_CURRENT ? 0644 :
									0444;
		attr->kattr.show = legion_wmi_legacy_show;
		attr->kattr.store = field == LEGACY_FIELD_CURRENT ?
					    legion_wmi_legacy_store :
					    NULL;
		attr->priv = priv;
		attr->limit = limit;
		attr->field = field;

		list[count++] = &attr->kattr.attr;
	}

	group->name = limit->name;
	group->attrs = list;

	return 0;
}

/*
 * Groups of the limits supported by the firmware, -ENODEV when there is none
 */
static int legion_wmi_legacy_build_groups(struct legion_wmi_legacy_priv *priv)
{
	const struct legion_wmi_legacy_class *wmi_class = priv->wmi_class;
	struct device *dev = &priv->wdev->dev;
	unsigned int supported = 0;
	int err;

	priv->groups = devm_kcalloc(dev, wmi_class->count,
				    sizeof(*priv->groups), GFP_KERNEL);
	if (!priv->groups)
		return -ENOMEM;

	for (size_t i = 0; i < wmi_class->count; i++) {
		const struct legion_wmi_legacy_limit *limit =
			&wmi_class->limits[i];

		if (!legion_wmi_legacy_supported(priv, limit)) {
			dev_dbg(dev, "%s is not supported\n", limit->name);
			continue;
		}

		err = legion_wmi_legacy_build_group(priv, limit,
						    &priv->groups[i]);
		if (err)
			return err;

		supported++;
	}

	if (!supported)
		return -ENODEV;

	dev_info(dev, "%u limits of %s supported\n", supported,
		 wmi_class->base_path);

	return 0;
}

/**
 * legion_wmi_legacy_fw_attr_add() - Register the device and the supported limits
 * @priv: The driver data.
 *
 * Return: Either 0, or an error code.
 */
static int legion_wmi_legacy_fw_attr_add(struct legion_wmi_legacy_priv *priv)
{
	const struct legion_wmi_legacy_class *wmi_class = priv->wmi_class;
	size_t i;
	int err;

	priv->ida_id = ida_alloc(&legion_wmi_legacy_ida, GFP_KERNEL);
	if (priv->ida_id < 0)
		return priv->ida_id;

	priv->fw_attr_dev = device_create(&legion_firmware_attributes_class,
					  NULL, MKDEV(0, 0), priv, "%s-%u",
					  wmi_class->base_path, priv->ida_id);
	if (IS_ERR(priv->fw_attr_dev)) {
		err = PTR_ERR(priv->fw_attr_dev);
		goto err_free_ida;
	}

	priv->fw_attr_kset = kset_create_and_add("attributes", NULL,
						 &priv->fw_attr_dev->kobj);
	if (!priv->fw_attr_kset) {
		err = -ENOMEM;
		goto err_unregister_fw_attr_dev;
	}

	for (i = 0; i < wmi_class->count; i++) {
		if (!priv->groups[i].name)
			continue;

		err = sysfs_create_group(&priv->fw_attr_kset->kobj,
					 &priv->groups[i]);
		if (err)
			goto err_remove_groups;
	}

	return 0;

err_remove_groups:
	while (i--)
		if (priv->groups[i].name)
			sysfs_remove_group(&priv->fw_attr_kset->kobj,
					   &priv->groups[i]);
	kset_unregister(priv->fw_attr_kset);
err_unregister_fw_attr_dev:
	device_unregister(priv->fw_attr_dev);
err_free_ida:
	ida_free(&legion_wmi_legacy_ida, priv->ida_id);
	return err;
}

static void legion_wmi_legacy_fw_attr_remove(struct legion_wmi_legacy_priv *priv)
{
	for (size_t i = 0; i < priv->wmi_class->count; i++)
		if (priv->groups[i].name)
			sysfs_remove_group(&priv->fw_attr_kset->kobj,
					   &priv->groups[i]);

	kset_unregister(priv->fw_attr_kset);
	device_unregister(priv->fw_attr_dev);
	ida_free(&legion_wmi_legacy_ida, priv->ida_id);
}

int legion_wmi_legacy_sysfs_init(struct legion_wmi_legacy_priv *priv)
{
	const int err = legion_wmi_legacy_build_groups(priv);

	if (err)
		return err;

	return legion_wmi_legacy_fw_attr_add(priv);
}

void legion_wmi_legacy_sysfs_exit(struct legion_wmi_legacy_priv *priv)
{
	legion_wmi_legacy_fw_attr_remove(priv);
}
