// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the lenovo-wmi-capdata01 driver of the Linux kernel (now drivers/platform/x86/lenovo/wmi-capdata.c),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */
#include "legion-wmi-capdata01.h"
#include "legion-wmi-other.h"
#include "legion-common.h"

#include <linux/acpi.h>
#include <linux/cleanup.h>
#include <linux/component.h>
#include <linux/container_of.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/gfp_types.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/mutex_types.h>
#include <linux/overflow.h>
#include <linux/types.h>
#include <linux/wmi.h>

#define LEGION_CAPABILITY_DATA_01_GUID "7A8F5407-CB67-4D6E-B547-39B3BE018154"

struct legion_wmi_cd01_priv {
	struct wmi_device *wdev;
	struct legion_wmi_cd01_list *list;
};

struct legion_wmi_cd01_list {
	struct mutex list_mutex; /* list R/W mutex */
	u8 count;
	struct legion_wmi_capdata01 data[];
};

/**
 * legion_wmi_cd01_component_bind() - Bind component to master device.
 * @cd01_dev: Pointer to the capability data 01 driver parent device.
 * @om_dev: Pointer to the other mode driver parent device.
 * @data: Other mode driver data, receives the capability data 01 list.
 *
 * On the master bind of the other mode, provide a pointer to the local
 * capability data 01 list. This is used to call legion_wmi_cd01_get_data() to
 * look up attribute data from the other mode driver.
 *
 * Return: 0
 */
static int legion_wmi_cd01_component_bind(struct device *cd01_dev,
					  struct device *om_dev, void *data)
{
	struct legion_wmi_cd01_priv *priv = dev_get_drvdata(cd01_dev);
	struct legion_wmi_other_priv **other_priv = data;

	if (!priv || !priv->list)
		return -ENODEV;

	(*other_priv)->cd01_list = priv->list;

	return 0;
}

static const struct component_ops legion_wmi_cd01_component_ops = {
	.bind = legion_wmi_cd01_component_bind,
};

/**
 * legion_wmi_cd01_get_data() - Get the data of the specified attribute
 * @list: The capability data 01 list.
 * @attribute_id: The capdata attribute ID to be found (with the power mode).
 * @output: Pointer to a capdata01 struct to return the data.
 *
 * Retrieves the capability data 01 of the given attribute in its power mode,
 * the first one when the firmware has the ID twice.
 *
 * Return: 0 on success, -ENODEV without the list, or -EINVAL.
 */
int legion_wmi_cd01_get_data(struct legion_wmi_cd01_list *list,
			     u32 attribute_id,
			     struct legion_wmi_capdata01 *output)
{
	if (!list)
		return -ENODEV;

	guard(mutex)
		(&list->list_mutex);

	for (u8 idx = 0; idx < list->count; idx++) {
		if (list->data[idx].id != attribute_id)
			continue;
		memcpy(output, &list->data[idx], sizeof(list->data[idx]));
		return 0;
	};

	return -EINVAL;
}

/**
 * legion_wmi_cd01_cache() - Cache all WMI data block information
 * @priv: Capability data 01 driver data.
 * @instances: Number of the WMI data block instances.
 *
 * Loop through each WMI data block and cache the data. Only the valid
 * instances are kept (a buffer of the size of the data), an empty one would be
 * a feature with the ID 0.
 *
 * Return: 0.
 */
static int legion_wmi_cd01_cache(struct legion_wmi_cd01_priv *priv,
				 int instances)
{
	guard(mutex)
		(&priv->list->list_mutex);
	for (int idx = 0; idx < instances; idx++) {
		struct legion_wmi_capdata01 *data =
			&priv->list->data[priv->list->count];
		union acpi_object *ret_obj __free(kfree) = NULL;

		ret_obj = wmidev_block_query(priv->wdev, idx);
		if (!ret_obj || ret_obj->type != ACPI_TYPE_BUFFER ||
		    ret_obj->buffer.length < sizeof(*data)) {
			dev_dbg(&priv->wdev->dev, "instance %d skipped\n", idx);
			continue;
		}

		memcpy(data, ret_obj->buffer.pointer, sizeof(*data));
		priv->list->count++;
	}

	return 0;
}

/**
 * legion_wmi_cd01_alloc() - Allocate a cd01_list struct in drvdata
 * @priv: Capability data 01 driver data.
 *
 * Allocate a cd01_list struct large enough to contain data from all WMI data
 * blocks provided by the interface.
 *
 * Return: The number of the WMI data block instances, or an error.
 */
static int legion_wmi_cd01_alloc(struct legion_wmi_cd01_priv *priv)
{
	struct legion_wmi_cd01_list *list = NULL;
	const int count = wmidev_instance_count(priv->wdev);
	const size_t list_size = struct_size(list, data, count);

	list = devm_kzalloc(&priv->wdev->dev, list_size, GFP_KERNEL);
	if (!list)
		return -ENOMEM;

	const int ret = devm_mutex_init(&priv->wdev->dev, &list->list_mutex);

	if (ret)
		return ret;

	priv->list = list;

	return count;
}

/**
 * legion_wmi_cd01_setup() - Cache all WMI data block information
 * @priv: Capability data 01 driver data.
 *
 * Allocate a cd01_list struct large enough to contain data from all WMI data
 * blocks provided by the interface. Then loop through each data block and
 * cache the data.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_cd01_setup(struct legion_wmi_cd01_priv *priv)
{
	const int ret = legion_wmi_cd01_alloc(priv);

	if (ret < 0)
		return ret;

	return legion_wmi_cd01_cache(priv, ret);
}

static int legion_wmi_cd01_probe(struct wmi_device *wdev, const void *context)

{
	struct legion_wmi_cd01_priv *priv = NULL;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;
	priv->list = NULL;

	dev_set_drvdata(&wdev->dev, priv);

	const int ret = legion_wmi_cd01_setup(priv);

	if (ret)
		return ret;

	return component_add(&wdev->dev, &legion_wmi_cd01_component_ops);
}

static void legion_wmi_cd01_remove(struct wmi_device *wdev)
{
	component_del(&wdev->dev, &legion_wmi_cd01_component_ops);
}

static const struct wmi_device_id legion_wmi_cd01_id_table[] = {
	{ LEGION_CAPABILITY_DATA_01_GUID, NULL },
	{}
};

static struct wmi_driver legion_wmi_cd01_driver = {
	.driver = {
		.name = "legion_wmi_cd01",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_cd01_id_table,
	.probe = legion_wmi_cd01_probe,
	.remove = legion_wmi_cd01_remove,
	.no_singleton = true,
};

/**
 * legion_wmi_cd01_match() - Match rule for the master driver.
 * @dev: Pointer to the capability data 01 parent device.
 * @data: Unused void pointer for passing match criteria.
 *
 * Return: int.
 */
int legion_wmi_cd01_match(struct device *dev, void *data)
{
	return dev->driver == &legion_wmi_cd01_driver.driver;
}

int legion_wmi_cd01_driver_init(void)
{
	return wmi_driver_register(&legion_wmi_cd01_driver);
}

void legion_wmi_cd01_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_cd01_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_cd01_id_table);
