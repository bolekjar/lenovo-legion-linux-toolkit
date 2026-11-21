// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the lenovo-wmi-events driver of the Linux kernel (drivers/platform/x86/lenovo/wmi-events.c),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */
#include "legion-common.h"
#include "legion-wmi-events.h"
#include "legion-wmi-gamezone.h"

#include <linux/acpi.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/types.h>
#include <linux/wmi.h>

#define THERMAL_MODE_EVENT_GUID "D320289E-8FEA-41E0-86F9-911D83151B5F"
#define CHARGE_MODE_EVENT_GUID "D320289E-8FEA-41E0-86F9-711D83151B5F"
#define UNKNOWN_EVENT_GUID "D320289E-8FEA-41E0-86F9-811D83151B5F"
#define UNKNOWN_EVENT2_GUID "D320289E-8FEA-41E1-86F9-611D83151B5F"
#define SMART_FAN_MODE_GUID "D320289E-8FEA-41E0-86F9-611D83151B5F"
#define KEYLOCK_STATUS_GUID "10AFC6D9-EA8B-4590-A2E7-1CD3C84BB4B1"
#define UTILITY_GUID "8fc0de0c-b4e4-43fd-b0f3-8871711c1294"

#define LEGION_WMI_EVENT_DEVICE(guid, type)                          \
	.guid_string = (guid), .context = &(enum legion_events_type) \
	{                                                            \
		type                                                 \
	}

static BLOCKING_NOTIFIER_HEAD(events_chain_head);

struct legion_wmi_events_priv {
	struct wmi_device *wdev;
	enum legion_events_type type;
	/*
	 * Serializes the events of the device: the kernel runs the ACPI notifications in parallel (kacpi_notify
	 * workqueue per CPU, the WMI core takes notify_lock for reading), the game zone reads the firmware for the
	 * thermal mode event; a later event reads the firmware after the store and the uevent of the earlier one
	 */
	struct mutex lock;
};

/**
 * legion_wmi_events_register_notifier() - Add a notifier to the notifier chain.
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
 * legion_wmi_events_unregister_notifier() - Remove a notifier from the notifier
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
 * devm_legion_wmi_events_unregister_notifier() - Remove a notifier from the notifier
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
 * devm_legion_wmi_events_register_notifier() - Add a notifier to the notifier chain.
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
	const int ret = legion_wmi_events_register_notifier(nb);

	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(
		dev, devm_legion_wmi_events_unregister_notifier, nb);
}

/*
 * Thermal mode event to the game zone first: the effective mode (the game zone corrects the custom mode which the
 * older firmware reports as performance), the value as it is for an invalid mode
 */
static u64 legion_wmi_events_thermal_mode(struct wmi_device *wdev, u64 value)
{
	enum thermal_mode mode;

	switch (value) {
	case LEGION_WMI_GZ_THERMAL_MODE_QUIET:
	case LEGION_WMI_GZ_THERMAL_MODE_BALANCED:
	case LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE:
	case LEGION_WMI_GZ_THERMAL_MODE_EXTREME:
	case LEGION_WMI_GZ_THERMAL_MODE_CUSTOM:
		break;
	default:
		dev_err(&wdev->dev, "Got invalid thermal mode: %llx\n", value);
		return value;
	}

	/* Converted by value: the chain gets the type which the game zone reads and writes */
	mode = value;
	if (blocking_notifier_call_chain(&events_chain_head,
					 LEGION_WMI_EVENT_THERMAL_MODE,
					 &mode) == NOTIFY_BAD)
		dev_err(&wdev->dev,
			"Failed to send notification to call chain for WMI Events\n");

	return mode;
}

/*
 * Power charge mode event (AC adapter) to the game zone
 */
static void legion_wmi_events_power_charge_mode(struct wmi_device *wdev,
						u64 value)
{
	enum power_adapter_status status;
	int ret;

	switch (value) {
	case LEGION_WMI_GZ_AC_CONNECTED:
	case LEGION_WMI_GZ_AC_CONNECTED_LOW_WATTAGE:
	case LEGION_WMI_GZ_AC_DISCONNECTED:
		break;
	default:
		dev_err(&wdev->dev, "Got invalid power charge mode: %llx\n",
			value);
		return;
	}

	status = value;
	ret = blocking_notifier_call_chain(&events_chain_head,
					   LEGION_WMI_EVENT_POWER_CHARGE_MODE,
					   &status);
	if (ret == NOTIFY_BAD)
		dev_err(&wdev->dev,
			"Failed to send notification to call chain for WMI Events\n");
}

/**
 * legion_wmi_events_notify() - Call functions for the notifier call chain.
 * @wdev: The parent WMI device of the driver.
 * @obj: ACPI object passed by the registered WMI Event.
 *
 * Validate WMI event data, notify all registered drivers of the event and send a uevent (EVENT_TYPE,
 * EVENT_VALUE). EVENT_VALUE of the thermal mode is the effective thermal mode after the game zone (255 for the
 * custom mode also on the older firmware), the other events carry the value of the firmware.
 */
static void legion_wmi_events_notify(struct wmi_device *wdev,
				     union acpi_object *obj)
{
	struct legion_wmi_events_priv *priv = dev_get_drvdata(&wdev->dev);
	const bool integer = obj->type == ACPI_TYPE_INTEGER;
	u64 value = integer ? obj->integer.value : 0;
	char event_type[64] = { 0 };
	char event_value[64] = { 0 };
	char *envp[] = { event_type, event_value, NULL };

	/* The chain (game zone) and the uevent of one event before the next one */
	guard(mutex)(&priv->lock);

	if (integer) {
		switch (priv->type) {
		case LEGION_WMI_EVENT_THERMAL_MODE:
			value = legion_wmi_events_thermal_mode(wdev, value);
			break;
		case LEGION_WMI_EVENT_POWER_CHARGE_MODE:
			legion_wmi_events_power_charge_mode(wdev, value);
			break;
		default:
			break;
		}
	}

	if (priv->type == LEGION_WMI_EVENT_UNKNOWN)
		return;

	snprintf(event_type, sizeof(event_type), "EVENT_TYPE=%d", priv->type);
	if (integer)
		snprintf(event_value, sizeof(event_value), "EVENT_VALUE=%llu",
			 value);
	else
		snprintf(event_value, sizeof(event_value), "EVENT_VALUE=N/A");

	kobject_uevent_env(&wdev->dev.kobj, KOBJ_CHANGE, envp);
}

static int legion_wmi_events_probe(struct wmi_device *wdev, const void *context)
{
	if (!context)
		return -EINVAL;

	struct legion_wmi_events_priv *priv =
		devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;
	priv->type = *(enum legion_events_type *)context;

	const int ret = devm_mutex_init(&wdev->dev, &priv->lock);

	if (ret)
		return ret;

	dev_set_drvdata(&wdev->dev, priv);

	return 0;
}

static const struct wmi_device_id legion_wmi_events_id_table[] = {
	{ LEGION_WMI_EVENT_DEVICE(THERMAL_MODE_EVENT_GUID,
				  LEGION_WMI_EVENT_THERMAL_MODE) },
	{ LEGION_WMI_EVENT_DEVICE(CHARGE_MODE_EVENT_GUID,
				  LEGION_WMI_EVENT_POWER_CHARGE_MODE) },
	{ LEGION_WMI_EVENT_DEVICE(UNKNOWN_EVENT_GUID,
				  LEGION_WMI_EVENT_UNKNOWN) },
	{ LEGION_WMI_EVENT_DEVICE(UNKNOWN_EVENT2_GUID,
				  LEGION_WMI_EVENT_UNKNOWN) },
	{ LEGION_WMI_EVENT_DEVICE(SMART_FAN_MODE_GUID,
				  LEGION_WMI_EVENT_SMART_FAN_MODE) },
	{ LEGION_WMI_EVENT_DEVICE(KEYLOCK_STATUS_GUID,
				  LEGION_WMI_EVENT_KEYLOCK_STATUS) },
	{ LEGION_WMI_EVENT_DEVICE(UTILITY_GUID, LENOVO_WMI_EVENT_UTILITY) },
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

int legion_wmi_events_driver_init(void)
{
	return wmi_driver_register(&legion_wmi_events_driver);
}

void legion_wmi_events_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_events_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_events_id_table);
