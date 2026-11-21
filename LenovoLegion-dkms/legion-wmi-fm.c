// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the notifier chain of the lenovo-wmi-other driver of the Linux kernel
 * (drivers/platform/x86/lenovo/wmi-other.c),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */
#include "legion-wmi-fm.h"
#include "legion-wmi-ftable.h"
#include "legion-wmi-fm-sysfs.h"
#include "legion-hwmon.h"
#include "legion-wmi-capdata00.h"
#include "legion-wmi-helpers.h"
#include "legion-wmi-platform.h"

#include <linux/wmi.h>
#include <linux/component.h>
#include <linux/unaligned.h>

#define WMI_GUID_LENOVO_FAN_METHOD "92549549-4bde-4f06-ac04-ce8bf898dbaa"

const int fan_table_id_v1[] = { 1, 1, 2, 0 };
const int fan_sensor_id_v1[] = { 4, 1, 5, 0 };

const int fan_table_id_v2[] = { 1, 0, 2, 4 };
const int fan_sensor_id_v2[] = { 1, 0, 5, 4 };

static BLOCKING_NOTIFIER_HEAD(legion_fm_chain_head);

/**
 * lenovo_wmi_fm_register_notifier() - Add a notifier to the blocking notifier chain
 * @nb: The notifier_block struct to register
 *
 * Call blocking_notifier_chain_register to register the notifier block to the
 * lenovo-wmi-other driver notifier chain.
 *
 * Return: 0 on success, %-EEXIST on error.
 */
static int lenovo_wmi_fm_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&legion_fm_chain_head, nb);
}

/**
 * lenovo_wmi_fm_unregister_notifier() - Remove a notifier from the blocking notifier
 * chain.
 * @nb: The notifier_block struct to register
 *
 * Call blocking_notifier_chain_unregister to unregister the notifier block from the
 * lenovo-wmi-other driver notifier chain.
 *
 * Return: 0 on success, %-ENOENT on error.
 */
static int lenovo_wmi_fm_unregister_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&legion_fm_chain_head, nb);
}

/**
 * devm_lenovo_wmi_fm_unregister_notifier() - Remove a notifier from the blocking
 * notifier chain.
 * @data: Void pointer to the notifier_block struct to register.
 *
 * Call lwmi_om_unregister_notifier to unregister the notifier block from the
 * lenovo-wmi-other driver notifier chain.
 *
 * Return: 0 on success, %-ENOENT on error.
 */
static void devm_lenovo_wmi_fm_unregister_notifier(void *data)
{
	struct notifier_block *nb = data;

	lenovo_wmi_fm_unregister_notifier(nb);
}

static int legion_wmi_fm_call(struct notifier_block *nb, unsigned long cmd,
			      void *data)
{
	struct legion_wmi_fm_priv *priv =
		container_of(nb, struct legion_wmi_fm_priv, hwmon_nb);
	switch (cmd) {
	case LEGION_WMI_FM_GET_CPU_FAN_MAX_SPEED: {
		int *fan_max_speed = data;
		*fan_max_speed = priv->fan_cpu_max_speed;
	}
		return NOTIFY_OK;
	case LEGION_WMI_FM_GET_GPU_FAN_MAX_SPEED: {
		int *fan_max_speed = data;
		*fan_max_speed = priv->fan_gpu_max_speed;
	}
		return NOTIFY_OK;
	case LEGION_WMI_FM_GET_SYS_FAN_MAX_SPEED: {
		int *fan_max_speed = data;
		*fan_max_speed = priv->fan_sys_max_speed;
	}
		return NOTIFY_OK;
	default:
		return NOTIFY_DONE;
	}
}

/**
 * devm_lenovo_wmi_fm_register_notifier() - Add a notifier to the blocking notifier
 * chain.
 * @dev: The parent device of the notifier_block struct.
 * @nb: The notifier_block struct to register
 *
 * Call lwmi_om_register_notifier to register the notifier block to the
 * lenovo-wmi-other driver notifier chain. Then add devm_lwmi_om_unregister_notifier
 * as a device managed action to automatically unregister the notifier block
 * upon parent device removal.
 *
 * Return: 0 on success, or an error code.
 */
int devm_lenovo_wmi_fm_register_notifier(struct device *dev,
					 struct notifier_block *nb)
{
	const int ret = lenovo_wmi_fm_register_notifier(nb);

	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(
		dev, devm_lenovo_wmi_fm_unregister_notifier, nb);
}

int legion_wmi_fm_notifier_call(void *data,
				enum gamezone_events_type gamezone_events_type)
{
	const int ret = blocking_notifier_call_chain(
		&legion_fm_chain_head, gamezone_events_type, &data);

	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

/*
 * Sensors of the older firmware (the same as Lenovo Legion Toolkit): Fan_GetCurrentSensorTemperature (CPU sensor 3,
 * GPU sensor 4) and Fan_GetCurrentFanSpeed (CPU fan 0, GPU fan 1), answered for the capability IDs of the other
 * mode (source of the hwmon)
 */
static const struct {
	enum CapabilityID id;
	u32 method_id;
	u8 argument;
} legion_wmi_fm_sensors[] = {
	{ CpuCurrentTemperature, WMI_METHOD_ID_FAN_GET_CURRENT_SENSOR_TEMPERATURE, 3 },
	{ GpuCurrentTemperature, WMI_METHOD_ID_FAN_GET_CURRENT_SENSOR_TEMPERATURE, 4 },
	{ CpuCurrentFanSpeed, WMI_METHOD_ID_FAN_GET_CURRENT_FAN_SPEED, 0 },
	{ GpuCurrentFanSpeed, WMI_METHOD_ID_FAN_GET_CURRENT_FAN_SPEED, 1 },
};

static int legion_wmi_fm_sensor_read(struct wmi_device *wdev, int index,
				     u32 *value)
{
	u8 argument = legion_wmi_fm_sensors[index].argument;
	u8 result[8];
	size_t length = 0;
	int ret;

	ret = legion_wmi_dev_evaluate_bytes(wdev, 0,
					    legion_wmi_fm_sensors[index].method_id,
					    &argument, sizeof(argument), result,
					    sizeof(result), &length);
	if (ret)
		return ret;

	if (length < sizeof(u16))
		return -ENODATA;

	*value = get_unaligned_le16(result); /* uint16 */
	return 0;
}

static int legion_wmi_fm_sensor_index(enum CapabilityID id)
{
	for (int i = 0; i < ARRAY_SIZE(legion_wmi_fm_sensors); i++)
		if (legion_wmi_fm_sensors[i].id == id)
			return i;

	return -1;
}

static int legion_wmi_fm_sensors_call(struct notifier_block *nb,
				      unsigned long action, void *data)
{
	struct legion_wmi_fm_priv *priv =
		container_of(nb, struct legion_wmi_fm_priv, sensors_nb);
	const struct hwmon_capability *request = data;
	const enum CapabilityID id =
		request->capability_id & ~LEGION_WMI_MODE_ID_MASK;
	const int index = legion_wmi_fm_sensor_index(id);

	switch (action) {
	case LEGION_WMI_FM_GET_CAPABILITY_VALUE:
		if (index < 0 ||
		    legion_wmi_fm_sensor_read(priv->wdev, index, request->data))
			return NOTIFY_BAD;

		return NOTIFY_OK;
	case LEGION_WMI_FM_GET_CAPABILITY_DATA: {
		struct legion_wmi_capdata00 *capdata = request->data;
		u32 value;

		capdata->id = id;
		capdata->supported =
			index >= 0 &&
			!legion_wmi_fm_sensor_read(priv->wdev, index, &value);
		capdata->default_value = 0;
		return NOTIFY_OK;
	}
	default:
		return NOTIFY_DONE;
	}
}

/*
 * Fan table of the custom mode written by WMI_METHOD_ID_FAN_SET_TABLE (packed, little endian):
 * Fan_Set_Table([in, Max(64)] uint8 FanTable[]), the firmware gets 64 bytes (the same as Lenovo Legion Toolkit),
 * ASL reads its fields only (newer firmware: up to 0x19)
 */
struct legion_wmi_fan_table_write {
	u8 fstm; /* FSMD: 1 = fan table of the custom mode */
	u8 fsid;
	__le32 fstl; /* FSST */
	__le16 fss[MAX_FANCURVE_SIZE]; /* FSS0..FSS9: levels 0..MAX_FANCURVE_LEVEL */
	u8 reserved[38];
} __packed;
static_assert(sizeof(struct legion_wmi_fan_table_write) == 64);

/*
 * Fan table of the custom mode read by WMI_METHOD_ID_FAN_GET_TABLE (packed, little endian)
 */
struct legion_wmi_fan_table_read {
	__le32 fsfl;
	__le32 fss[MAX_FANCURVE_SIZE]; /* FSS0..FSS9: levels 0..MAX_FANCURVE_LEVEL */
	__le32 fssa;
} __packed;
static_assert(sizeof(struct legion_wmi_fan_table_read) == 0x30);

/*
 * Input of WMI_METHOD_ID_FAN_GET_TABLE: Fan_Get_Table([in] uint8 FanID, [in] uint8 SensorID)
 */
struct legion_wmi_fan_table_read_input {
	u8 fan_id;
	u8 sensor_id;
} __packed;
static_assert(sizeof(struct legion_wmi_fan_table_read_input) == 2);

/* Maximal size of the returned buffer accepted from the firmware */
#define LEGION_WMI_FAN_TABLE_READ_MAX 176

/**
 * legion_wmi_fm_table_get() - Fan table of the custom mode (Fan_Get_Table)
 * @priv: The fan method driver data.
 * @points: Points of the fan table.
 *
 * The older firmware returns another table (uint8, by FanID and SensorID), Lenovo Legion Toolkit does not read
 * it (it reads no fan table by this method at all).
 *
 * Return: 0 on success, -EOPNOTSUPP on the older firmware, or an error code.
 */
int legion_wmi_fm_table_get(struct legion_wmi_fm_priv *priv,
			    u32 points[MAX_FANCURVE_SIZE])
{
	u8 buffer[LEGION_WMI_FAN_TABLE_READ_MAX] = {};
	/*
	 * Input of the MOF: the newer firmware ignores it (ASL returns the one fan table of the custom mode),
	 * passed for a firmware which reads it
	 */
	struct legion_wmi_fan_table_read_input input = {
		.fan_id = 0,
		.sensor_id = 0,
	};
	struct legion_wmi_fan_table_read table;
	size_t len = 0;
	int ret;

	if (priv->legacy_platform)
		return -EOPNOTSUPP;

	ret = legion_wmi_dev_evaluate_buffer(priv->wdev, 0,
					     WMI_METHOD_ID_FAN_GET_TABLE,
					     (u8 *)&input, sizeof(input), buffer,
					     sizeof(buffer), &len);
	if (ret)
		return ret;

	/* Fewer bytes than the fan table: not a fan table */
	if (len < sizeof(table))
		return -EIO;

	memcpy(&table, buffer, sizeof(table));

	for (int i = 0; i < MAX_FANCURVE_SIZE; i++)
		points[i] = le32_to_cpu(table.fss[i]);

	return 0;
}

/**
 * legion_wmi_fm_table_set() - Write the fan table of the custom mode (Fan_Set_Table)
 * @priv: The fan method driver data.
 * @points: Points of the fan table, levels 0..MAX_FANCURVE_LEVEL.
 *
 * The firmware uses the fan table in the custom mode only.
 *
 * Return: 0 on success, -EBUSY out of the custom mode, -EINVAL for a level out of range, or an error code.
 */
int legion_wmi_fm_table_set(struct legion_wmi_fm_priv *priv,
			    const u8 points[MAX_FANCURVE_SIZE])
{
	struct legion_wmi_fan_table_write table = { .fstm = 1 };
	enum thermal_mode mode;
	int ret = legion_wmi_fm_notifier_call(&mode,
					      LEGION_WMI_GZ_GET_THERMAL_MODE);

	if (ret)
		return ret;

	if (mode != LEGION_WMI_GZ_THERMAL_MODE_CUSTOM)
		return -EBUSY;

	for (int i = 0; i < MAX_FANCURVE_SIZE; i++) {
		if (points[i] > MAX_FANCURVE_LEVEL)
			return -EINVAL;

		table.fss[i] = cpu_to_le16(points[i]);
	}

	return legion_wmi_dev_evaluate_buffer(priv->wdev, 0,
					      WMI_METHOD_ID_FAN_SET_TABLE,
					      (u8 *)&table, sizeof(table), NULL,
					      0, NULL);
}

/**
 * legion_wmi_fm_full_speed_get() - Fan full speed of the older firmware (Fan_Get_FullSpeed)
 * @priv: The fan method driver data.
 * @on: Fan full speed is on.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_fm_full_speed_get(struct legion_wmi_fm_priv *priv, bool *on)
{
	u32 value = 0;
	const int ret = legion_wmi_dev_evaluate_int(
		priv->wdev, 0, WMI_METHOD_ID_FAN_GET_FULLSPEED, NULL, 0, &value);

	if (ret)
		return ret;

	*on = value;
	return 0;
}

/**
 * legion_wmi_fm_full_speed_set() - Fan full speed of the older firmware (Fan_Set_FullSpeed)
 * @priv: The fan method driver data.
 * @on: Fan full speed on / off.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_fm_full_speed_set(struct legion_wmi_fm_priv *priv, bool on)
{
	u8 argument = on;

	return legion_wmi_dev_evaluate_int(priv->wdev, 0,
					   WMI_METHOD_ID_FAN_SET_FULLSPEED,
					   &argument, sizeof(argument), NULL);
}

/*
 * Fan full speed is supported on the older platform only (the newer firmware has no Fan_Get_FullSpeed), not with
 * an old BIOS (part of the custom mode) and only when the firmware answers (e.g. JYCN fails in the WMI method, see
 * legion-laptop)
 */
static bool legion_wmi_fm_full_speed_supported(struct legion_wmi_fm_priv *priv)
{
	bool on;
	int ret;

	if (!priv->legacy_platform ||
	    !legion_wmi_platform_bios_supported(&priv->wdev->dev))
		return false;

	ret = legion_wmi_fm_full_speed_get(priv, &on);
	if (ret) {
		dev_dbg(&priv->wdev->dev,
			"Fan_Get_FullSpeed failed (%d), fan full speed is not supported\n",
			ret);
		return false;
	}

	return true;
}

/*
 * Maximal fan speed of a row of the fan table (fan, sensor), 0 when the table has no such row
 */
static int legion_wmi_fm_max_speed(const struct legion_wmi_fm_priv *priv,
				   int fan_id, int sensor_id)
{
	for (int i = 0; i < FAN_MAX; ++i)
		if (priv->fan_max_speeds[i].fan_id == fan_id &&
		    priv->fan_max_speeds[i].sensor_id == sensor_id)
			return priv->fan_max_speeds[i].max_speed;

	return 0;
}

/*
 * Maximal fan speeds for the hwmon from the rows of the fan table. Older firmware: the rows of Lenovo Legion
 * Toolkit (CPU fan 0, GPU fan 1 with the sensors 3 / 4, otherwise both with the sensor 0)
 */
static void legion_wmi_fm_max_speeds(struct legion_wmi_fm_priv *priv)
{
	if (priv->legacy_platform) {
		for (int i = 0;
		     i < FAN_MAX && priv->fan_max_speeds[i].fan_id != -1; ++i)
			dev_dbg(&priv->wdev->dev,
				"fan table: fan %d, sensor %d, max speed %d\n",
				priv->fan_max_speeds[i].fan_id,
				priv->fan_max_speeds[i].sensor_id,
				priv->fan_max_speeds[i].max_speed);

		priv->fan_cpu_max_speed = legion_wmi_fm_max_speed(priv, 0, 3);
		priv->fan_gpu_max_speed = legion_wmi_fm_max_speed(priv, 1, 4);

		if (!priv->fan_cpu_max_speed && !priv->fan_gpu_max_speed) {
			priv->fan_cpu_max_speed =
				legion_wmi_fm_max_speed(priv, 0, 0);
			priv->fan_gpu_max_speed =
				legion_wmi_fm_max_speed(priv, 1, 0);
		}

		priv->fan_sys_max_speed = 0;
	} else if (priv->smart_fan_version == 8) {
		priv->fan_cpu_max_speed = legion_wmi_fm_max_speed(
			priv, fan_table_id_v2[CPU], fan_sensor_id_v2[CPU]);
		priv->fan_gpu_max_speed = legion_wmi_fm_max_speed(
			priv, fan_table_id_v2[GPU], fan_sensor_id_v2[GPU]);
		priv->fan_sys_max_speed = legion_wmi_fm_max_speed(
			priv, fan_table_id_v2[SYS], fan_sensor_id_v2[SYS]);
	} else {
		priv->fan_cpu_max_speed = legion_wmi_fm_max_speed(
			priv, fan_table_id_v1[CPU], fan_sensor_id_v1[CPU]);
		priv->fan_gpu_max_speed = legion_wmi_fm_max_speed(
			priv, fan_table_id_v1[GPU], fan_sensor_id_v1[GPU]);
	}
}

/*
 * Properties of the firmware, before the sysfs and the hwmon use them (after the components are bound)
 */
static int legion_wmi_fm_detect(struct legion_wmi_fm_priv *priv)
{
	int err = legion_wmi_fm_notifier_call(
		&priv->smart_fan_version, LEGION_WMI_GZ_GET_SMARTFAN_VERSION);
	if (err)
		return err;

	/* Older firmware: fan full speed and sensors by the fan method, no reading of the fan table */
	priv->legacy_platform = legion_wmi_platform_legacy(&err);
	if (err)
		return err;

	priv->fan_full_speed = legion_wmi_fm_full_speed_supported(priv);
	legion_wmi_fm_max_speeds(priv);

	return 0;
}

static int legion_wmi_fm_master_bind(struct device *dev)
{
	struct legion_wmi_fm_priv *priv = dev_get_drvdata(dev);
	int ret = component_bind_all(dev, &priv);

	if (ret)
		return ret;

	ret = legion_wmi_fm_detect(priv);
	if (ret)
		goto sysfs_err;

	ret = legion_wmi_fm_sysfs_init(priv);
	if (ret)
		goto sysfs_err;

	priv->hwmon_nb.notifier_call = legion_wmi_fm_call;
	ret = legion_hwmon_fm_register_notifier(&priv->hwmon_nb);
	if (ret)
		goto notifier_err;

	/* Older firmware: source of the sensors and owner of the hwmon (the other mode does not provide them there) */
	if (priv->legacy_platform) {
		priv->sensors_nb.notifier_call = legion_wmi_fm_sensors_call;
		priv->hwmon = legion_hwmon_register(dev, &priv->sensors_nb);
		if (IS_ERR(priv->hwmon)) {
			ret = PTR_ERR(priv->hwmon);
			priv->hwmon = NULL;
			goto hwmon_err;
		}
	}

	return 0;
hwmon_err:
	legion_hwmon_fm_unregister_notifier(&priv->hwmon_nb);
notifier_err:
	legion_wmi_fm_sysfs_exit(priv);
sysfs_err:
	component_unbind_all(dev, NULL);
	return ret;
}

static void legion_wmi_fm_master_unbind(struct device *dev)
{
	struct legion_wmi_fm_priv *priv = dev_get_drvdata(dev);

	legion_hwmon_unregister(priv->hwmon);
	priv->hwmon = NULL;
	legion_hwmon_fm_unregister_notifier(&priv->hwmon_nb);
	legion_wmi_fm_sysfs_exit(priv);

	component_unbind_all(dev, NULL);

	/* The fan table belongs to its component (device managed), not valid after the unbind */
	priv->ft_list = NULL;
}

static const struct component_master_ops legion_wmi_fm_master_ops = {
	.bind = legion_wmi_fm_master_bind,
	.unbind = legion_wmi_fm_master_unbind,
};

static const struct wmi_device_id legion_fm_other_id_table[] = {
	{ WMI_GUID_LENOVO_FAN_METHOD, NULL },
	{}
};

static int legion_fm_other_probe(struct wmi_device *wdev, const void *context)
{
	struct component_match *master_match = NULL;
	struct legion_wmi_fm_priv *priv =
		devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;

	dev_set_drvdata(&wdev->dev, priv);

	component_match_add(&wdev->dev, &master_match, legion_wmi_ftable_match,
			    NULL);
	if (IS_ERR(master_match))
		return (int)PTR_ERR(master_match);

	/*
	 * Component master of the fan table: bound when it is probed (registered before the fan method by
	 * module_init, so at once)
	 */
	return component_master_add_with_match(
		&wdev->dev, &legion_wmi_fm_master_ops, master_match);
}

static void legion_fm_other_remove(struct wmi_device *wdev)
{
	component_master_del(&wdev->dev, &legion_wmi_fm_master_ops);
}

static struct wmi_driver legion_fm_other_driver = {
	.driver = {
		.name = "legion_wmi_fan_method",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_fm_other_id_table,
	.probe = legion_fm_other_probe,
	.remove = legion_fm_other_remove,
	.no_singleton = true,
};

int legion_wmi_fm_driver_init(void)
{
	return wmi_driver_register(&legion_fm_other_driver);
}

void legion_wmi_fm_driver_exit(void)
{
	wmi_driver_unregister(&legion_fm_other_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_fm_other_id_table);
