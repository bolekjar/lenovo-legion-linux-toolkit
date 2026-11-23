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
#include "legion-wmi-ftable.h"
#include "legion-common.h"
#include "legion-wmi-fm.h"
#include "legion-wmi-platform.h"

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

#define LEGION_CAPABILITY_FTABLE_GUID "87fb2a6d-d802-48e7-9208-4576c5f5c8d8"

static_assert(sizeof(struct legion_wmi_ftable) == 0x3e);

struct legion_wmi_ftable_priv {
	struct wmi_device *wdev;
	struct legion_wmi_ftable_list *list;
};

struct legion_wmi_ftable_list {
	struct mutex list_mutex; /* list R/W mutex */
	u8 count;
	struct legion_wmi_ftable data[];
};

/*
 * Fan of a row: the newer firmware has uint16 Mode and uint16 Fan_Id, the older firmware uint32 Fan_Id and no
 * mode (binary MOF of LENOVO_FAN_TABLE_DATA); the rest of the row up to the sensor table is the same, so the fan
 * of the older firmware is the low half of Fan_Id read as the mode (little endian)
 */
static u16 legion_wmi_ftable_fan_id(const struct legion_wmi_ftable *row,
				    bool legacy)
{
	return legacy ? row->mode : row->fanId;
}

/**
 * legion_wmi_ftable_component_bind() - Bind component to master device.
 * @ftable_dev: Pointer to the lenovo-wmi-ftable driver parent device.
 * @om_dev: Pointer to the lenovo-wmi-other driver parent device.
 * @data: ftable_list object pointer used to return the capability data.
 *
 * On lenovo-wmi-other's master bind, provide a pointer to the local ftable
 * list. This is used to call lwmi_ftable_get_data to look up attribute data
 * from the lenovo-wmi-other driver.
 *
 * Return: 0
 */
static int legion_wmi_ftable_component_bind(struct device *ftable_dev,
					    struct device *om_dev, void *data)
{
	struct legion_wmi_ftable_priv *priv = dev_get_drvdata(ftable_dev);
	struct legion_wmi_fm_priv **fm_priv = data;
	bool legacy;
	int err;

	if (!priv || !priv->list || !fm_priv || !(*fm_priv))
		return -ENODEV;

	/* Layout of the rows of the firmware (legion_wmi_ftable_fan_id()), known once the other mode is probed */
	legacy = legion_wmi_platform_legacy(&err);
	if (err)
		return err;

	/*
	 * Initialize
	 */
	for (int i = 0; i < FAN_MAX; ++i) {
		(*fm_priv)->fan_max_speeds[i].fan_id = -1;
		(*fm_priv)->fan_max_speeds[i].max_speed = 0;
		(*fm_priv)->fan_max_speeds[i].sensor_id = -1;
	}

	(*fm_priv)->ft_list = priv->list;
	for (int idx = 0; idx < priv->list->count; idx++) {
		for (int i = 0; i < FAN_MAX; ++i) {
			if ((*fm_priv)->fan_max_speeds[i].fan_id ==
				    legion_wmi_ftable_fan_id(&priv->list->data[idx], legacy) &&
			    (*fm_priv)->fan_max_speeds[i].sensor_id ==
				    priv->list->data[idx].sensorId) {
				(*fm_priv)->fan_max_speeds[i].max_speed = max(
					(*fm_priv)->fan_max_speeds[i].max_speed,
					priv->list->data[idx].fanTableData
						[sizeof(priv->list->data[idx]
								.fanTableData) /
							 sizeof(priv->list
									->data[idx]
									.fanTableData
										[0]) -
						 1]);
				break;
			} else if ((*fm_priv)->fan_max_speeds[i].fan_id == -1 &&
				   (*fm_priv)->fan_max_speeds[i].sensor_id ==
					   -1) {
				(*fm_priv)->fan_max_speeds[i].fan_id =
					legion_wmi_ftable_fan_id(&priv->list->data[idx], legacy);
				(*fm_priv)->fan_max_speeds[i].sensor_id =
					priv->list->data[idx].sensorId;
				(*fm_priv)->fan_max_speeds[i].max_speed =
					priv->list->data[idx].fanTableData
						[sizeof(priv->list->data[idx]
								.fanTableData) /
							 sizeof(priv->list
									->data[idx]
									.fanTableData
										[0]) -
						 1];
				break;
			}
		}
	}

	return 0;
}

static const struct component_ops legion_wmi_ftable_component_ops = {
	.bind = legion_wmi_ftable_component_bind,
};

/**
 * legion_wmi_ftable_get_data - Get the data of the specified attribute
 * @list: The lenovo-wmi-ftable pointer to its ftable_list struct.
 * @attribute_id: The capdata attribute ID to be found.
 * @output: Pointer to a ftable struct to return the data.
 *
 * Retrieves the capability data 01 struct pointer for the given
 * attribute for its specified thermal mode.
 *
 * Return: 0 on success, or -EINVAL.
 */
int legion_wmi_ftable_get_data(struct legion_wmi_ftable_list *list,
			       const u16 mode, const u16 fanId,
			       const u16 sensorId,
			       struct legion_wmi_ftable *output)
{
	if (!list)
		return -ENODEV;

	guard(mutex)
		(&list->list_mutex);
	for (u8 idx = 0; idx < list->count; idx++) {
		if (list->data[idx].mode == mode &&
		    list->data[idx].fanId == fanId &&
		    list->data[idx].sensorId == sensorId) {
			memcpy(output, &list->data[idx],
			       sizeof(list->data[idx]));
			return 0;
		}
	};

	return -EINVAL;
}

/*
 * Points [start, end) of the fan curve may only be raised (upward) or lowered
 * from their default level, the default fan curve has the level n at the
 * point n (Fan_Get_Table default 1..10, the same as Lenovo Legion Toolkit).
 * An empty or invalid range (e.g. 7..0) is ignored.
 */
static void legion_wmi_ftable_curve_range(u8 start, u8 end, bool upward,
					  u8 limit[MAX_FANCURVE_SIZE])
{
	if (start >= end || end > MAX_FANCURVE_SIZE)
		return;

	for (u8 point = start; point < end; point++) {
		const u8 level = point + 1;

		limit[point] = upward ? max(limit[point], level) :
					min(limit[point], level);
	}
}

/**
 * legion_wmi_ftable_curve_limits() - Limits of the points of the fan curve
 * @list: The fan table.
 * @dev: Device of the messages.
 * @legacy: Older firmware (the rows have no mode, all rows are used).
 * @min: Minimal level of every point.
 * @max: Maximal level of every point.
 *
 * The firmware has one fan curve of the custom mode for all fans: the limits of
 * all rows of the custom mode are combined (the strictest one). The EC does not
 * check them, a curve under them runs the fans at the minimal speed under load
 * (measured on Legion Pro 7 16IAX10H), Lenovo Legion Space keeps them.
 *
 * Return: 0, or -ENODEV without the fan table.
 */
int legion_wmi_ftable_curve_limits(struct legion_wmi_ftable_list *list,
				   struct device *dev, bool legacy,
				   u8 min[MAX_FANCURVE_SIZE],
				   u8 max[MAX_FANCURVE_SIZE])
{
	for (int point = 0; point < MAX_FANCURVE_SIZE; point++) {
		min[point] = MIN_FANCURVE_LEVEL;
		max[point] = MAX_FANCURVE_LEVEL;
	}

	if (!list)
		return -ENODEV;

	guard(mutex)(&list->list_mutex);
	for (u8 idx = 0; idx < list->count; idx++) {
		const struct legion_wmi_ftable *row = &list->data[idx];

		if (!legacy && row->mode != LEGION_WMI_GZ_THERMAL_MODE_CUSTOM)
			continue;

		dev_dbg(dev,
			"fan table row %u: upward %u..%u, downward %u..%u\n",
			idx, row->startUpward, row->endUpward,
			row->startDownward, row->endDownward);

		legion_wmi_ftable_curve_range(row->startUpward, row->endUpward,
					      true, min);
		legion_wmi_ftable_curve_range(row->startDownward,
					      row->endDownward, false, max);
	}

	return 0;
}

/**
 * legion_wmi_ftable_get_data_legacy() - Row of the fan table of the older firmware
 * @list: The fan table.
 * @fanId: The fan (low half of the uint32 Fan_Id, legion_wmi_ftable_fan_id()).
 * @sensorId: The sensor.
 * @output: The row.
 *
 * The rows of the older firmware have no mode, they are the fan tables of the
 * custom mode.
 *
 * Return: 0 on success, -ENODEV without the fan table, -EINVAL without the row.
 */
int legion_wmi_ftable_get_data_legacy(struct legion_wmi_ftable_list *list,
				      u16 fanId, u16 sensorId,
				      struct legion_wmi_ftable *output)
{
	if (!list)
		return -ENODEV;

	guard(mutex)(&list->list_mutex);
	for (u8 idx = 0; idx < list->count; idx++) {
		if (legion_wmi_ftable_fan_id(&list->data[idx], true) == fanId &&
		    list->data[idx].sensorId == sensorId) {
			memcpy(output, &list->data[idx], sizeof(*output));
			return 0;
		}
	}

	return -EINVAL;
}

/**
 * legion_wmi_ftable_cache() - Cache all WMI data block information
 * @priv: lenovo-wmi-ftable driver data.
 * @instances: Number of the WMI data block instances.
 *
 * Loop through each WMI data block and cache the data. Only the valid rows are
 * kept (a buffer of the row, MAX_FANCURVE_SIZE points of both tables), an
 * empty row would be taken for a fan with the ID 0.
 *
 * Return: 0 on success, or an error.
 */
static int legion_wmi_ftable_cache(const struct legion_wmi_ftable_priv *priv,
				   int instances)
{
	guard(mutex)
		(&priv->list->list_mutex);
	for (int idx = 0; idx < instances; idx++) {
		struct legion_wmi_ftable *row =
			&priv->list->data[priv->list->count];
		union acpi_object *ret_obj __free(kfree) = NULL;

		ret_obj = wmidev_block_query(priv->wdev, idx);
		if (!ret_obj)
			return -ENODEV;

		if (ret_obj->type != ACPI_TYPE_BUFFER ||
		    ret_obj->buffer.length < sizeof(*row))
			continue;

		memcpy(row, ret_obj->buffer.pointer, sizeof(*row));
		if (row->lengthFanTable != MAX_FANCURVE_SIZE ||
		    row->lengthSensorTable != MAX_FANCURVE_SIZE) {
			dev_dbg(&priv->wdev->dev,
				"row %d skipped, table lengths %u / %u\n", idx,
				row->lengthFanTable, row->lengthSensorTable);
			continue;
		}

		priv->list->count++;
	}

	return 0;
}

/**
 * legion_wmi_ftable_alloc() - Allocate a ftable_list struct in drvdata
 * @priv: lenovo-wmi-ftable driver data.
 *
 * Allocate a ftable_list struct large enough to contain data from all WMI data
 * blocks provided by the interface.
 *
 * Return: The number of the WMI data block instances, or an error.
 */
static int legion_wmi_ftable_alloc(struct legion_wmi_ftable_priv *priv)
{
	struct legion_wmi_ftable_list *list = NULL;

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
 * legion_wmi_ftable_setup() - Cache all WMI data block information
 * @priv: lenovo-wmi-ftable driver data.
 *
 * Allocate a ftable_list struct large enough to contain data from all WMI data
 * blocks provided by the interface. Then loop through each data block and
 * cache the data.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_ftable_setup(struct legion_wmi_ftable_priv *priv)
{
	const int ret = legion_wmi_ftable_alloc(priv);

	if (ret < 0)
		return ret;

	return legion_wmi_ftable_cache(priv, ret);
}

static int legion_wmi_ftable_probe(struct wmi_device *wdev, const void *context)

{
	struct legion_wmi_ftable_priv *priv =
		devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;
	dev_set_drvdata(&wdev->dev, priv);

	const int ret = legion_wmi_ftable_setup(priv);

	if (ret)
		return ret;

	return component_add(&wdev->dev, &legion_wmi_ftable_component_ops);
}

static void legion_wmi_ftable_remove(struct wmi_device *wdev)
{
	component_del(&wdev->dev, &legion_wmi_ftable_component_ops);
}

static const struct wmi_device_id legion_wmi_ftable_id_table[] = {
	{ LEGION_CAPABILITY_FTABLE_GUID, NULL },
	{}
};

static struct wmi_driver legion_wmi_ftable_driver = {
	.driver = {
		.name = "legion_wmi_ftable",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_ftable_id_table,
	.probe = legion_wmi_ftable_probe,
	.remove = legion_wmi_ftable_remove,
	.no_singleton = true,
};

/**
 * legion_wmi_ftable_match() - Match rule for the master driver.
 * @dev: Pointer to the capability data 01 parent device.
 * @data: Unused void pointer for passing match criteria.
 *
 * Return: int.
 */
int legion_wmi_ftable_match(struct device *dev, void *data)
{
	return dev->driver == &legion_wmi_ftable_driver.driver;
}

int legion_wmi_ftable_driver_init(void)
{
	return wmi_driver_register(&legion_wmi_ftable_driver);
}

void legion_wmi_ftable_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_ftable_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_ftable_id_table);
