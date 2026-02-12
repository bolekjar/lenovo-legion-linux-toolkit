// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-wmi-capdata00.h"
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


#define LEGION_CAPABILITY_DATA_00_GUID "362A3AFE-3D96-4665-8530-96dAD5BB300E"

struct legion_wmi_cd00_priv {
	struct wmi_device *wdev;
	struct legion_wmi_cd00_list *list;
};

struct legion_wmi_cd00_list {
	struct mutex list_mutex; /* list R/W mutex */
	u8 count;
	struct legion_wmi_capdata00 data[];
};


/**
 * legion_wmi_cd00_component_bind() - Bind component to master device.
 * @cd00_dev: Pointer to the lenovo-wmi-capdata01 driver parent device.
 * @om_dev: Pointer to the lenovo-wmi-other driver parent device.
 * @data: capdata01_list object pointer used to return the capability data.
 *
 * On lenovo-wmi-other's master bind, provide a pointer to the local capdata01
 * list. This is used to call lwmi_cd00_get_data to look up attribute data
 * from the lenovo-wmi-other driver.
 *
 * Return: 0
 */
static int legion_wmi_cd00_component_bind(struct device *cd00_dev,
				    struct device *om_dev, void *data)
{
	struct legion_wmi_cd00_priv *priv = dev_get_drvdata(cd00_dev);
	struct legion_wmi_other_priv **other_priv = data;

	if(!priv || !priv->list)
		return -ENODEV;


	(*other_priv)->cd00_list = priv->list;

	return 0;
}

static const struct component_ops legion_wmi_cd00_component_ops = {
	.bind = legion_wmi_cd00_component_bind,
};

/**
 * legion_wmi_cd00_get_data - Get the data of the specified attribute
 * @list: The lenovo-wmi-capdata01 pointer to its cd00_list struct.
 * @attribute_id: The capdata attribute ID to be found.
 * @output: Pointer to a capdata01 struct to return the data.
 *
 * Retrieves the capability data 01 struct pointer for the given
 * attribute for its specified thermal mode.
 *
 * Return: 0 on success, or -EINVAL.
 */
int legion_wmi_cd00_get_data(struct legion_wmi_cd00_list *list, u32 attribute_id, struct legion_wmi_capdata00 *output)
{
	u8 idx = 0;

    if (!list)
        return -ENODEV;

    guard(mutex)(&list->list_mutex);

	for (idx = 0; idx < list->count; idx++) {
		if (list->data[idx].id != attribute_id)
			continue;
		memcpy(output, &list->data[idx], sizeof(list->data[idx]));
		return 0;
	};

	return -EINVAL;
}

/**
 * legion_wmi_cd00_cache() - Cache all WMI data block information
 * @priv: lenovo-wmi-capdata01 driver data.
 *
 * Loop through each WMI data block and cache the data.
 *
 * Return: 0 on success, or an error.
 */
static int legion_wmi_cd00_cache(struct legion_wmi_cd00_priv *priv)
{
	int idx;

	guard(mutex)(&priv->list->list_mutex);
	for (idx = 0; idx < priv->list->count; idx++) {
		union acpi_object *ret_obj __free(kfree) = NULL;

		ret_obj = wmidev_block_query(priv->wdev, idx);
		if (!ret_obj)
			return -ENODEV;

		if (ret_obj->type != ACPI_TYPE_BUFFER ||
		    ret_obj->buffer.length < sizeof(priv->list->data[idx]))
			continue;

		memcpy(&priv->list->data[idx], ret_obj->buffer.pointer,
		       min(ret_obj->buffer.length,sizeof(priv->list->data[idx])));
	}

	return 0;
}

/**
 * legion_wmi_cd00_alloc() - Allocate a cd00_list struct in drvdata
 * @priv: lenovo-wmi-capdata01 driver data.
 *
 * Allocate a cd00_list struct large enough to contain data from all WMI data
 * blocks provided by the interface.
 *
 * Return: 0 on success, or an error.
 */
static int legion_wmi_cd00_alloc(struct legion_wmi_cd00_priv *priv)
{
	struct legion_wmi_cd00_list *list = NULL;
	size_t list_size = 0;
	int count = 0, ret = 0;

	count = wmidev_instance_count(priv->wdev);
	list_size = struct_size(list, data, count);

	list = devm_kzalloc(&priv->wdev->dev, list_size, GFP_KERNEL);
	if (!list)
		return -ENOMEM;

	ret = devm_mutex_init(&priv->wdev->dev, &list->list_mutex);
	if (ret)
		return ret;

	list->count = count;
	priv->list = list;

	return 0;
}

/**
 * legion_wmi_cd00_setup() - Cache all WMI data block information
 * @priv: lenovo-wmi-capdata01 driver data.
 *
 * Allocate a cd00_list struct large enough to contain data from all WMI data
 * blocks provided by the interface. Then loop through each data block and
 * cache the data.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_cd00_setup(struct legion_wmi_cd00_priv *priv)
{
	int ret;

	ret = legion_wmi_cd00_alloc(priv);
	if (ret)
		return ret;

	return legion_wmi_cd00_cache(priv);
}


static int legion_wmi_cd00_probe(struct wmi_device *wdev, const void *context)

{
	struct legion_wmi_cd00_priv *priv = NULL;
	int ret = 0;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;
	priv->list = NULL;


	dev_set_drvdata(&wdev->dev, priv);

	ret = legion_wmi_cd00_setup(priv);
	if (ret)
		return ret;

	return component_add(&wdev->dev, &legion_wmi_cd00_component_ops);
}

static void legion_wmi_cd00_remove(struct wmi_device *wdev)
{
	component_del(&wdev->dev, &legion_wmi_cd00_component_ops);
}

static const struct wmi_device_id legion_wmi_cd00_id_table[] = {
	{ LEGION_CAPABILITY_DATA_00_GUID, NULL },
	{}
};

static struct wmi_driver legion_wmi_cd00_driver = {
	.driver = {
		.name = "legion_wmi_cd00",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_cd00_id_table,
	.probe = legion_wmi_cd00_probe,
	.remove = legion_wmi_cd00_remove,
	.no_singleton = true,
};

/**
 * lwmi_cd00_match() - Match rule for the master driver.
 * @dev: Pointer to the capability data 01 parent device.
 * @data: Unused void pointer for passing match criteria.
 *
 * Return: int.
 */
int legion_wmi_cd00_match(struct device *dev, void *data)
{
	return dev->driver == &legion_wmi_cd00_driver.driver;
}


int  legion_wmi_cd00_driver_init(struct device *parent) {
	return wmi_driver_register(&legion_wmi_cd00_driver);
}

void legion_wmi_cd00_driver_exit(void) {
	wmi_driver_unregister(&legion_wmi_cd00_driver);
}

MODULE_DEVICE_TABLE(acpi, legion_wmi_cd00_id_table);

