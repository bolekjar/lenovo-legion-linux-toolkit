// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Features of the ACPI device VPC2004 (Lenovo "VPC" interface) which are lost with the blacklisted ideapad_laptop:
 * battery conservation mode (charge limit), rapid charge, USB charging, Fn lock and camera power. Only the ACPI device is bound,
 * the WMI GUID 8FC0DE0C of ideapad_laptop stays with the events driver of this module.
 *
 * The ACPI methods, their bits and the EC handshake are the same as in ideapad_laptop of the Linux kernel
 * (drivers/platform/x86/lenovo/ideapad-laptop.c).
 *
 * Firmware attributes: /sys/class/legion-firmware-attributes/legion-vpc-0/attributes/<feature>/
 * {current_value, supported, type, display_name}, a write sends a uevent of the class device. Since kernel 6.16 the
 * conservation mode and rapid charge are also charge_types of the battery (Standard, Fast, Long_Life) for the battery
 * settings of the desktops. A write is verified by
 * reading the state back: a feature whose write the firmware ignores (camera power of Legion Pro 7 16IAX10H, reported
 * by _CFG) is marked as not supported.
 */
#include "legion-acpi-vpc.h"
#include "legion-common.h"
#include "legion-firmware-attributes-class.h"
#include "legion-wmi-helpers.h"

#include <linux/acpi.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/idr.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/version.h>

#include <acpi/battery.h>

/*
 * charge_types of a power supply extension (struct power_supply_ext.charge_types) since 6.16
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
#define LEGION_ACPI_VPC_BATTERY_EXT
#endif

/*
 * Name of the ACPI battery hook functions, detected by the Makefile (renamed in some kernels)
 */
#ifdef LEGION_ACPI_BATTERY_HOOK_PREFIX
#define legion_battery_hook_register acpi_battery_hook_register
#define legion_battery_hook_unregister acpi_battery_hook_unregister
#else
#define legion_battery_hook_register battery_hook_register
#define legion_battery_hook_unregister battery_hook_unregister
#endif

#define LEGION_ACPI_VPC_FW_ATTR_BASE_PATH "legion-vpc"

/* Capabilities of _CFG */
#define VPC_CFG_CAP_CAM_BIT 19

/* GBMD state / SBMC commands of the battery charge modes */
#define VPC_GBMD_RAPID_CHARGE_STATE_BIT 2
#define VPC_GBMD_CONSERVATION_STATE_BIT 5
#define VPC_GBMD_RAPID_CHARGE_SUPPORTED_BIT 17

#define VPC_SBMC_CONSERVATION_ON 3
#define VPC_SBMC_CONSERVATION_OFF 5
#define VPC_SBMC_RAPID_CHARGE_ON 7
#define VPC_SBMC_RAPID_CHARGE_OFF 8

/* HALS state / SALS commands */
#define VPC_HALS_USB_CHARGING_SUPPORT_BIT 6
#define VPC_HALS_USB_CHARGING_STATE_BIT 7
#define VPC_HALS_FNLOCK_SUPPORT_BIT 9
#define VPC_HALS_FNLOCK_STATE_BIT 10

#define VPC_SALS_USB_CHARGING_ON 0xa
#define VPC_SALS_USB_CHARGING_OFF 0xb
#define VPC_SALS_FNLOCK_ON 0xe
#define VPC_SALS_FNLOCK_OFF 0xf

/* EC commands (VPCR/VPCW) */
#define VPC_CMD_R_CAMERA 0x1d
#define VPC_CMD_W_CAMERA 0x1e

#define VPC_EC_TIMEOUT_MS 200
#define VPC_EC_POLL_MIN_US 150
#define VPC_EC_POLL_MAX_US 300

enum legion_acpi_vpc_feature {
	VPC_CONSERVATION_MODE,
	VPC_USB_CHARGING,
	VPC_FN_LOCK,
	VPC_CAMERA_POWER,
	VPC_RAPID_CHARGE,
	VPC_FEATURES_COUNT
};

struct legion_acpi_vpc_priv {
	struct acpi_device *adev;
	struct device *fw_attr_dev;
	struct kset *fw_attr_kset;
	struct mutex mutex; /* serializes the ACPI methods and the EC handshake */
	bool supported[VPC_FEATURES_COUNT]; /* cleared when the firmware ignores a write */
	int ida_id;
#ifdef LEGION_ACPI_VPC_BATTERY_EXT
	struct device *dev;
	struct acpi_battery_hook battery_hook;
	const struct power_supply_ext *battery_ext;
	bool battery_hook_registered;
#endif
};

static DEFINE_IDA(legion_acpi_vpc_ida);

static const char *const legion_acpi_vpc_names[VPC_FEATURES_COUNT] = {
	[VPC_CONSERVATION_MODE] = "conservation_mode",
	[VPC_USB_CHARGING] = "usb_charging",
	[VPC_FN_LOCK] = "fn_lock",
	[VPC_CAMERA_POWER] = "camera_power",
	[VPC_RAPID_CHARGE] = "rapid_charge",
};

static int vpc_eval_int(acpi_handle handle, char *name, unsigned long *res)
{
	unsigned long long result;

	if (ACPI_FAILURE(acpi_evaluate_integer(handle, name, NULL, &result)))
		return -EIO;

	*res = result;
	return 0;
}

static int vpc_exec(acpi_handle handle, char *name, unsigned long arg)
{
	return ACPI_FAILURE(acpi_execute_simple_method(handle, name, arg)) ?
		       -EIO :
		       0;
}

static int vpc_eval_vpcr(acpi_handle handle, unsigned long cmd,
			 unsigned long *res)
{
	struct acpi_object_list params;
	union acpi_object in_obj;
	unsigned long long result;

	params.count = 1;
	params.pointer = &in_obj;
	in_obj.type = ACPI_TYPE_INTEGER;
	in_obj.integer.value = cmd;

	if (ACPI_FAILURE(acpi_evaluate_integer(handle, "VPCR", &params, &result)))
		return -EIO;

	*res = result;
	return 0;
}

static int vpc_eval_vpcw(acpi_handle handle, unsigned long cmd,
			 unsigned long data)
{
	struct acpi_object_list params;
	union acpi_object in_obj[2];

	params.count = 2;
	params.pointer = in_obj;
	in_obj[0].type = ACPI_TYPE_INTEGER;
	in_obj[0].integer.value = cmd;
	in_obj[1].type = ACPI_TYPE_INTEGER;
	in_obj[1].integer.value = data;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, "VPCW", &params, NULL)))
		return -EIO;

	return 0;
}

/*
 * The EC clears the command register (VPCR 1) when it has finished a command
 */
static int vpc_ec_wait(acpi_handle handle)
{
	unsigned long end = jiffies + msecs_to_jiffies(VPC_EC_TIMEOUT_MS) + 1;
	unsigned long val;
	int err;

	while (time_before(jiffies, end)) {
		usleep_range(VPC_EC_POLL_MIN_US, VPC_EC_POLL_MAX_US);

		err = vpc_eval_vpcr(handle, 1, &val);
		if (err)
			return err;

		if (val == 0)
			return 0;
	}

	acpi_handle_err(handle, "timeout of the EC command\n");
	return -ETIMEDOUT;
}

static int vpc_ec_read(acpi_handle handle, unsigned long cmd,
		       unsigned long *data)
{
	int err;

	err = vpc_eval_vpcw(handle, 1, cmd);
	if (err)
		return err;

	err = vpc_ec_wait(handle);
	if (err)
		return err;

	return vpc_eval_vpcr(handle, 0, data);
}

static int vpc_ec_write(acpi_handle handle, unsigned long cmd,
			unsigned long data)
{
	int err;

	err = vpc_eval_vpcw(handle, 0, data);
	if (err)
		return err;

	err = vpc_eval_vpcw(handle, 1, cmd);
	if (err)
		return err;

	return vpc_ec_wait(handle);
}

/*
 * State of a feature from the firmware, caller holds priv->mutex
 */
static int legion_acpi_vpc_read(struct legion_acpi_vpc_priv *priv,
				enum legion_acpi_vpc_feature feature,
				unsigned int *value)
{
	acpi_handle handle = priv->adev->handle;
	unsigned long result;
	int err;

	lockdep_assert_held(&priv->mutex);

	switch (feature) {
	case VPC_CONSERVATION_MODE:
		err = vpc_eval_int(handle, "GBMD", &result);
		*value = test_bit(VPC_GBMD_CONSERVATION_STATE_BIT, &result);
		break;
	case VPC_USB_CHARGING:
		err = vpc_eval_int(handle, "HALS", &result);
		*value = test_bit(VPC_HALS_USB_CHARGING_STATE_BIT, &result);
		break;
	case VPC_FN_LOCK:
		err = vpc_eval_int(handle, "HALS", &result);
		*value = test_bit(VPC_HALS_FNLOCK_STATE_BIT, &result);
		break;
	case VPC_CAMERA_POWER:
		err = vpc_ec_read(handle, VPC_CMD_R_CAMERA, &result);
		*value = !!result;
		break;
	case VPC_RAPID_CHARGE:
		err = vpc_eval_int(handle, "GBMD", &result);
		*value = test_bit(VPC_GBMD_RAPID_CHARGE_STATE_BIT, &result);
		break;
	default:
		return -EINVAL;
	}

	return err;
}

/*
 * Write of a feature to the firmware, caller holds priv->mutex
 */
static int legion_acpi_vpc_write(struct legion_acpi_vpc_priv *priv,
				 enum legion_acpi_vpc_feature feature,
				 unsigned int value)
{
	acpi_handle handle = priv->adev->handle;
	int err;

	lockdep_assert_held(&priv->mutex);

	switch (feature) {
	case VPC_CONSERVATION_MODE:
		if (value && priv->supported[VPC_RAPID_CHARGE]) {
			err = vpc_exec(handle, "SBMC",
				       VPC_SBMC_RAPID_CHARGE_OFF);
			if (err)
				return err;
		}
		return vpc_exec(handle, "SBMC",
				value ? VPC_SBMC_CONSERVATION_ON :
					VPC_SBMC_CONSERVATION_OFF);
	case VPC_USB_CHARGING:
		return vpc_exec(handle, "SALS",
				value ? VPC_SALS_USB_CHARGING_ON :
					VPC_SALS_USB_CHARGING_OFF);
	case VPC_FN_LOCK:
		return vpc_exec(handle, "SALS",
				value ? VPC_SALS_FNLOCK_ON :
					VPC_SALS_FNLOCK_OFF);
	case VPC_CAMERA_POWER:
		return vpc_ec_write(handle, VPC_CMD_W_CAMERA, value);
	case VPC_RAPID_CHARGE:
		if (value && priv->supported[VPC_CONSERVATION_MODE]) {
			err = vpc_exec(handle, "SBMC",
				       VPC_SBMC_CONSERVATION_OFF);
			if (err)
				return err;
		}
		return vpc_exec(handle, "SBMC",
				value ? VPC_SBMC_RAPID_CHARGE_ON :
					VPC_SBMC_RAPID_CHARGE_OFF);
	default:
		return -EINVAL;
	}
}

/**
 * legion_acpi_vpc_get() - Read the state of a feature
 * @priv: The driver data.
 * @feature: The feature.
 * @value: The state (0 or 1).
 *
 * Context: Takes priv->mutex.
 * Return: 0, -EOPNOTSUPP for an unsupported feature or an error of the ACPI method.
 */
static int legion_acpi_vpc_get(struct legion_acpi_vpc_priv *priv,
			       enum legion_acpi_vpc_feature feature,
			       unsigned int *value)
{
	if (!READ_ONCE(priv->supported[feature]))
		return -EOPNOTSUPP;

	guard(mutex)(&priv->mutex);

	return legion_acpi_vpc_read(priv, feature, value);
}

/**
 * legion_acpi_vpc_set() - Set the state of a feature and verify it
 * @priv: The driver data.
 * @feature: The feature.
 * @value: The state (0 or 1).
 * @current_value: The state read back after the write.
 *
 * Conservation mode and rapid charge are mutually exclusive (the same as the manufacturer utilities): switching
 * one of them on switches the other one off.
 *
 * The state is read back: the firmware may report a feature (e.g. camera power by _CFG) which it does not control
 * on the model and ignore the write. Such a feature is marked as not supported.
 *
 * Context: Takes priv->mutex.
 * Return: 0, -EOPNOTSUPP for an unsupported feature, -EIO for a write ignored by the firmware or an error of the
 * ACPI method.
 */
static int legion_acpi_vpc_set(struct legion_acpi_vpc_priv *priv,
			       enum legion_acpi_vpc_feature feature,
			       unsigned int value, unsigned int *current_value)
{
	int err;

	if (!READ_ONCE(priv->supported[feature]))
		return -EOPNOTSUPP;

	guard(mutex)(&priv->mutex);

	err = legion_acpi_vpc_write(priv, feature, value);
	if (err)
		return err;

	err = legion_acpi_vpc_read(priv, feature, current_value);
	if (err)
		return err;

	if (*current_value != value) {
		WRITE_ONCE(priv->supported[feature], false);
		dev_warn(priv->fw_attr_dev,
			 "%s: write of %u was ignored by the firmware (reads %u), the feature is not supported\n",
			 legion_acpi_vpc_names[feature], value,
			 *current_value);
		return -EIO;
	}

	return 0;
}

/*
 * Notify user space about a changed feature (EVENT_TYPE: legion_events_type, EVENT_VALUE: state)
 */
static void legion_acpi_vpc_send_event(struct legion_acpi_vpc_priv *priv,
				       enum legion_acpi_vpc_feature feature,
				       unsigned int value)
{
	char event_type[32];
	char event_value[32];
	char *envp[] = { event_type, event_value, NULL };

	snprintf(event_type, sizeof(event_type), "EVENT_TYPE=%u",
		 (unsigned int)LEGION_ACPI_VPC_CONSERVATION_MODE + feature);
	snprintf(event_value, sizeof(event_value), "EVENT_VALUE=%u", value);

	if (kobject_uevent_env(&priv->fw_attr_dev->kobj, KOBJ_CHANGE, envp))
		dev_err(priv->fw_attr_dev,
			"Failed to send uevent for attribute change\n");
}

static struct legion_acpi_vpc_priv *legion_acpi_vpc_priv(struct kobject *kobj)
{
	/* kobj is the kset "attributes", its parent the class device */
	return dev_get_drvdata(kobj_to_dev(kobj->parent));
}

static ssize_t legion_acpi_vpc_current_show(struct kobject *kobj, char *buf,
					    enum legion_acpi_vpc_feature feature)
{
	struct legion_acpi_vpc_priv *priv = legion_acpi_vpc_priv(kobj);
	unsigned int value;
	int err;

	if (!priv)
		return -ENODEV;

	err = legion_acpi_vpc_get(priv, feature, &value);
	if (err)
		return err;

	return sysfs_emit(buf, "%u\n", value);
}

static ssize_t legion_acpi_vpc_current_store(struct kobject *kobj,
					     const char *buf, size_t count,
					     enum legion_acpi_vpc_feature feature)
{
	struct legion_acpi_vpc_priv *priv = legion_acpi_vpc_priv(kobj);
	unsigned int value, current_value;
	int err;

	if (!priv)
		return -ENODEV;

	err = kstrtouint(buf, 10, &value);
	if (err)
		return err;

	if (value > 1)
		return -EINVAL;

	err = legion_acpi_vpc_set(priv, feature, value, &current_value);

	/*
	 * Feature found unsupported by the write: user space reads the attributes again (supported is 0 now)
	 */
	if (err == -EIO && !READ_ONCE(priv->supported[feature]))
		legion_acpi_vpc_send_event(priv, feature, current_value);

	if (err)
		return err;

	legion_acpi_vpc_send_event(priv, feature, value);

	return count;
}

static ssize_t legion_acpi_vpc_supported_show(struct kobject *kobj, char *buf,
					      enum legion_acpi_vpc_feature feature)
{
	struct legion_acpi_vpc_priv *priv = legion_acpi_vpc_priv(kobj);

	if (!priv)
		return -ENODEV;

	return sysfs_emit(buf, "%u\n", READ_ONCE(priv->supported[feature]) ? 1 : 0);
}

/*
 * Group of a feature: current_value (0/1), supported, type, display_name
 */
#define LEGION_ACPI_VPC_FEATURE(_name, _feature, _label)                            \
	static ssize_t func_##_name##_current_get(struct kobject *kobj,            \
						  struct kobj_attribute *kattr,    \
						  char *buf)                       \
	{                                                                          \
		return legion_acpi_vpc_current_show(kobj, buf, _feature);          \
	}                                                                          \
	static ssize_t func_##_name##_current_set(struct kobject *kobj,            \
						  struct kobj_attribute *kattr,    \
						  const char *buf, size_t count)   \
	{                                                                          \
		return legion_acpi_vpc_current_store(kobj, buf, count, _feature);  \
	}                                                                          \
	static struct kobj_attribute kobj_attr_##_name##_current =                 \
		LEGION_WMI_ATTR_RW(current_value, _name##_current);                \
	static ssize_t func_##_name##_supported_show(struct kobject *kobj,         \
						     struct kobj_attribute *kattr, \
						     char *buf)                    \
	{                                                                          \
		return legion_acpi_vpc_supported_show(kobj, buf, _feature);        \
	}                                                                          \
	static struct kobj_attribute kobj_attr_##_name##_supported =               \
		LEGION_WMI_ATTR_RO(supported, _name##_supported);                  \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(type, "boolean", _name##_type);      \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name, _label,                \
					      _name##_display_name);               \
	static struct attribute *legion_acpi_vpc_##_name##_attrs[] = {             \
		&kobj_attr_##_name##_current.attr,                                 \
		&kobj_attr_##_name##_supported.attr,                               \
		&kobj_attr_##_name##_type.attr,                                    \
		&kobj_attr_##_name##_display_name.attr,                            \
		NULL                                                               \
	}

LEGION_ACPI_VPC_FEATURE(conservation_mode, VPC_CONSERVATION_MODE,
			"Battery conservation mode (charge limit)");
LEGION_ACPI_VPC_FEATURE(usb_charging, VPC_USB_CHARGING,
			"USB charging when the notebook is off");
LEGION_ACPI_VPC_FEATURE(fn_lock, VPC_FN_LOCK, "Fn lock");
LEGION_ACPI_VPC_FEATURE(camera_power, VPC_CAMERA_POWER, "Camera power");
LEGION_ACPI_VPC_FEATURE(rapid_charge, VPC_RAPID_CHARGE, "Rapid charge");

static const struct attribute_group legion_acpi_vpc_groups[] = {
	{ .name = "conservation_mode",
	  .attrs = legion_acpi_vpc_conservation_mode_attrs },
	{ .name = "usb_charging", .attrs = legion_acpi_vpc_usb_charging_attrs },
	{ .name = "fn_lock", .attrs = legion_acpi_vpc_fn_lock_attrs },
	{ .name = "camera_power", .attrs = legion_acpi_vpc_camera_power_attrs },
	{ .name = "rapid_charge", .attrs = legion_acpi_vpc_rapid_charge_attrs },
};

/**
 * legion_acpi_vpc_fw_attr_add() - Register the firmware attributes
 * @priv: The driver data.
 *
 * Return: Either 0, or an error code.
 */
static int legion_acpi_vpc_fw_attr_add(struct legion_acpi_vpc_priv *priv)
{
	unsigned int i;
	int err;

	priv->ida_id = ida_alloc(&legion_acpi_vpc_ida, GFP_KERNEL);
	if (priv->ida_id < 0)
		return priv->ida_id;

	priv->fw_attr_dev = device_create(&legion_firmware_attributes_class,
					  NULL, MKDEV(0, 0), priv, "%s-%u",
					  LEGION_ACPI_VPC_FW_ATTR_BASE_PATH,
					  priv->ida_id);
	if (IS_ERR(priv->fw_attr_dev)) {
		err = PTR_ERR(priv->fw_attr_dev);
		goto err_free_ida;
	}

	priv->fw_attr_kset = kset_create_and_add("attributes", NULL,
						 &priv->fw_attr_dev->kobj);
	if (!priv->fw_attr_kset) {
		err = -ENOMEM;
		goto err_unregister_fw_attr_dev;
	}

	for (i = 0; i < ARRAY_SIZE(legion_acpi_vpc_groups); i++) {
		err = sysfs_create_group(&priv->fw_attr_kset->kobj,
					 &legion_acpi_vpc_groups[i]);
		if (err)
			goto err_remove_groups;
	}

	return 0;

err_remove_groups:
	while (i--)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,
				   &legion_acpi_vpc_groups[i]);
	kset_unregister(priv->fw_attr_kset);
err_unregister_fw_attr_dev:
	device_unregister(priv->fw_attr_dev);
err_free_ida:
	ida_free(&legion_acpi_vpc_ida, priv->ida_id);
	return err;
}

/**
 * legion_acpi_vpc_fw_attr_remove() - Unregister the firmware attributes
 * @priv: The driver data.
 */
static void legion_acpi_vpc_fw_attr_remove(struct legion_acpi_vpc_priv *priv)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(legion_acpi_vpc_groups); i++)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,
				   &legion_acpi_vpc_groups[i]);

	kset_unregister(priv->fw_attr_kset);
	device_unregister(priv->fw_attr_dev);
	ida_free(&legion_acpi_vpc_ida, priv->ida_id);
}

#ifdef LEGION_ACPI_VPC_BATTERY_EXT
/*
 * charge_types of the battery: Fast = rapid charge, Long_Life = conservation mode, Standard = none of them
 */
static int legion_acpi_vpc_psy_get(struct power_supply *psy,
				   const struct power_supply_ext *ext, void *data,
				   enum power_supply_property psp,
				   union power_supply_propval *val)
{
	struct legion_acpi_vpc_priv *priv = data;
	unsigned int conservation = 0, rapid = 0;
	int err;

	scoped_guard(mutex, &priv->mutex) {
		err = legion_acpi_vpc_read(priv, VPC_CONSERVATION_MODE,
					   &conservation);
		if (err)
			return err;

		if (READ_ONCE(priv->supported[VPC_RAPID_CHARGE])) {
			err = legion_acpi_vpc_read(priv, VPC_RAPID_CHARGE,
						   &rapid);
			if (err)
				return err;
		}
	}

	if (rapid && conservation) {
		dev_err(priv->dev,
			"unexpected charge_types: both Fast and Long_Life are enabled\n");
		return -EINVAL;
	}

	if (rapid)
		val->intval = POWER_SUPPLY_CHARGE_TYPE_FAST;
	else if (conservation)
		val->intval = POWER_SUPPLY_CHARGE_TYPE_LONGLIFE;
	else
		val->intval = POWER_SUPPLY_CHARGE_TYPE_STANDARD;

	return 0;
}

static int legion_acpi_vpc_psy_set(struct power_supply *psy,
				   const struct power_supply_ext *ext, void *data,
				   enum power_supply_property psp,
				   const union power_supply_propval *val)
{
	struct legion_acpi_vpc_priv *priv = data;
	unsigned int current_value;
	int err;

	switch (val->intval) {
	case POWER_SUPPLY_CHARGE_TYPE_FAST:
		err = legion_acpi_vpc_set(priv, VPC_RAPID_CHARGE, 1,
					  &current_value);
		break;
	case POWER_SUPPLY_CHARGE_TYPE_LONGLIFE:
		err = legion_acpi_vpc_set(priv, VPC_CONSERVATION_MODE, 1,
					  &current_value);
		break;
	case POWER_SUPPLY_CHARGE_TYPE_STANDARD:
		err = legion_acpi_vpc_set(priv, VPC_CONSERVATION_MODE, 0,
					  &current_value);
		if (!err && READ_ONCE(priv->supported[VPC_RAPID_CHARGE]))
			err = legion_acpi_vpc_set(priv, VPC_RAPID_CHARGE, 0,
						  &current_value);
		break;
	default:
		return -EINVAL;
	}

	if (err)
		return err;

	/* The daemon reads the firmware attributes again */
	legion_acpi_vpc_send_event(priv, VPC_CONSERVATION_MODE,
				   val->intval ==
					   POWER_SUPPLY_CHARGE_TYPE_LONGLIFE);

	return 0;
}

static int legion_acpi_vpc_psy_is_writeable(struct power_supply *psy,
					    const struct power_supply_ext *ext,
					    void *data,
					    enum power_supply_property psp)
{
	return 1;
}

static const enum power_supply_property legion_acpi_vpc_psy_props[] = {
	POWER_SUPPLY_PROP_CHARGE_TYPES,
};

#define LEGION_ACPI_VPC_BATTERY_EXTENSION(_name, _charge_types)                \
	static const struct power_supply_ext _name = {                       \
		.name = "legion_acpi_vpc",                                   \
		.properties = legion_acpi_vpc_psy_props,                     \
		.num_properties = ARRAY_SIZE(legion_acpi_vpc_psy_props),     \
		.charge_types = _charge_types,                               \
		.get_property = legion_acpi_vpc_psy_get,                     \
		.set_property = legion_acpi_vpc_psy_set,                     \
		.property_is_writeable = legion_acpi_vpc_psy_is_writeable,   \
	}

LEGION_ACPI_VPC_BATTERY_EXTENSION(legion_acpi_vpc_battery_ext,
				  BIT(POWER_SUPPLY_CHARGE_TYPE_STANDARD) |
					  BIT(POWER_SUPPLY_CHARGE_TYPE_LONGLIFE));

LEGION_ACPI_VPC_BATTERY_EXTENSION(legion_acpi_vpc_battery_ext_rapid,
				  BIT(POWER_SUPPLY_CHARGE_TYPE_STANDARD) |
					  BIT(POWER_SUPPLY_CHARGE_TYPE_FAST) |
					  BIT(POWER_SUPPLY_CHARGE_TYPE_LONGLIFE));

static int legion_acpi_vpc_battery_add(struct power_supply *battery,
				       struct acpi_battery_hook *hook)
{
	struct legion_acpi_vpc_priv *priv =
		container_of(hook, struct legion_acpi_vpc_priv, battery_hook);

	return power_supply_register_extension(battery, priv->battery_ext,
					       priv->dev, priv);
}

static int legion_acpi_vpc_battery_remove(struct power_supply *battery,
					  struct acpi_battery_hook *hook)
{
	struct legion_acpi_vpc_priv *priv =
		container_of(hook, struct legion_acpi_vpc_priv, battery_hook);

	power_supply_unregister_extension(battery, priv->battery_ext);

	return 0;
}

/*
 * Registered only with the conservation mode, unregistered in remove before the firmware attributes
 * (the extension sends their uevents)
 */
static void legion_acpi_vpc_battery_register(struct legion_acpi_vpc_priv *priv)
{
	if (!priv->supported[VPC_CONSERVATION_MODE])
		return;

	priv->battery_ext = priv->supported[VPC_RAPID_CHARGE] ?
				    &legion_acpi_vpc_battery_ext_rapid :
				    &legion_acpi_vpc_battery_ext;
	priv->battery_hook.name = "Legion VPC Battery Extension";
	priv->battery_hook.add_battery = legion_acpi_vpc_battery_add;
	priv->battery_hook.remove_battery = legion_acpi_vpc_battery_remove;

	legion_battery_hook_register(&priv->battery_hook);
	priv->battery_hook_registered = true;
}

static void legion_acpi_vpc_battery_unregister(struct legion_acpi_vpc_priv *priv)
{
	if (priv->battery_hook_registered)
		legion_battery_hook_unregister(&priv->battery_hook);
}
#else
static void legion_acpi_vpc_battery_register(struct legion_acpi_vpc_priv *priv)
{
}

static void legion_acpi_vpc_battery_unregister(struct legion_acpi_vpc_priv *priv)
{
}
#endif

/**
 * legion_acpi_vpc_check_features() - Find the features supported by the firmware
 * @priv: The driver data.
 * @cfg: The capabilities (_CFG).
 *
 * The same checks as ideapad_laptop: methods present and their capability bits.
 */
static void legion_acpi_vpc_check_features(struct legion_acpi_vpc_priv *priv,
					   unsigned long cfg)
{
	acpi_handle handle = priv->adev->handle;
	unsigned long val;

	if (acpi_has_method(handle, "GBMD") && acpi_has_method(handle, "SBMC") &&
	    !vpc_eval_int(handle, "GBMD", &val)) {
		priv->supported[VPC_CONSERVATION_MODE] = true;
		priv->supported[VPC_RAPID_CHARGE] =
			test_bit(VPC_GBMD_RAPID_CHARGE_SUPPORTED_BIT, &val);
	}

	if (acpi_has_method(handle, "HALS") && acpi_has_method(handle, "SALS") &&
	    !vpc_eval_int(handle, "HALS", &val)) {
		priv->supported[VPC_USB_CHARGING] =
			test_bit(VPC_HALS_USB_CHARGING_SUPPORT_BIT, &val);
		priv->supported[VPC_FN_LOCK] =
			test_bit(VPC_HALS_FNLOCK_SUPPORT_BIT, &val);
	}

	priv->supported[VPC_CAMERA_POWER] =
		test_bit(VPC_CFG_CAP_CAM_BIT, &cfg) &&
		acpi_has_method(handle, "VPCR") &&
		acpi_has_method(handle, "VPCW");
}

static int legion_acpi_vpc_probe(struct platform_device *pdev)
{
	struct acpi_device *adev = ACPI_COMPANION(&pdev->dev);
	struct legion_acpi_vpc_priv *priv;
	unsigned long cfg;
	int err;

	if (!adev || vpc_eval_int(adev->handle, "_CFG", &cfg))
		return -ENODEV;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->adev = adev;
#ifdef LEGION_ACPI_VPC_BATTERY_EXT
	priv->dev = &pdev->dev;
#endif

	err = devm_mutex_init(&pdev->dev, &priv->mutex);
	if (err)
		return err;

	legion_acpi_vpc_check_features(priv, cfg);

	dev_info(&pdev->dev,
		 "conservation mode %u, rapid charge %u, USB charging %u, Fn lock %u, camera power %u\n",
		 priv->supported[VPC_CONSERVATION_MODE],
		 priv->supported[VPC_RAPID_CHARGE],
		 priv->supported[VPC_USB_CHARGING], priv->supported[VPC_FN_LOCK],
		 priv->supported[VPC_CAMERA_POWER]);

	platform_set_drvdata(pdev, priv);

	err = legion_acpi_vpc_fw_attr_add(priv);
	if (err)
		return err;

	legion_acpi_vpc_battery_register(priv);

	return 0;
}

static void legion_acpi_vpc_remove(struct platform_device *pdev)
{
	struct legion_acpi_vpc_priv *priv = platform_get_drvdata(pdev);

	legion_acpi_vpc_battery_unregister(priv);
	legion_acpi_vpc_fw_attr_remove(priv);
}

static const struct acpi_device_id legion_acpi_vpc_ids[] = {
	{ "VPC2004", 0 },
	{}
};
MODULE_DEVICE_TABLE(acpi, legion_acpi_vpc_ids);

static struct platform_driver legion_acpi_vpc_driver = {
	.driver = {
		.name = "legion_acpi_vpc",
		.acpi_match_table = legion_acpi_vpc_ids,
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
	},
	.probe = legion_acpi_vpc_probe,
	.remove = legion_acpi_vpc_remove,
};

int legion_acpi_vpc_driver_init(void)
{
	return platform_driver_register(&legion_acpi_vpc_driver);
}

void legion_acpi_vpc_driver_exit(void)
{
	platform_driver_unregister(&legion_acpi_vpc_driver);
}
