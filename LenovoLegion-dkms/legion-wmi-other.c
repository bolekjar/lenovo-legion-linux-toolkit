// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-wmi-other.h"
#include "legion-wmi-capdata01.h"
#include "legion-wmi-capdata00.h"
#include "legion-wmi-ddata.h"
#include "legion-wmi-other-sysfs.h"
#include "legion-wmi-gamezone.h"
#include "legion-hwmon.h"
#include "legion-wmi-helpers.h"
#include "legion-dkms.h"
#include "legion-rapl-mmio.h"
#include "legion-rapl.h"

#include <linux/module.h>
#include <linux/device.h>
#include <linux/wmi.h>
#include <linux/component.h>
#include <linux/platform_device.h>
#include <linux/completion.h>

#define LEGION_WMI_LENOVO_OTHER_METHOD_GUID "DC2A8805-3A8C-41BA-A6F7-092E0089CD3B"

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
 * lwmi_om_unregister_notifier() - Remove a notifier from the blocking notifier
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
 * dev_lenovo_wmi_other_unregister_notifier() - Remove a notifier from the blocking
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
	int ret;

	ret = lenovo_wmi_other_register_notifier(nb);
	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(dev, devm_lenovo_wmi_other_unregister_notifier,
			nb);
}


int legion_wmi_other_notifier_call(void *data,enum gamezone_events_type gamezone_events_type)
{
	int ret;

	ret = blocking_notifier_call_chain(&legion_other_chain_head,gamezone_events_type, &data);
	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}


static int legion_wmi_other_call(struct notifier_block *nb,unsigned long action, void *data)
{
	struct legion_wmi_other_priv *priv = container_of(nb, struct legion_wmi_other_priv, hwmon_nb);
	struct hwmon_capability*     value = data;


	switch(action)
	{
	case LEGION_WMI_FM_GET_CAPABILITY_VALUE:
	{
		struct wmi_method_args_32 args;

		args.arg0 = value->capability_id  & (~LEGION_WMI_MODE_ID_MASK);
		args.arg1 = 0;

		if(legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_GET,
					    (unsigned char *)&args, sizeof(args),
					    value->data))
		{
			return NOTIFY_BAD;
		}

		return NOTIFY_OK;
	}
	break;
	case LEGION_WMI_FM_GET_CAPABILITY_DATA:
	{
		int ret = legion_wmi_cd00_get_data(priv->cd00_list,value->capability_id & (~LEGION_WMI_MODE_ID_MASK), value->data);
		if (ret && ret != -EINVAL) {
			return NOTIFY_BAD;
		}
		return NOTIFY_OK;
	}
	break;
	default:
		return NOTIFY_DONE;
	break;
	}

	return NOTIFY_DONE;
}


static int legion_wmi_other_dkms_call(struct notifier_block *nb,unsigned long action, void *data)
{
	struct legion_wmi_other_priv *priv 	  = container_of(nb, struct legion_wmi_other_priv, dkms_nb);
	struct other_events_data * event_data = data;

	unsigned int pl1_w 	 = 0,
				 pl1_time_s = 0,
				 pl2_w 	 = 0;

	if(action == LEGION_WMI_OTHER_SET_THERMAL_MODE_RAPL_AND_MMIO)
	{
		struct legion_wmi_capdata01 capdata;
		struct legion_wmi_ddata     lddata[48] = {{0,0}};

		if (legion_wmi_cd01_get_data(priv->cd01_list,(CPULongTermPowerLimit  & (~LEGION_WMI_MODE_ID_MASK)) | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, event_data->mode), &capdata)) {
			return NOTIFY_BAD;
		}
		pl1_w = capdata.max_value;


		if (legion_wmi_cd01_get_data(priv->cd01_list,(CPUShortTermPowerLimit & (~LEGION_WMI_MODE_ID_MASK)) | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, event_data->mode), &capdata)) {
			return NOTIFY_BAD;
		}
		pl2_w = capdata.max_value;


		if (legion_wmi_dd_get_data(priv->dd_list,(CPUPL1Tau & (~LEGION_WMI_MODE_ID_MASK)) | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, event_data->mode), lddata,sizeof(lddata)/sizeof(lddata[0]) - 1)) {

			if (legion_wmi_cd01_get_data(priv->cd01_list,(CPUPL1Tau & (~LEGION_WMI_MODE_ID_MASK)) | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, event_data->mode), &capdata)) {
				return NOTIFY_BAD;
			}

			pl1_time_s = capdata.default_value;
		}
		else
		{
			pl1_time_s = 0;
			for(int i = 0;i < (sizeof(lddata)/sizeof(lddata[0])) && lddata[i].id != 0;++i)
			{
				pl1_time_s = max(pl1_time_s,lddata[i].value);
			}
		}

		if(legion_set_power_and_time_sysfs(event_data->rapl_private, pl1_w * 1000000, pl1_time_s * 1000000, pl2_w * 1000000))
		{
			return NOTIFY_BAD;
		}
	}


	if(action == LEGION_WMI_OTHER_SET_THERMAL_MODE_RAPL_AND_MMIO ||
	   action == LEGION_WMI_OTHER_SET_THERMAL_MODE_RAPL_MMIO_ONLY
	   )
	{
		if(event_data->mode == LEGION_WMI_GZ_THERMAL_MODE_CUSTOM)
		{
			struct wmi_method_args_32 args;

			args.arg0 = CPULongTermPowerLimit  & (~LEGION_WMI_MODE_ID_MASK);
			if(legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_GET,(unsigned char *)&args, sizeof(args),&pl1_w))
			{
				return NOTIFY_BAD;
			}

			args.arg0 = CPUShortTermPowerLimit  & (~LEGION_WMI_MODE_ID_MASK);
			if(legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_GET,(unsigned char *)&args, sizeof(args),&pl2_w))
			{
				return NOTIFY_BAD;
			}

			args.arg0 = CPUPL1Tau  & (~LEGION_WMI_MODE_ID_MASK);
			if(legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_GET,(unsigned char *)&args, sizeof(args),&pl1_time_s))
			{
				return NOTIFY_BAD;
			}
		}
		else
		{
			struct legion_wmi_capdata01 capdata;

			if (legion_wmi_cd01_get_data(priv->cd01_list,(CPULongTermPowerLimit  & (~LEGION_WMI_MODE_ID_MASK)) | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, event_data->mode), &capdata)) {
				return NOTIFY_BAD;
			}
			pl1_w = capdata.default_value;


			if (legion_wmi_cd01_get_data(priv->cd01_list,(CPUShortTermPowerLimit & (~LEGION_WMI_MODE_ID_MASK)) | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, event_data->mode), &capdata)) {
				return NOTIFY_BAD;
			}
			pl2_w = capdata.default_value;

			if (legion_wmi_cd01_get_data(priv->cd01_list,(CPUPL1Tau & (~LEGION_WMI_MODE_ID_MASK)) | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, event_data->mode), &capdata)) {
				return NOTIFY_BAD;
			}
			pl1_time_s = capdata.default_value;
		}

		if(legion_set_power_and_time(event_data->rapl_mmio_private,pl1_w * 1000,pl1_time_s * 1000000,pl2_w * 1000))
		{
			return NOTIFY_BAD;
		}

	}

	return NOTIFY_OK;
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
	int ret;

	ret = component_bind_all(dev, &priv);
	if (ret)
		return ret;

	ret = legion_wmi_other_sysfs_init(priv);
	if (ret) {
		goto sysfs_err;
	}

	priv->hwmon_nb.notifier_call = legion_wmi_other_call;
	ret = legion_hwmon_other_register_notifier(dev, &priv->hwmon_nb);
	if (ret)
		goto hwmon_err;

	priv->dkms_nb.notifier_call = legion_wmi_other_dkms_call;
	ret = devm_lenovo_dkms_register_notifier(dev, &priv->dkms_nb);
	if (ret)
		goto dkms_err;

	/* Signal that binding is complete */
	complete(&priv->bind_complete);

	return 0;

dkms_err:
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

	legion_wmi_other_sysfs_exit(priv);

	component_unbind_all(dev, NULL);
}

static const struct component_master_ops legion_wmi_om_master_ops = {
	.bind = legion_wmi_om_master_bind,
	.unbind = legion_wmi_om_master_unbind,
};

static int legion_wmi_other_probe(struct wmi_device *wdev, const void *context)
{
	struct component_match 		 *master_match = NULL;
	struct legion_wmi_other_priv *priv = NULL;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->cd01_list = NULL;
	priv->dd_list = NULL;
	priv->wdev = wdev;

	/* Initialize completion */
	init_completion(&priv->bind_complete);

	dev_set_drvdata(&wdev->dev, priv);

	component_match_add(&wdev->dev, &master_match, legion_wmi_cd00_match, NULL);
	if (IS_ERR(master_match))
		return PTR_ERR(master_match);

	component_match_add(&wdev->dev, &master_match, legion_wmi_cd01_match, NULL);
	if (IS_ERR(master_match))
		return PTR_ERR(master_match);

	component_match_add(&wdev->dev, &master_match, legion_wmi_dd_match, NULL);
	if (IS_ERR(master_match))
		return PTR_ERR(master_match);

	/* Register as a component master (for cd01/dd components) */
	int ret = component_master_add_with_match(&wdev->dev, &legion_wmi_om_master_ops,
					       master_match);
	if (ret)
		return ret;

	/* Wait for binding to complete (with timeout) */
	if (!wait_for_completion_timeout(&priv->bind_complete, msecs_to_jiffies(5000))) {
		component_master_del(&wdev->dev, &legion_wmi_om_master_ops);
		return -ETIMEDOUT;
	}


	return 0;
}

static void legio_nwmi_other_remove(struct wmi_device *wdev)
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
	.remove = legio_nwmi_other_remove,
	.no_singleton = true,
};

int legion_wmi_other_driver_init(struct device *dev) {
	return wmi_driver_register(&legion_wmi_other_driver);
}

void legion_wmi_other_driver_exit(void) {
	wmi_driver_unregister(&legion_wmi_other_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_other_id_table);
