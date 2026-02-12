// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-wmi-gamezone.h"
#include "legion-wmi-events.h"
#include "legion-wmi-helpers.h"
#include "legion-wmi-gamezone-sysfs.h"
#include "legion-wmi-other.h"
#include "legion-wmi-fm.h"
#include "legion-hwmon.h"

#include <linux/acpi.h>
#include <linux/dmi.h>
#include <linux/export.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/wmi.h>

#define LEGION_GAMEZONE_GUID "887B54E3-DDDC-4B2C-8B88-68A26A8835D0"

struct quirk_entry {
	bool extreme_supported;
};

static struct quirk_entry quirk_no_extreme_bug = {
	.extreme_supported = false,
};

static const struct dmi_system_id fwbug_list[] = {
	{
		.ident = "Legion Go 8APU1",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
			DMI_MATCH(DMI_PRODUCT_VERSION, "Legion Go 8APU1"),
		},
		.driver_data = &quirk_no_extreme_bug,
	},
	{
		.ident = "Legion Go S 8APU1",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
			DMI_MATCH(DMI_PRODUCT_VERSION, "Legion Go S 8APU1"),
		},
		.driver_data = &quirk_no_extreme_bug,
	},
	{
		.ident = "Legion Go S 8ARP1",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
			DMI_MATCH(DMI_PRODUCT_VERSION, "Legion Go S 8ARP1"),
		},
		.driver_data = &quirk_no_extreme_bug,
	},
	{},

};


static bool legion_is_ac_connected(const enum power_adapter_status adapter_status)
{
	return (adapter_status == LEGION_WMI_GZ_AC_CONNECTED) || (adapter_status == LEGION_WMI_GZ_AC_CONNECTED_LOW_WATTAGE);
}

static enum thermal_mode  legion_current_mode(const struct lenovo_wmi_gz_priv *priv)
{
	if(legion_is_ac_connected(priv->current_adapter_status))
	{
		return priv->current_mode_on_ac;
	}

	return priv->current_mode_on_battery;
}

static void legion_modify_current_mode(struct lenovo_wmi_gz_priv *priv,const enum thermal_mode mode)
{
	guard(spinlock)(&priv->gz_mode_lock);

	if(legion_is_ac_connected(priv->current_adapter_status))
	{
		priv->current_mode_on_ac = mode;
		return;
	}

	priv->current_mode_on_battery = mode;
}

int legion_wmi_gz_get(struct wmi_device *wdev,enum LEGION_GAMEZONE_METHOD_ID method_id,u32 *value) {
	return legion_wmi_dev_evaluate_int(wdev, 0x0, method_id,
				     NULL, 0, value);
}

int legion_wmi_gz_set(struct wmi_device *wdev,enum LEGION_GAMEZONE_METHOD_ID method_id,u32 value) {
	struct wmi_method_args_32 args = {
			value,
			0
	};
	return legion_wmi_dev_evaluate_int(wdev, 0x0,method_id,(u8 *)&args, sizeof(args), NULL);
}

int legion_wmi_gz_get_string(struct wmi_device *wdev,enum LEGION_GAMEZONE_METHOD_ID method_id,char *retval,size_t retval_size){
	return legion_wmi_dev_evaluate_string(wdev, 0x0, method_id,NULL, 0, retval,retval_size);
}

/**
 * legion_wmi_gz_call() - Call method for lenovo-wmi-other driver notifier.
 *
 * @nb: The notifier_block registered to lenovo-wmi-other driver.
 * @cmd: The event type.
 *
 *
 * Return: Notifier_block status.
 */
static int legion_wmi_gz_call(struct lenovo_wmi_gz_priv *priv, unsigned long cmd,void *data)
{
	switch (cmd) {
	case LEGION_WMI_GZ_GET_THERMAL_MODE: {
		enum thermal_mode **mode = data;
		scoped_guard(spinlock, &priv->gz_mode_lock) {
			**mode = legion_current_mode(priv);
		}
	}
		return NOTIFY_OK;
	case LEGION_WMI_GZ_GET_SUPPORTED_THERMAL_MODES: {
		enum thermal_mode **mode = data;
		scoped_guard(spinlock, &priv->gz_mode_lock) {
			(*mode)[0] =  LEGION_WMI_GZ_THERMAL_MODE_QUIET;
			(*mode)[1] =  LEGION_WMI_GZ_THERMAL_MODE_BALANCED;
			(*mode)[2] =  LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE;
			(*mode)[3] =  LEGION_WMI_GZ_THERMAL_MODE_CUSTOM;
			if(priv->extreme_supported)
			{
				(*mode)[4] =  LEGION_WMI_GZ_THERMAL_MODE_EXTREME;
				(*mode)[5] = LEGION_WMI_GZ_THERMAL_MODE_END;
			}
			else
			{
				(*mode)[4] = LEGION_WMI_GZ_THERMAL_MODE_END;
			}
		}
	}
		return NOTIFY_OK;
	case LEGION_WMI_GZ_GET_SMARTFAN_VERSION: {
		int **version = data ;
		scoped_guard(spinlock, &priv->gz_mode_lock) {
			**version = priv->preloaded_values.IsSupportSmartFan;
		}
	}
		return NOTIFY_OK;

	default:
		return NOTIFY_DONE;
	}
}


static int legion_wmi_other_gz_call(struct notifier_block *nb, unsigned long cmd,void *data){
	return legion_wmi_gz_call(container_of(nb, struct lenovo_wmi_gz_priv, other_nb),cmd,data);
}

static int legion_wmi_fm_gz_call(struct notifier_block *nb, unsigned long cmd,void *data){
	return legion_wmi_gz_call(container_of(nb, struct lenovo_wmi_gz_priv, fm_nb),cmd,data);
}

/**
 * legion_wmi_gz_extreme_supported() - Evaluate if a device supports extreme thermal mode.
 * @profile_support_ver: Version of the WMI interface.
 *
 * Determine if the extreme thermal mode is supported by the hardware.
 * Anything version 5 or lower does not. For devices with a version 6 or
 * greater do a DMI check, as some devices report a version that supports
 * extreme mode but have an incomplete entry in the BIOS. To ensure this
 * cannot be set, quirk them to prevent assignment.
 *
 * Return: bool.
 */
static bool legion_wmi_gz_extreme_supported(int profile_support_ver)
{
	const struct dmi_system_id *dmi_id;
	struct quirk_entry *quirks;

	if (profile_support_ver < 6)
		return false;

	dmi_id = dmi_first_match(fwbug_list);
	if (!dmi_id)
		return true;

	quirks = dmi_id->driver_data;

	return quirks->extreme_supported;
}

/**
 * legion_wmi_gz_thermal_mode_get() - Get the current thermal mode.
 * @wdev: The Gamezone interface WMI device.
 * @mode: Pointer to return the thermal mode with.
 *
 * Return: 0 on success, or an error code.
 */
/*static int legion_wmi_gz_thermal_mode_get(struct wmi_device *wdev,
				    enum thermal_mode *mode)
{
	return legion_wmi_dev_evaluate_int(wdev, 0x0, WMI_METHOD_ID_GETSMARTFANMODE,
				     NULL, 0, mode);
}*/


/**
 * legion_wmi_gz_charging_mode_get() - Get the current Charging mode.
 * interface to determine the support level.
 * @wdev: The Gamezone WMI device.
 * @supported: Pointer to return the support level with.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_gz_charging_mode_get(struct wmi_device *wdev,
					  enum power_adapter_status *mode)
{
	return legion_wmi_gz_get(wdev,GetPowerChargeMode,mode);
}

/**
 * legion_wmi_gz_thermal_mode_set() - Set the current thermal mode.
 * interface to determine the support level.
 * @wdev: The Gamezone WMI device.
 * @supported: Pointer to return the support level with.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_gz_thermal_mode_set(struct wmi_device *wdev,enum thermal_mode mode)
{
	return legion_wmi_gz_set(wdev, SetSmartFanMode,mode);
}


/**
 * legion_wmi_gz_event_call() - Call method for lenovo-wmi-events driver notifier.
 * block call chain.
 * @nb: The notifier_block registered to lenovo-wmi-events driver.
 * @cmd: The event type.
 * @data: The data to be updated by the event.
 *
 * For LWMI_EVENT_THERMAL_MODE, set current_mode and notify platform_profile
 * of a change.
 *
 * Return: notifier_block status.
 */
static int legion_wmi_gz_event_call(struct notifier_block *nb, unsigned long cmd,
			      void *data)
{
	struct lenovo_wmi_gz_priv *priv = container_of(nb, struct lenovo_wmi_gz_priv, event_nb);

	switch (cmd) {
	case LEGION_WMI_EVENT_THERMAL_MODE:
	{
		legion_modify_current_mode(priv,*(enum thermal_mode *)data);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
		platform_profile_notify(priv->ppdev);
#else
		platform_profile_notify();
#endif
		return NOTIFY_OK;
	}
	case LEGION_WMI_EVENT_POWER_CHARGE_MODE:
	{
		/* Just update the adapter status and notify user space.
		 * Do NOT call WMI methods from within event callbacks as this
		 * can cause deadlocks or freezes. The thermal mode will be
		 * applied when user space requests a profile change.
		 */
		scoped_guard(spinlock, &priv->gz_mode_lock) {
			priv->current_adapter_status = *(enum power_adapter_status *)data;
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
		platform_profile_notify(priv->ppdev);
#else
		platform_profile_notify();
#endif
		return NOTIFY_OK;
	}
	default:
		return NOTIFY_DONE;
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)

/**
 * legion_wmi_gz_profile_get() - Get the current platform profile.
 * @dev: the Gamezone interface parent device.
 * @profile: Pointer to provide the current platform profile with.
 *
 * Call lwmi_gz_thermal_mode_get and convert the thermal mode into a platform
 * profile based on the support level of the interface.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_gz_profile_get(struct device *dev,enum platform_profile_option *profile)
{
	switch (legion_current_mode(dev_get_drvdata(dev))) {
	case LEGION_WMI_GZ_THERMAL_MODE_QUIET:
		*profile = PLATFORM_PROFILE_QUIET;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_BALANCED:
		*profile = PLATFORM_PROFILE_BALANCED;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE:
		*profile = PLATFORM_PROFILE_PERFORMANCE;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_EXTREME:
		*profile = PLATFORM_PROFILE_BALANCED_PERFORMANCE;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_CUSTOM:
		*profile = PLATFORM_PROFILE_CUSTOM;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * legion_wmi_gz_profile_set() - Set the current platform profile.
 * @dev: The Gamezone interface parent device.
 * @profile: Pointer to the desired platform profile.
 *
 * Convert the given platform profile into a thermal mode based on the support
 * level of the interface, then call the WMI method to set the thermal mode.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_gz_profile_set(struct device *dev,enum platform_profile_option profile)
{
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(dev);
	enum thermal_mode mode;
	int ret;

	switch (profile) {
	case PLATFORM_PROFILE_QUIET:
		mode = LEGION_WMI_GZ_THERMAL_MODE_QUIET;
		break;
	case PLATFORM_PROFILE_BALANCED:
		mode = LEGION_WMI_GZ_THERMAL_MODE_BALANCED;
		break;
	case PLATFORM_PROFILE_PERFORMANCE:
		if(legion_is_ac_connected(priv->current_adapter_status))
		{
			mode = LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE;
		}
		else
		{
			mode = LEGION_WMI_GZ_THERMAL_MODE_BALANCED;
		}
		break;
	default:
		return -EOPNOTSUPP;
	}

	ret = legion_wmi_gz_thermal_mode_set(priv->wdev,mode);
	if (ret)
		return ret;

	legion_modify_current_mode(priv,mode);

	return 0;
}


/**
 * legion_wmi_gz_platform_profile_probe - Enable and set up the platform profile
 * device.
 * @drvdata: Driver data for the interface.
 * @choices: Container for enabled platform profiles.
 *
 * Determine if thermal mode is supported, and if so to what feature level.
 * Then enable all supported platform profiles.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_gz_platform_profile_probe(void *drvdata, unsigned long *choices)
{
	set_bit(PLATFORM_PROFILE_QUIET,choices);
	set_bit(PLATFORM_PROFILE_BALANCED, choices);
	set_bit(PLATFORM_PROFILE_PERFORMANCE, choices);

	return 0;
}


/**
 * legion_wmi_gz_platform_profile_hidden_choices - Enable and set up the platform profile
 * device.
 * @drvdata: Driver data for the interface.
 * @choices: Container for enabled platform profiles.
 *
 * Determine if thermal mode is supported, and if so to what feature level.
 * Then enable all supported platform profiles.
 *
 * Return: 0 on success, or an error code.
 */
static int legion_wmi_gz_platform_profile_hidden_choices(void *drvdata, unsigned long *choices)
{
	struct lenovo_wmi_gz_priv *priv = drvdata;

	set_bit(PLATFORM_PROFILE_CUSTOM, choices);
	if (priv->extreme_supported)
		set_bit(PLATFORM_PROFILE_BALANCED_PERFORMANCE, choices);

	return 0;
}

static const struct platform_profile_ops legion_wmi_gz_platform_profile_ops = {
	.probe = legion_wmi_gz_platform_profile_probe,
	.hidden_choices = legion_wmi_gz_platform_profile_hidden_choices,
	.profile_get = legion_wmi_gz_profile_get,
	.profile_set = legion_wmi_gz_profile_set
};
#else

static int legion_wmi_gz_profile_get(struct platform_profile_handler *pprof,
				       enum platform_profile_option *profile)
{

	switch (legion_current_mode(container_of(pprof, struct lenovo_wmi_gz_priv, platform_profile_handler))) {
	case LEGION_WMI_GZ_THERMAL_MODE_QUIET:
		*profile = PLATFORM_PROFILE_QUIET;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_BALANCED:
		*profile = PLATFORM_PROFILE_BALANCED;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE:
		*profile = PLATFORM_PROFILE_PERFORMANCE;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_EXTREME:
		*profile = PLATFORM_PROFILE_BALANCED_PERFORMANCE;
		break;
	case LEGION_WMI_GZ_THERMAL_MODE_CUSTOM:
		*profile = PLATFORM_PROFILE_BALANCED_PERFORMANCE;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int legion_wmi_gz_profile_set(struct platform_profile_handler *pprof,
				       enum platform_profile_option profile)
{

	struct lenovo_wmi_gz_priv *priv = container_of(pprof, struct lenovo_wmi_gz_priv, platform_profile_handler);
	enum thermal_mode mode;
	int ret;

	switch (profile) {
	case PLATFORM_PROFILE_QUIET:
		mode = LEGION_WMI_GZ_THERMAL_MODE_QUIET;
		break;
	case PLATFORM_PROFILE_BALANCED:
		mode = LEGION_WMI_GZ_THERMAL_MODE_BALANCED;
		break;
	case PLATFORM_PROFILE_PERFORMANCE:
		if(legion_is_ac_connected(priv->current_adapter_status))
		{
			mode = LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE;
		}
		else
		{
			mode = LEGION_WMI_GZ_THERMAL_MODE_BALANCED;
		}
		break;
	default:
		return -EOPNOTSUPP;
	}

	ret = legion_wmi_gz_thermal_mode_set(priv->wdev,mode);
	if (ret)
		return ret;

	legion_modify_current_mode(priv,mode);

	return 0;
}

#endif

static int legion_wmi_read_static_values(struct wmi_device *wdev,struct game_zone_preloaded_method_values* cache)
{
	int ret = 0;

	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportSmartFan,NULL,0, (u32*)(&cache->IsSupportSmartFan));
	if (ret)
		return ret;

	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportGpuOC,NULL,0,&cache->IsSupportGpuOC);
	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportCpuOC,NULL,0, &cache->IsSupportCpuOC);
	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportDisableWinKey,NULL,0, &cache->IsSupportDisableWinKey);
	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportDisableTP,NULL,0, &cache->IsSupportDisableTP);
	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportGSync,NULL,0, &cache->IsSupportGSync);
	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportOD,NULL,0, &cache->IsSupportOD);
	ret = legion_wmi_dev_evaluate_int(wdev,0x0, IsSupportIGPUMode,NULL,0, &cache->IsSupportIGPUMode);


	ret = legion_wmi_dev_evaluate_int(wdev,0x0, GetFanCount,NULL,0, &cache->GetFanCount);
	ret = legion_wmi_dev_evaluate_int(wdev,0x0, GetFanMaxSpeed,NULL,0, &cache->GetFanMaxSpeed);

	return 0;
}

static int legion_wmi_gz_probe(struct wmi_device *wdev, const void *context)
{
	struct lenovo_wmi_gz_priv *priv;
	int ret;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->preloaded_values.IsSupportCpuOC = -1;
	priv->preloaded_values.IsSupportDisableTP = -1;
	priv->preloaded_values.IsSupportDisableWinKey = -1;
	priv->preloaded_values.IsSupportGSync = -1;
	priv->preloaded_values.IsSupportGpuOC = -1;
	priv->preloaded_values.IsSupportIGPUMode = -1;
	priv->preloaded_values.IsSupportOD = -1;
	priv->preloaded_values.IsSupportSmartFan = -1;
	priv->preloaded_values.GetFanCount = -1;
	priv->preloaded_values.GetFanMaxSpeed = -1;

	priv->current_mode_on_ac 		=  DEFAULT_THERMAL_MODE;
	priv->current_mode_on_battery 	=  DEFAULT_THERMAL_MODE;
	priv->wdev = wdev;


	spin_lock_init(&priv->gz_mode_lock);

	dev_set_drvdata(&wdev->dev, priv);

	ret = legion_wmi_read_static_values(wdev,&priv->preloaded_values);
	if (ret)
		return ret;

	if (priv->preloaded_values.IsSupportSmartFan < 1)
		return -ENODEV;

	ret = legion_wmi_gz_charging_mode_get(wdev, &priv->current_adapter_status);
	if (ret)
		return ret;

	/* Set initial balanced mode - if this fails, device may not support it */
	ret = legion_wmi_gz_thermal_mode_set(wdev,DEFAULT_THERMAL_MODE);
	if (ret) {
		dev_warn(&wdev->dev, "Failed to set initial thermal mode: %d\n", ret);
		/* Continue anyway as this is not fatal */
	}

	priv->extreme_supported = legion_wmi_gz_extreme_supported(priv->preloaded_values.IsSupportSmartFan);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
	priv->ppdev = devm_platform_profile_register(&wdev->dev, "lenovo-wmi-gamezone",
						     priv, &legion_wmi_gz_platform_profile_ops);
	if (IS_ERR(priv->ppdev)) {
		dev_err(&wdev->dev, "Failed to register platform profile\n");
		return PTR_ERR(priv->ppdev);
	}
#else
	priv->platform_profile_handler.profile_get = legion_wmi_gz_profile_get;
	priv->platform_profile_handler.profile_set = legion_wmi_gz_profile_set;

	set_bit(PLATFORM_PROFILE_QUIET, priv->platform_profile_handler.choices);
	set_bit(PLATFORM_PROFILE_BALANCED,priv->platform_profile_handler.choices);
	set_bit(PLATFORM_PROFILE_BALANCED_PERFORMANCE,priv->platform_profile_handler.choices);
	set_bit(PLATFORM_PROFILE_PERFORMANCE,priv->platform_profile_handler.choices);

	ret = platform_profile_register(&priv->platform_profile_handler);
	if (ret)
		return ret;
#endif

	priv->event_nb.notifier_call = legion_wmi_gz_event_call;
	ret = devm_legion_wmi_events_register_notifier(&wdev->dev, &priv->event_nb);
	if (ret)
		goto err_unregister_platform_profile;

	priv->other_nb.notifier_call = legion_wmi_other_gz_call;
	ret = devm_lenovo_wmi_other_register_notifier(&wdev->dev, &priv->other_nb);
	if (ret)
		goto err_unregister_platform_profile;

	priv->fm_nb.notifier_call = legion_wmi_fm_gz_call;
	ret = devm_lenovo_wmi_fm_register_notifier(&wdev->dev, &priv->fm_nb);
	if (ret)
		goto err_unregister_platform_profile;

	ret = legion_wmi_gamezone_sysfs_init(priv);
	if (ret)
		goto err_unregister_platform_profile;

	return 0;

err_unregister_platform_profile:
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 14, 0)
	platform_profile_remove();
#endif
	return ret;
}

static const struct wmi_device_id legion_wmi_gz_id_table[] = {
	{ LEGION_GAMEZONE_GUID, NULL },
	{}
};


static void legion_wmi_gz_remove(struct wmi_device *wdev)
{
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(&wdev->dev);
	
	/* Remove sysfs first before devres cleanup */
	legion_wmi_gamezone_sysfs_exit(priv);
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 14, 0)
	/* Unregister platform profile for older kernels (not devres-managed) */
	platform_profile_remove();
#endif
	/* For kernels >= 6.14.0, platform profile device is automatically cleaned up by devres */
}

static struct wmi_driver lenovo_wmi_gz_driver = {
	.driver = {
		.name = "legion_wmi_gamezone",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_gz_id_table,
	.probe  = legion_wmi_gz_probe,
	.remove = legion_wmi_gz_remove,
	.no_singleton = true,
};

int  legion_wmi_gamezone_driver_init(struct device *parent){
	int ret;

    ret = wmi_driver_register(&lenovo_wmi_gz_driver);

	return ret;
}

void legion_wmi_gamezone_driver_exit(void) {
	wmi_driver_unregister(&lenovo_wmi_gz_driver);
}

MODULE_DEVICE_TABLE(wmi,legion_wmi_gz_id_table);
