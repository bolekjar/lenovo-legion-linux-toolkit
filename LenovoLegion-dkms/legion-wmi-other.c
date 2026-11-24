// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the lenovo-wmi-other driver of the Linux kernel (drivers/platform/x86/lenovo/wmi-other.c),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */
#include "legion-wmi-other.h"
#include "legion-wmi-capdata01.h"
#include "legion-wmi-capdata00.h"
#include "legion-wmi-ddata.h"
#include "legion-wmi-other-sysfs.h"
#include "legion-wmi-gamezone.h"
#include "legion-hwmon.h"
#include "legion-wmi-platform.h"
#include "legion-wmi-helpers.h"

#include <linux/module.h>
#include <linux/device.h>
#include <linux/wmi.h>
#include <linux/component.h>
#include <linux/bitfield.h>

#define LEGION_WMI_LENOVO_OTHER_METHOD_GUID \
	"DC2A8805-3A8C-41BA-A6F7-092E0089CD3B"

static BLOCKING_NOTIFIER_HEAD(legion_other_chain_head);

static const struct wmi_device_id legion_wmi_other_id_table[] = {
	{ LEGION_WMI_LENOVO_OTHER_METHOD_GUID, NULL },
	{}
};

/**
 * lenovo_wmi_other_register_notifier() - Add a notifier to the blocking notifier chain
 * @nb: The notifier_block struct to register
 *
 * Call blocking_notifier_chain_register to register the notifier block to the
 * lenovo-wmi-other driver notifier chain.
 *
 * Return: 0 on success, %-EEXIST on error.
 */
static int lenovo_wmi_other_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&legion_other_chain_head, nb);
}

/**
 * lenovo_wmi_other_unregister_notifier() - Remove a notifier from the blocking notifier
 * chain.
 * @nb: The notifier_block struct to register
 *
 * Call blocking_notifier_chain_unregister to unregister the notifier block from the
 * lenovo-wmi-other driver notifier chain.
 *
 * Return: 0 on success, %-ENOENT on error.
 */
static int lenovo_wmi_other_unregister_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&legion_other_chain_head, nb);
}

/**
 * devm_lenovo_wmi_other_unregister_notifier() - Remove a notifier from the blocking
 * notifier chain.
 * @data: Void pointer to the notifier_block struct to register.
 *
 * Call lwmi_om_unregister_notifier to unregister the notifier block from the
 * lenovo-wmi-other driver notifier chain.
 *
 * Return: 0 on success, %-ENOENT on error.
 */
static void devm_lenovo_wmi_other_unregister_notifier(void *data)
{
	struct notifier_block *nb = data;

	lenovo_wmi_other_unregister_notifier(nb);
}

/**
 * devm_lenovo_wmi_other_register_notifier() - Add a notifier to the blocking notifier
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
int devm_lenovo_wmi_other_register_notifier(struct device *dev,
					    struct notifier_block *nb)
{
	const int ret = lenovo_wmi_other_register_notifier(nb);

	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(
		dev, devm_lenovo_wmi_other_unregister_notifier, nb);
}

int legion_wmi_other_notifier_call(
	void *data, const enum gamezone_events_type gamezone_events_type)
{
	const int ret = blocking_notifier_call_chain(
		&legion_other_chain_head, gamezone_events_type, &data);
	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

/**
 * legion_wmi_other_feature_get() - Current value of a feature (GetFeatureValue)
 * @priv: The other mode driver data.
 * @feature_id: Capability ID of the feature, its mode is ignored.
 * @value: The value.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_other_feature_get(const struct legion_wmi_other_priv *priv,
				 u32 feature_id, u32 *value)
{
	struct wmi_method_args_32 args = {
		.arg0 = feature_id & ~LEGION_WMI_MODE_ID_MASK,
	};

	return legion_wmi_dev_evaluate_int(priv->wdev, 0x0,
					   LEGION_WMI_OTHER_FEATURE_VALUE_GET,
					   (unsigned char *)&args, sizeof(args),
					   value);
}

/*
 * Write of a checked value of a feature (SetFeatureValue)
 */
static int legion_wmi_other_feature_write(const struct legion_wmi_other_priv *priv,
					  u32 feature_id, u32 value)
{
	struct wmi_method_args_32 args = {
		.arg0 = feature_id & ~LEGION_WMI_MODE_ID_MASK,
		.arg1 = value,
	};

	return legion_wmi_dev_evaluate_int(priv->wdev, 0x0,
					   LEGION_WMI_OTHER_FEATURE_VALUE_SET,
					   (unsigned char *)&args, sizeof(args),
					   NULL);
}

/*
 * Value of a feature without a range: one of the discrete values of the custom mode (discrete data)
 */
static int legion_wmi_other_feature_discrete(const struct legion_wmi_other_priv *priv,
					     u32 feature_id_custom, u32 value)
{
	struct legion_wmi_ddata lddata[48] = { { 0, 0 } };
	int ret;

	ret = legion_wmi_dd_get_data(priv->dd_list, feature_id_custom, lddata,
				     ARRAY_SIZE(lddata) - 1);
	if (ret)
		return ret;

	for (int i = 0; i < ARRAY_SIZE(lddata) && lddata[i].id != 0; ++i)
		if (lddata[i].value == value)
			return 0;

	return -EINVAL;
}

/**
 * legion_wmi_other_feature_set() - Set a feature of the custom mode (capability data 01)
 * @priv: The other mode driver data.
 * @feature_id: Capability ID of the feature, its mode is ignored.
 * @value: The value.
 *
 * The firmware uses the value in the custom mode only. The value is checked by
 * the capability data 01 of the custom mode: its range, or one of the discrete
 * values when the feature has no range.
 *
 * Return: 0 on success, -EBUSY out of the custom mode, -ENODEV for a feature not
 * supported, -EOPNOTSUPP for a feature not writable, -EINVAL for a value out of
 * range, or an error code.
 */
int legion_wmi_other_feature_set(const struct legion_wmi_other_priv *priv,
				 u32 feature_id, u32 value)
{
	const u32 feature_id_custom =
		(feature_id & ~LEGION_WMI_MODE_ID_MASK) |
		FIELD_PREP(LEGION_WMI_MODE_ID_MASK,
			   LEGION_WMI_GZ_THERMAL_MODE_CUSTOM);
	struct legion_wmi_capdata01 capdata;
	enum thermal_mode mode;
	int ret;

	ret = legion_wmi_other_notifier_call(&mode,
					     LEGION_WMI_GZ_GET_THERMAL_MODE);
	if (ret)
		return ret;

	if (mode != LEGION_WMI_GZ_THERMAL_MODE_CUSTOM)
		return -EBUSY;

	ret = legion_wmi_cd01_get_data(priv->cd01_list, feature_id_custom,
				       &capdata);
	if (ret)
		return ret;

	if (!(capdata.supported & LEGION_WMI_SUPP_VALID))
		return -ENODEV;

	/* Not writable by the firmware (in the custom mode) */
	if (!(capdata.supported & LEGION_WMI_SUPP_SET))
		return -EOPNOTSUPP;

	if (capdata.max_value == 0 && capdata.min_value == 0 &&
	    capdata.step == 0)
		ret = legion_wmi_other_feature_discrete(priv, feature_id_custom,
							value);
	else if (value < capdata.min_value || value > capdata.max_value)
		ret = -EINVAL;

	if (ret)
		return ret;

	return legion_wmi_other_feature_write(priv, feature_id, value);
}

/**
 * legion_wmi_other_feature_set_range() - Set a feature (capability data 00) in a range
 * @priv: The other mode driver data.
 * @feature_id: Capability ID of the feature, its mode is ignored.
 * @value: The value.
 * @min: Minimal value.
 * @max: Maximal value.
 *
 * Return: 0 on success, -ENODEV for a feature not supported, -EOPNOTSUPP for a
 * feature not writable, -EINVAL for a value out of range, or an error code.
 */
int legion_wmi_other_feature_set_range(const struct legion_wmi_other_priv *priv,
				       u32 feature_id, u32 value, u32 min,
				       u32 max)
{
	struct legion_wmi_capdata00 capdata = { 0, 0, 0 };
	int ret;

	/* A feature not in the capability data (-EINVAL) is not supported */
	ret = legion_wmi_cd00_get_data(priv->cd00_list,
				       feature_id & ~LEGION_WMI_MODE_ID_MASK,
				       &capdata);
	if (ret && ret != -EINVAL)
		return ret;

	if (!(capdata.supported & LEGION_WMI_SUPP_VALID))
		return -ENODEV;

	if (!(capdata.supported & LEGION_WMI_SUPP_SET))
		return -EOPNOTSUPP;

	if (value < min || value > max)
		return -EINVAL;

	return legion_wmi_other_feature_write(priv, feature_id, value);
}

static int legion_wmi_other_call(struct notifier_block *nb,
				 const unsigned long action, void *data)
{
	const struct legion_wmi_other_priv *priv =
		container_of(nb, struct legion_wmi_other_priv, hwmon_nb);
	const struct hwmon_capability *value = data;

	switch (action) {
	case LEGION_WMI_FM_GET_CAPABILITY_VALUE:
		if (legion_wmi_other_feature_get(priv, value->capability_id,
						 value->data))
			return NOTIFY_BAD;

		return NOTIFY_OK;
	case LEGION_WMI_FM_GET_CAPABILITY_DATA: {
		const int ret = legion_wmi_cd00_get_data(
			priv->cd00_list,
			value->capability_id & (~LEGION_WMI_MODE_ID_MASK),
			value->data);
		if (ret && ret != -EINVAL)
			return NOTIFY_BAD;
		return NOTIFY_OK;
	} break;
	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_DONE;
}

/**
 * legion_wmi_om_master_bind() - Bind all components of the other mode driver
 * @dev: The lenovo-wmi-other driver basic device.
 *
 * Call component_bind_all to bind the lenovo-wmi-capdata01 driver to the
 * lenovo-wmi-other master driver. On success, assign the capability data 01
 * list pointer to the driver data struct for later access. This pointer
 * is only valid while the capdata01 interface exists. Finally, register all
 * firmware attribute groups.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_om_master_bind(struct device *dev)
{
	struct legion_wmi_other_priv *priv = dev_get_drvdata(dev);
	int ret = component_bind_all(dev, &priv);

	if (ret)
		return ret;

	ret = legion_wmi_other_sysfs_init(priv);
	if (ret)
		goto sysfs_err;

	/*
	 * Source of the sensors and owner of the hwmon, not on the older platform (the fan method provides the
	 * sensors there, the same as Lenovo Legion Toolkit)
	 */
	priv->hwmon_nb.notifier_call = legion_wmi_other_call;
	if (!legion_wmi_platform_legacy(&ret)) {
		priv->hwmon = legion_hwmon_register(dev, &priv->hwmon_nb);
		if (IS_ERR(priv->hwmon)) {
			ret = PTR_ERR(priv->hwmon);
			priv->hwmon = NULL;
			goto hwmon_err;
		}
	}

	return 0;

hwmon_err:
	legion_wmi_other_sysfs_exit(priv);
sysfs_err:
	component_unbind_all(dev, NULL);
	return ret;
}

/**
 * legion_wmi_om_master_unbind() - Unbind all components of the other mode driver
 * @dev: The lenovo-wmi-other driver basic device
 *
 * Unregister all capability data attribute groups. Then call
 * component_unbind_all to unbind the lenovo-wmi-capdata01 driver from the
 * lenovo-wmi-other master driver. Finally, free the IDA for this device.
 */
static void legion_wmi_om_master_unbind(struct device *dev)
{
	struct legion_wmi_other_priv *priv = dev_get_drvdata(dev);

	legion_hwmon_unregister(priv->hwmon);
	priv->hwmon = NULL;

	legion_wmi_other_sysfs_exit(priv);

	component_unbind_all(dev, NULL);

	/* The lists belong to the components (device managed), not valid after the unbind */
	priv->cd00_list = NULL;
	priv->cd01_list = NULL;
	priv->dd_list = NULL;
}

static const struct component_master_ops legion_wmi_om_master_ops = {
	.bind = legion_wmi_om_master_bind,
	.unbind = legion_wmi_om_master_unbind,
};

/**
 * legion_wmi_other_chain_call() - Answer the requests of the other drivers
 * @nb: The notifier_block of the other mode driver.
 * @action: The request, LEGION_WMI_OM_GET_LEGIONZONE_VERSION.
 * @data: Pointer to the pointer of the unsigned int result.
 *
 * Available from the probe, also when the capability data are missing (older
 * firmware). The LegionZone version is read in the same order as Lenovo Legion
 * Toolkit: GetFeatureValue(LegionZoneSupportVersion), otherwise
 * Get_Support_LegionZone_Version.
 *
 * Called before the game zone (higher priority), the chain returns the result of the
 * last called block, so the request is answered with NOTIFY_STOP.
 *
 * Return: NOTIFY_STOP on success, NOTIFY_BAD on a failure, NOTIFY_DONE for other requests.
 */
static int legion_wmi_other_chain_call(struct notifier_block *nb,
				       const unsigned long action, void *data)
{
	const struct legion_wmi_other_priv *priv =
		container_of(nb, struct legion_wmi_other_priv, other_nb);
	unsigned int **version = data;
	u32 value = 0;

	if (action != LEGION_WMI_OM_GET_LEGIONZONE_VERSION)
		return NOTIFY_DONE;

	if (!legion_wmi_other_feature_get(priv, LegionZoneSupportVersion,
					  &value) &&
	    value) {
		**version = value;
		return NOTIFY_STOP;
	}

	if (legion_wmi_dev_evaluate_int(priv->wdev, 0x0,
					LEGION_WMI_OTHER_LEGIONZONE_VERSION_GET,
					NULL, 0, &value))
		return NOTIFY_BAD;

	**version = value;
	return NOTIFY_STOP;
}

static int legion_wmi_other_probe(struct wmi_device *wdev, const void *context)
{
	struct component_match *master_match = NULL;
	struct legion_wmi_other_priv *priv = NULL;
	bool legacy;
	int ret;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;

	dev_set_drvdata(&wdev->dev, priv);

	priv->other_nb.notifier_call = legion_wmi_other_chain_call;
	priv->other_nb.priority = 1;
	ret = devm_lenovo_wmi_other_register_notifier(&wdev->dev,
						      &priv->other_nb);
	if (ret)
		return ret;

	/*
	 * Older platform detected once, now that the LegionZone version is answered, and kept by the game zone for
	 * all drivers (probed after the game zone and before the fan method and the CPU / GPU method)
	 */
	legacy = legion_wmi_platform_detect(&wdev->dev, &ret);
	if (!ret && legion_wmi_other_notifier_call(&legacy,
						   LEGION_WMI_GZ_SET_LEGACY_PLATFORM))
		dev_warn(&wdev->dev, "Older platform not passed to the game zone\n");

	component_match_add(&wdev->dev, &master_match, legion_wmi_cd00_match,
			    NULL);
	if (IS_ERR(master_match))
		return (int)PTR_ERR(master_match);

	component_match_add(&wdev->dev, &master_match, legion_wmi_cd01_match,
			    NULL);
	if (IS_ERR(master_match))
		return (int)PTR_ERR(master_match);

	component_match_add(&wdev->dev, &master_match, legion_wmi_dd_match,
			    NULL);
	if (IS_ERR(master_match))
		return (int)PTR_ERR(master_match);

	/*
	 * Component master of the capability data 00, 01 and discrete data: bound when all of them are probed
	 * (registered before the other mode by module_init, so at once)
	 */
	return component_master_add_with_match(
		&wdev->dev, &legion_wmi_om_master_ops, master_match);
}

static void legion_wmi_other_remove(struct wmi_device *wdev)
{
	/* Remove as a component master first (this calls unbind) */
	component_master_del(&wdev->dev, &legion_wmi_om_master_ops);
}

static struct wmi_driver legion_wmi_other_driver = {
	.driver = {
		.name = "legion_wmi_other",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_other_id_table,
	.probe = legion_wmi_other_probe,
	.remove = legion_wmi_other_remove,
	.no_singleton = true,
};

int legion_wmi_other_driver_init(void)
{
	return wmi_driver_register(&legion_wmi_other_driver);
}

void legion_wmi_other_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_other_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_other_id_table);
