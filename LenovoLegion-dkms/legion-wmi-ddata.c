// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the structure of the lenovo-wmi-capdata01 driver of the Linux kernel
 * (now drivers/platform/x86/lenovo/wmi-capdata.c),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */
#include "legion-wmi-ddata.h"
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

#define LEGION_CAPABILITY_DDATA_01_GUID "91433B17-B7B7-4640-BB40-34C67349FBEC"

struct legion_wmi_dd_priv {
	struct wmi_device *wdev;
	struct legion_wmi_dd_list *list;
};

struct legion_wmi_dd_list {
	struct mutex list_mutex; /* list R/W mutex */
	u8 count;
	struct legion_wmi_ddata data[];
};

/**
 * legion_wmi_dd_component_bind() - Bind component to master device.
 * @dd_dev: Pointer to the discrete data driver parent device.
 * @om_dev: Pointer to the other mode driver parent device.
 * @data: Other mode driver data, receives the discrete data list.
 *
 * On the master bind of the other mode, provide a pointer to the local
 * discrete data list. This is used to call legion_wmi_dd_get_data() to look up
 * the discrete values from the other mode driver.
 *
 * Return: 0
 */
static int legion_wmi_dd_component_bind(struct device *dd_dev,
					struct device *om_dev, void *data)
{
	struct legion_wmi_dd_priv *priv = dev_get_drvdata(dd_dev);
	struct legion_wmi_other_priv **other_priv = data;

	if (!priv || !priv->list || !other_priv || !(*other_priv))
		return -ENODEV;

	(*other_priv)->dd_list = priv->list;

	return 0;
}

static const struct component_ops legion_wmi_dd_component_ops = {
	.bind = legion_wmi_dd_component_bind,
};

/**
 * legion_wmi_dd_get_data() - Get the discrete values of the specified attribute
 * @list: The discrete data list.
 * @attribute_id: The attribute ID to be found (with the power mode).
 * @output: Array receiving the discrete values.
 * @output_len: Size of the array.
 *
 * Return: The number of the discrete values written to output, -ENODEV without
 * the list, -EINVAL without a value, or -E2BIG when they do not fit.
 */
int legion_wmi_dd_get_data(struct legion_wmi_dd_list *list, u32 attribute_id,
			   struct legion_wmi_ddata *output,
			   const size_t output_len)
{
	u8 out_idx = 0;

	if (!list)
		return -ENODEV;

	guard(mutex)
		(&list->list_mutex);
	for (u8 idx = 0; idx < list->count; idx++) {
		if (list->data[idx].id != attribute_id)
			continue;

		/* More values than the output can hold */
		if (out_idx >= output_len)
			return -E2BIG;

		memcpy(&output[out_idx++], &list->data[idx],
		       sizeof(list->data[idx]));
	}

	if (out_idx == 0)
		return -EINVAL;

	return out_idx;
}

/**
 * legion_wmi_dd_cache() - Cache all WMI data block information
 * @priv: Discrete data driver data.
 * @instances: Number of the WMI data block instances.
 *
 * Loop through each WMI data block and cache the data. Only the valid
 * instances are kept: a buffer of the size of the data and an ID (the firmware
 * fills the data block up with instances of the ID 0).
 *
 * Return: 0.
 */
static int legion_wmi_dd_cache(const struct legion_wmi_dd_priv *priv,
			       int instances)
{
	guard(mutex)
		(&priv->list->list_mutex);
	for (int idx = 0; idx < instances; idx++) {
		struct legion_wmi_ddata *data =
			&priv->list->data[priv->list->count];
		union acpi_object *ret_obj __free(kfree) = NULL;

		ret_obj = wmidev_block_query(priv->wdev, idx);
		if (!ret_obj || ret_obj->type != ACPI_TYPE_BUFFER ||
		    ret_obj->buffer.length < sizeof(*data)) {
			dev_dbg(&priv->wdev->dev, "instance %d skipped\n", idx);
			continue;
		}

		memcpy(data, ret_obj->buffer.pointer, sizeof(*data));
		if (data->id)
			priv->list->count++;
	}

	return 0;
}

/**
 * legion_wmi_dd_alloc() - Allocate a dd_list struct in drvdata
 * @priv: Discrete data driver data.
 *
 * Allocate a dd_list struct large enough to contain data from all WMI data
 * blocks provided by the interface.
 *
 * Return: The number of the WMI data block instances, or an error.
 */
static int legion_wmi_dd_alloc(struct legion_wmi_dd_priv *priv)
{
	struct legion_wmi_dd_list *list = NULL;

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
 * legion_wmi_dd_setup() - Cache all WMI data block information
 * @priv: Discrete data driver data.
 *
 * Allocate a dd_list struct large enough to contain data from all WMI data
 * blocks provided by the interface. Then loop through each data block and
 * cache the data.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_dd_setup(struct legion_wmi_dd_priv *priv)
{
	const int ret = legion_wmi_dd_alloc(priv);

	if (ret < 0)
		return ret;

	return legion_wmi_dd_cache(priv, ret);
}

static int legion_wmi_dd_probe(struct wmi_device *wdev, const void *context)

{
	struct legion_wmi_dd_priv *priv = NULL;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;
	dev_set_drvdata(&wdev->dev, priv);

	const int ret = legion_wmi_dd_setup(priv);

	if (ret)
		return ret;

	return component_add(&wdev->dev, &legion_wmi_dd_component_ops);
}

static void legion_wmi_dd_remove(struct wmi_device *wdev)
{
	component_del(&wdev->dev, &legion_wmi_dd_component_ops);
}

static const struct wmi_device_id legion_wmi_dd_id_table[] = {
	{ LEGION_CAPABILITY_DDATA_01_GUID, NULL },
	{}
};

static struct wmi_driver legion_wmi_dd_driver = {
	.driver = {
		.name = "legion_wmi_dd",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_dd_id_table,
	.probe = legion_wmi_dd_probe,
	.remove = legion_wmi_dd_remove,
	.no_singleton = true,
};

/**
 * legion_wmi_dd_match() - Match rule for the master driver.
 * @dev: Pointer to the discrete data parent device.
 * @data: Unused void pointer for passing match criteria.
 *
 * Return: int.
 */
int legion_wmi_dd_match(struct device *dev, void *data)
{
	return dev->driver == &legion_wmi_dd_driver.driver;
}

int legion_wmi_dd_driver_init(void)
{
	return wmi_driver_register(&legion_wmi_dd_driver);
}

void legion_wmi_dd_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_dd_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_dd_id_table);
