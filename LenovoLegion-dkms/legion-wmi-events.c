// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-wmi-events.h"
#include "legion-wmi-gamezone.h"

#include <linux/acpi.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/types.h>
#include <linux/wmi.h>

#define THERMAL_MODE_EVENT_GUID "D320289E-8FEA-41E0-86F9-911D83151B5F"
#define CHARGE_MODE_EVENT_GUID  "D320289E-8FEA-41E0-86F9-711D83151B5F"
#define UNKNOWN_EVENT_GUID      "D320289E-8FEA-41E0-86F9-811D83151B5F"
#define UNKNOWN_EVENT2_GUID     "D320289E-8FEA-41E1-86F9-611D83151B5F"
#define SMART_FAN_MODE_GUID		"D320289E-8FEA-41E0-86F9-611D83151B5F"
#define KEYLOCK_STATUS_GUID     "10AFC6D9-EA8B-4590-A2E7-1CD3C84BB4B1"
#define UTILITY_GUID    		"8fc0de0c-b4e4-43fd-b0f3-8871711c1294"

#define LEGION_WMI_EVENT_DEVICE(guid, type)                        \
	.guid_string = (guid), .context = &(enum legion_wmi_events_type) \
	{                                                          \
		type                                               \
	}

static BLOCKING_NOTIFIER_HEAD(events_chain_head);

struct legion_wmi_events_priv {
	struct wmi_device *wdev;
	enum legion_wmi_events_type type;
};

/**
 * lwmi_events_register_notifier() - Add a notifier to the notifier chain.
 * @nb: The notifier_block struct to register
 *
 * Call blocking_notifier_chain_register to register the notifier block to the
 * lenovo-wmi-events driver blocking notifier chain.
 *
 * Return: 0 on success, %-EEXIST on error.
 */
int legion_wmi_events_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&events_chain_head, nb);
}

/**
 * lwmi_events_unregister_notifier() - Remove a notifier from the notifier
 * chain.
 * @nb: The notifier_block struct to unregister
 *
 * Call blocking_notifier_chain_unregister to unregister the notifier block
 * from the lenovo-wmi-events driver blocking notifier chain.
 *
 * Return: 0 on success, %-ENOENT on error.
 */
int legion_wmi_events_unregister_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&events_chain_head, nb);
}

/**
 * devm_lwmi_events_unregister_notifier() - Remove a notifier from the notifier
 * chain.
 * @data: Void pointer to the notifier_block struct to unregister.
 *
 * Call lwmi_events_unregister_notifier to unregister the notifier block from
 * the lenovo-wmi-events driver blocking notifier chain.
 *
 * Return: 0 on success, %-ENOENT on error.
 */
static void devm_legion_wmi_events_unregister_notifier(void *data)
{
	struct notifier_block *nb = data;

	legion_wmi_events_unregister_notifier(nb);
}

/**
 * devm_lwmi_events_register_notifier() - Add a notifier to the notifier chain.
 * @dev: The parent device of the notifier_block struct.
 * @nb: The notifier_block struct to register
 *
 * Call lwmi_events_register_notifier to register the notifier block to the
 * lenovo-wmi-events driver blocking notifier chain. Then add, as a device
 * managed action, unregister_notifier to automatically unregister the
 * notifier block upon its parent device removal.
 *
 * Return: 0 on success, or an error code.
 */
int devm_legion_wmi_events_register_notifier(struct device *dev,
				       struct notifier_block *nb)
{
	int ret;

	ret = legion_wmi_events_register_notifier(nb);
	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(dev, devm_legion_wmi_events_unregister_notifier, nb);
}

/**
 * lwmi_events_notify() - Call functions for the notifier call chain.
 * @wdev: The parent WMI device of the driver.
 * @obj: ACPI object passed by the registered WMI Event.
 *
 * Validate WMI event data and notify all registered drivers of the event and
 * its output.
 *
 * Return: 0 on success, or an error code.
 */
static void legion_wmi_events_notify(struct wmi_device *wdev, union acpi_object *obj)
{
	struct legion_wmi_events_priv *priv = dev_get_drvdata(&wdev->dev);
	int sel_prof;
	int ret;
	char event_type[64]  = {0};
	char event_value[64] = {0};
	char *envp[] = { event_type,event_value ,NULL };

	if(priv->type != LEGION_WMI_EVENT_UNKNOWN)
	{
		sprintf(event_type,"EVENT_TYPE=%d" ,priv->type);
		if(obj->type == ACPI_TYPE_INTEGER)
		{
			sprintf(event_value,"EVENT_VALUE=%llu",obj->integer.value);
		}
		else
		{
			sprintf(event_value,"EVENT_VALUE=N/A");
		}

		kobject_uevent_env(&wdev->dev.kobj, KOBJ_CHANGE,envp);
	}

	switch (priv->type) {
	case LEGION_WMI_EVENT_THERMAL_MODE:
	{
		if (obj->type != ACPI_TYPE_INTEGER)
			return;

		sel_prof = obj->integer.value;

		switch (sel_prof) {
		case LEGION_WMI_GZ_THERMAL_MODE_QUIET:
		case LEGION_WMI_GZ_THERMAL_MODE_BALANCED:
		case LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE:
		case LEGION_WMI_GZ_THERMAL_MODE_EXTREME:
		case LEGION_WMI_GZ_THERMAL_MODE_CUSTOM:
			ret = blocking_notifier_call_chain(&events_chain_head,
					LEGION_WMI_EVENT_THERMAL_MODE,
							   &sel_prof);
			if (ret == NOTIFY_BAD)
				dev_err(&wdev->dev,
					"Failed to send notification to call chain for WMI Events\n");
			return;
		default:
			dev_err(&wdev->dev, "Got invalid thermal mode: %x",
				sel_prof);
			return;
		}
	}
		break;
	case LEGION_WMI_EVENT_POWER_CHARGE_MODE:
	{
		if (obj->type != ACPI_TYPE_INTEGER)
			return;

		sel_prof = obj->integer.value;

		switch (sel_prof) {
		case LEGION_WMI_GZ_AC_CONNECTED:
		case LEGION_WMI_GZ_AC_CONNECTED_LOW_WATTAGE:
		case LEGION_WMI_GZ_AC_DISCONNECTED:
			ret = blocking_notifier_call_chain(&events_chain_head,
					LEGION_WMI_EVENT_POWER_CHARGE_MODE,
							   &sel_prof);
			if (ret == NOTIFY_BAD)
				dev_err(&wdev->dev,
					"Failed to send notification to call chain for WMI Events\n");
			return;
		default:
			dev_err(&wdev->dev, "Got invalid power charge mode: %x",
				sel_prof);
			return;
		};
	}
		break;
	default:
		return;
	}
}

static int legion_wmi_events_probe(struct wmi_device *wdev, const void *context)
{
	struct legion_wmi_events_priv *priv;

	if (!context)
		return -EINVAL;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;
	priv->type = *(enum legion_wmi_events_type *)context;
	dev_set_drvdata(&wdev->dev, priv);

	return 0;
}

static const struct wmi_device_id legion_wmi_events_id_table[] = {
	{ LEGION_WMI_EVENT_DEVICE(THERMAL_MODE_EVENT_GUID, LEGION_WMI_EVENT_THERMAL_MODE) },
	{ LEGION_WMI_EVENT_DEVICE(CHARGE_MODE_EVENT_GUID, LEGION_WMI_EVENT_POWER_CHARGE_MODE) },
	{ LEGION_WMI_EVENT_DEVICE(UNKNOWN_EVENT_GUID, LEGION_WMI_EVENT_UNKNOWN) },
	{ LEGION_WMI_EVENT_DEVICE(UNKNOWN_EVENT2_GUID, LEGION_WMI_EVENT_UNKNOWN) },
	{ LEGION_WMI_EVENT_DEVICE(SMART_FAN_MODE_GUID,LEGION_WMI_EVENT_SMART_FAN_MODE) },
	{ LEGION_WMI_EVENT_DEVICE(KEYLOCK_STATUS_GUID,LEGION_WMI_EVENT_KEYLOCK_STATUS) },
	{ LEGION_WMI_EVENT_DEVICE(UTILITY_GUID,LENOVO_WMI_EVENT_UTILITY) },
	{}
};

static struct wmi_driver legion_wmi_events_driver = {
	.driver = {
		.name = "legion_wmi_events",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_events_id_table,
	.probe = legion_wmi_events_probe,
	.notify = legion_wmi_events_notify,
	.no_singleton = true,
};

int  legion_wmi_events_driver_init(struct device *parent){
	return wmi_driver_register(&legion_wmi_events_driver);
}

void legion_wmi_events_driver_exit(void){
	wmi_driver_unregister(&legion_wmi_events_driver);
}

MODULE_DEVICE_TABLE(wmi,legion_wmi_events_id_table);
