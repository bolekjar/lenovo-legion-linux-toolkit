// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-wmi-fm.h"
#include "legion-wmi-ftable.h"
#include "legion-wmi-fm-sysfs.h"
#include "legion-hwmon.h"

#include <linux/wmi.h>
#include <linux/component.h>

#define WMI_GUID_LENOVO_FAN_METHOD "92549549-4bde-4f06-ac04-ce8bf898dbaa"

#define WMI_METHOD_ID_FAN_GET_TABLE 5
#define WMI_METHOD_ID_FAN_SET_TABLE 6

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


static int legion_wmi_fm_call(struct notifier_block *nb, unsigned long cmd,void *data)
{
	struct legion_wmi_fm_priv *priv = container_of(nb, struct legion_wmi_fm_priv, hwmon_nb);
	switch (cmd) {
	case LEGION_WMI_FM_GET_CPU_FAN_MAX_SPEED: {
		int  *fan_max_speed = data;
		*fan_max_speed = priv->fan_cpu_max_speed;
	}
		return NOTIFY_OK;
	case LEGION_WMI_FM_GET_GPU_FAN_MAX_SPEED: {
		int  *fan_max_speed = data;
		*fan_max_speed = priv->fan_gpu_max_speed;
	}
		return NOTIFY_OK;
	case LEGION_WMI_FM_GET_SYS_FAN_MAX_SPEED: {
		int  *fan_max_speed = data;
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
	int ret;

	ret = lenovo_wmi_fm_register_notifier(nb);
	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(dev, devm_lenovo_wmi_fm_unregister_notifier,
			nb);
}

int legion_wmi_fm_notifier_call(void *data,enum gamezone_events_type gamezone_events_type)
{
	int ret;

	ret = blocking_notifier_call_chain(&legion_fm_chain_head,gamezone_events_type, &data);
	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

static int legion_wmi_fm_master_bind(struct device *dev)
{
	struct legion_wmi_fm_priv *priv = dev_get_drvdata(dev);
	int ret;

	ret = component_bind_all(dev, &priv);
	if (ret)
		return ret;

	ret = legion_wmi_fm_sysfs_init(priv);
	if (ret) {
		goto sysfs_err;
	}

	priv->hwmon_nb.notifier_call = legion_wmi_fm_call;
	ret = legion_hwmon_fm_register_notifier(dev, &priv->hwmon_nb);
	if (ret)
		goto notifier_err;

	/* Signal that binding is complete */
	complete(&priv->bind_complete);

	return 0;
notifier_err:
	legion_wmi_fm_sysfs_exit(priv);
sysfs_err:
	component_unbind_all(dev, NULL);
	return ret;
}


static void legion_wmi_fm_master_unbind(struct device *dev)
{
	struct legion_wmi_fm_priv *priv = dev_get_drvdata(dev);

	legion_wmi_fm_sysfs_exit(priv);

	component_unbind_all(dev, NULL);
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
	struct component_match 		 *master_match 	= NULL;
	struct legion_wmi_fm_priv 	 *priv 			= NULL;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;

	/* Initialize completion */
	init_completion(&priv->bind_complete);

	dev_set_drvdata(&wdev->dev, priv);

	component_match_add(&wdev->dev, &master_match, legion_wmi_ftable_match, NULL);
	if (IS_ERR(master_match))
		return PTR_ERR(master_match);


	/* Register as a component master (for cd01/dd components) */
	int ret = component_master_add_with_match(&wdev->dev, &legion_wmi_fm_master_ops,
		       master_match);
	if (ret)
		return ret;

	/* Wait for binding to complete (with timeout) */
	if (!wait_for_completion_timeout(&priv->bind_complete, msecs_to_jiffies(5000))) {
		component_master_del(&wdev->dev, &legion_wmi_fm_master_ops);
		return -ETIMEDOUT;
	}

	return 0;
}

static void legio_fm_other_remove(struct wmi_device *wdev)
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
	.remove = legio_fm_other_remove,
	.no_singleton = true,
};


int  legion_wmi_fm_driver_init(struct device *parent) {
	return wmi_driver_register(&legion_fm_other_driver);
}


void legion_wmi_fm_driver_exit(void) {
	wmi_driver_unregister(&legion_fm_other_driver);
}

MODULE_DEVICE_TABLE(acpi, legion_fm_other_id_table);
