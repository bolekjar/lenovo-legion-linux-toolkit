// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the firmware attributes of the lenovo-wmi-other driver of the Linux kernel
 * (drivers/platform/x86/lenovo/wmi-other.c),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */

#include "legion-common.h"
#include "legion-wmi-ddata.h"
#include "legion-wmi-other-sysfs.h"
#include "legion-wmi-other.h"
#include "legion-wmi-helpers.h"
#include "legion-wmi-capdata01.h"
#include "legion-wmi-capdata00.h"
#include "legion-wmi-gamezone.h"
#include "legion-firmware-attributes-class.h"

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/component.h>
#include <linux/container_of.h>
#include <linux/device.h>
#include <linux/gfp_types.h>
#include <linux/idr.h>
#include <linux/wmi.h>
#include <linux/bitfield.h>

static DEFINE_IDA(legion_wmi_other_ida);

#define LEGION_WMI_OTHER_FW_ATTR_BASE_PATH "legion-wmi-other"

enum attribute_property {
	DEFAULT_VAL,
	MAX_VAL,
	MIN_VAL,
	STEP_VAL,
	SUPPORTED,
	STEPS_VAL
};

static ssize_t legion_wmi_other_cap01_attr_show_no_steps_val(
	struct legion_wmi_cd01_list *cd01_list, enum attribute_property prop,
	const u32 attribute_id_raw, enum thermal_mode modes[THERMAL_MODES_SIZE],
	char *buf, ssize_t *buf_len)
{
	struct legion_wmi_capdata01 capdata;
	int len = 0;

	modes[THERMAL_MODES_SIZE - 1] = LEGION_WMI_GZ_THERMAL_MODE_END;
	for (int i = 0; (i < THERMAL_MODES_SIZE) &&
			(modes[i] != LEGION_WMI_GZ_THERMAL_MODE_END);
	     i++) {
		int ret = 0;
		u32 value = 0;

		len += sysfs_emit_at(buf, len, "%d=", modes[i]);

		ret = legion_wmi_cd01_get_data(
			cd01_list,
			attribute_id_raw |
				FIELD_PREP(LEGION_WMI_MODE_ID_MASK, modes[i]),
			&capdata);
		if (ret) {
			if (modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)
				len += sysfs_emit_at(buf, len, ",");
			continue;
		}

		switch (prop) {
		case DEFAULT_VAL:
			value = capdata.default_value;
			break;
		case MAX_VAL:
			value = capdata.max_value;
			break;
		case MIN_VAL:
			value = capdata.min_value;
			break;
		case STEP_VAL:
			value = capdata.step;
			break;
		case SUPPORTED:
			value = capdata.supported;
			break;
		default:
			return -EINVAL;
		}

		len += sysfs_emit_at(buf, len, "%u", value);

		if (modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)
			len += sysfs_emit_at(buf, len, ",");
	}
	(*buf_len) += len;

	return 0;
}

static ssize_t legion_wmi_other_cap00_show(
	struct legion_wmi_cd00_list *cd00_list, enum attribute_property prop,
	const u32 attribute_id_raw, char *buf, ssize_t *buf_len)
{
	struct legion_wmi_capdata00 capdata;
	int len = 0;
	u32 value = 0;

	const int ret =
		legion_wmi_cd00_get_data(cd00_list, attribute_id_raw, &capdata);
	if (ret) {
		if (ret == -EINVAL) {
			len += sysfs_emit_at(buf, len, "%u", value);
			(*buf_len) += len;
			return 0;
		} else {
			return ret;
		}
	}

	switch (prop) {
	case DEFAULT_VAL:
		value = capdata.default_value;
		break;
	case SUPPORTED:
		value = capdata.supported;
		break;
	default:
		return -EINVAL;
	}

	len += sysfs_emit_at(buf, len, "%u", value);
	(*buf_len) += len;
	return 0;
}

static ssize_t legion_wmi_other_cap01_attr_show_steps_val(
	struct legion_wmi_dd_list *dd_list, const u32 attribute_id_raw,
	enum thermal_mode modes[THERMAL_MODES_SIZE], char *buf,
	ssize_t *buf_len)
{
	struct legion_wmi_ddata lddata[LEGION_WMI_DD_MAX_VALUES];
	int len = 0;

	modes[THERMAL_MODES_SIZE - 1] = LEGION_WMI_GZ_THERMAL_MODE_END;
	for (int i = 0; (i < THERMAL_MODES_SIZE) &&
			(modes[i] != LEGION_WMI_GZ_THERMAL_MODE_END);
	     i++) {
		len += sysfs_emit_at(buf, len, "%d=", modes[i]);

		/* Values of this mode only (count), the array is shared by the modes */
		const int count = legion_wmi_dd_get_data(
			dd_list,
			attribute_id_raw |
				FIELD_PREP(LEGION_WMI_MODE_ID_MASK, modes[i]),
			lddata, ARRAY_SIZE(lddata));

		for (int j = 0; j < count; ++j) {
			len += sysfs_emit_at(buf, len, "%u", lddata[j].value);

			if (j + 1 < count)
				len += sysfs_emit_at(buf, len, "|");
		}

		if (modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)
			len += sysfs_emit_at(buf, len, ",");
	}

	(*buf_len) += len;
	return 0;
}

/**
 * legion_wmi_other_cap01_attr_show() - Get the value of the specified attribute property
 *
 * @kobj: Pointer to the driver object.
 * @kattr: Pointer to the attribute calling this function.
 * @buf: The buffer to write to.
 * @tunable_attr: The attribute to be read.
 * @prop: The property of this attribute to be read.
 *
 * Retrieves the given property from the capability data 01 struct for the
 * specified attribute's "custom" thermal mode. This function is intended
 * to be generic so it can be called from any integer attributes "_show"
 * function.
 *
 * If the WMI is success the sysfs attribute is notified.
 *
 * Return: Either number of characters written to buf, or an error code.
 */
static ssize_t legion_wmi_other_cap01_attr_show(
	struct kobject *kobj, struct kobj_attribute *kattr, char *buf,
	const enum attribute_property prop, const u32 attribute_id)
{
	struct legion_wmi_other_priv *priv =
		dev_get_drvdata(kobj_to_dev(kobj->parent));
	ssize_t len = 0;
	enum thermal_mode modes[THERMAL_MODES_SIZE] = {
		LEGION_WMI_GZ_THERMAL_MODE_END
	};

	ssize_t ret = legion_wmi_other_notifier_call(
		&modes, LEGION_WMI_GZ_GET_SUPPORTED_THERMAL_MODES);
	if (ret)
		return ret;

	if (prop == STEPS_VAL) {
		ret = legion_wmi_other_cap01_attr_show_steps_val(
			priv->dd_list,
			attribute_id & (~LEGION_WMI_MODE_ID_MASK), modes, buf,
			&len);
		if (ret)
			return ret;
	} else {
		ret = legion_wmi_other_cap01_attr_show_no_steps_val(
			priv->cd01_list, prop,
			attribute_id & (~LEGION_WMI_MODE_ID_MASK), modes, buf,
			&len);
		if (ret)
			return ret;
	}

	len += sysfs_emit_at(buf, (int)len, "\n");
	return len;
}

static ssize_t legion_wmi_other_cap00_attr_show(struct kobject *kobj,
						struct kobj_attribute *kattr,
						char *buf,
						enum attribute_property prop,
						u32 attribute_id)
{
	const struct legion_wmi_other_priv *priv =
		dev_get_drvdata(kobj_to_dev(kobj->parent));
	ssize_t len = 0;
	enum thermal_mode modes[THERMAL_MODES_SIZE] = {
		LEGION_WMI_GZ_THERMAL_MODE_END
	};

	ssize_t ret = legion_wmi_other_notifier_call(
		&modes, LEGION_WMI_GZ_GET_SUPPORTED_THERMAL_MODES);
	if (ret)
		return ret;

	ret = legion_wmi_other_cap00_show(priv->cd00_list, prop, attribute_id,
					  buf, &len);
	if (ret)
		return ret;

	len += sysfs_emit_at(buf, (int)len, "\n");
	return len;
}

/*
 * Uevent of a written value (EVENT_TYPE, EVENT_VALUE), user space reads the attribute again
 */
static void legion_wmi_other_send_event(const struct legion_wmi_other_priv *priv,
					enum legion_events_type event_type_name,
					u32 value)
{
	char event_type[64] = { 0 };
	char event_value[64] = { 0 };
	char *envp[] = { event_type, event_value, NULL };

	snprintf(event_type, sizeof(event_type), "EVENT_TYPE=%u",
		 (unsigned int)event_type_name);
	snprintf(event_value, sizeof(event_value), "EVENT_VALUE=%u", value);

	if (kobject_uevent_env(&priv->wdev->dev.kobj, KOBJ_CHANGE, envp))
		dev_err(&priv->wdev->dev,
			"Failed to send uevent for attribute change\n");
}

/**
 * legion_wmi_attr_current_value_store() - Set the current value of the given attribute
 * @kobj: Pointer to the driver object.
 * @kattr: Pointer to the attribute calling this function.
 * @buf: The buffer to read from, this is parsed to `int` type.
 * @count: Required by sysfs attribute macros, pass in from the callee attr.
 * @tunable_attr: The attribute to be stored.
 *
 * Parses the value and sets it by legion_wmi_other_feature_set() (custom mode
 * only, checked by the capability data). This function is intended to be
 * generic so it can be called from any integer attribute's "_store" function.
 *
 * If the value is valid and WMI is success, then the sysfs attribute is
 * notified.
 *
 * Return: Either count, or an error code.
 */
static ssize_t legion_wmi_attr_current_value_store(
	struct kobject *kobj, struct kobj_attribute *kattr, const char *buf,
	const size_t count, const u32 attribute_id,
	const enum legion_events_type event_type_name)
{
	const struct legion_wmi_other_priv *priv =
		dev_get_drvdata(kobj_to_dev(kobj->parent));
	u32 value = 0;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	ret = legion_wmi_other_feature_set(priv, attribute_id, value);
	if (ret)
		return ret;

	legion_wmi_other_send_event(priv, event_type_name, value);

	return (ssize_t)count;
};

static ssize_t legion_wmi_attr_current_value_store_min_max(
	struct kobject *kobj, struct kobj_attribute *kattr, const char *buf,
	const size_t count, const u32 attribute_id, const int min,
	const int max, const enum legion_events_type event_type_name)
{
	const struct legion_wmi_other_priv *priv =
		dev_get_drvdata(kobj_to_dev(kobj->parent));
	u32 value = 0;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	ret = legion_wmi_other_feature_set_range(priv, attribute_id, value, min,
						 max);
	if (ret)
		return ret;

	legion_wmi_other_send_event(priv, event_type_name, value);

	return (ssize_t)count;
};

/**
 * legion_wmi_attr_current_value_show() - Get the current value of the given attribute
 * @kobj: Pointer to the driver object.
 * @kattr: Pointer to the attribute calling this function.
 * @buf: The buffer to write to.
 * @tunable_attr: The attribute to be read.
 *
 * Retrieves the value of the given attribute for the current smartfan profile.
 * The current smartfan profile is retrieved from the lenovo-wmi-gamezone driver.
 * This function is intended to be generic so it can be called from any integer
 * attribute's "_show" function.
 *
 * If the WMI is success the sysfs attribute is notified.
 *
 * Return: Either number of characters written to buf, or an error code.
 */
static ssize_t legion_wmi_attr_current_value_show(struct kobject *kobj,
						  struct kobj_attribute *kattr,
						  char *buf,
						  const u32 attribute_id)
{
	const struct legion_wmi_other_priv *priv =
		dev_get_drvdata(kobj_to_dev(kobj->parent));
	u32 retval;
	const int ret = legion_wmi_other_feature_get(priv, attribute_id, &retval);

	if (ret)
		return ret;

	return sysfs_emit(buf, "%u\n", retval);
}

/* Attribute property read only */
#define LEGION_WMI_RO_CAP01(_attrname, _func, _prop_type, _attr_id)            \
	static ssize_t func_##_func##_show(                                    \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) \
	{                                                                      \
		return legion_wmi_other_cap01_attr_show(kobj, kattr, buf,      \
							_prop_type, _attr_id); \
	}                                                                      \
	static struct kobj_attribute kobj_attr_##_func##_prop =                \
		LEGION_WMI_ATTR_RO(_attrname, _func)

/* Attribute property read only */
#define LEGION_WMI_RO_CAP00(_attrname, _func, _prop_type, _attr_id)            \
	static ssize_t func_##_func##_show(                                    \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) \
	{                                                                      \
		return legion_wmi_other_cap00_attr_show(kobj, kattr, buf,      \
							_prop_type, _attr_id); \
	}                                                                      \
	static struct kobj_attribute kobj_attr_##_func##_prop =                \
		LEGION_WMI_ATTR_RO(_attrname, _func)

/* Attribute current value read/write */
#define LEGION_WMI_CURRENT_VALUE(_attrname, _func, _attr_id, _name_notify)     \
	static ssize_t func_##_func##_set(struct kobject *kobj,                \
					  struct kobj_attribute *kattr,        \
					  const char *buf, size_t count)       \
	{                                                                      \
		return legion_wmi_attr_current_value_store(                    \
			kobj, kattr, buf, count, _attr_id, _name_notify);      \
	}                                                                      \
	static ssize_t func_##_func##_get(                                     \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) \
	{                                                                      \
		return legion_wmi_attr_current_value_show(kobj, kattr, buf,    \
							  _attr_id);           \
	}                                                                      \
	static struct kobj_attribute kobj_attr_##_func##_prop =                \
		LEGION_WMI_ATTR_RW(_attrname, _func)

/* Attribute current value read/write */
#define LEGION_WMI_CURRENT_MIN_MAX(_attrname, _func, _attr_id, _min, _max,     \
				   _name_notify)                               \
	static ssize_t func_##_func##_set(struct kobject *kobj,                \
					  struct kobj_attribute *kattr,        \
					  const char *buf, size_t count)       \
	{                                                                      \
		return legion_wmi_attr_current_value_store_min_max(            \
			kobj, kattr, buf, count, _attr_id, _min, _max,         \
			_name_notify);                                         \
	}                                                                      \
	static ssize_t func_##_func##_get(                                     \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) \
	{                                                                      \
		return legion_wmi_attr_current_value_show(kobj, kattr, buf,    \
							  _attr_id);           \
	}                                                                      \
	static struct kobj_attribute kobj_attr_##_func##_prop =                \
		LEGION_WMI_ATTR_RW(_attrname, _func)

#define LEGION_WMI_OTHER_CAP01(_attrname, _attr_id, _name, _name_notify)     \
	LEGION_WMI_RO_CAP01(default_value, _attrname##_default_value,        \
			    DEFAULT_VAL, _attr_id);                          \
	LEGION_WMI_RO_CAP01(max_value, _attrname##_max_value, MAX_VAL,       \
			    _attr_id);                                       \
	LEGION_WMI_RO_CAP01(min_value, _attrname##_min_value, MIN_VAL,       \
			    _attr_id);                                       \
	LEGION_WMI_RO_CAP01(scalar_increment, _attrname##_scalar_increment,  \
			    STEP_VAL, _attr_id);                             \
	LEGION_WMI_RO_CAP01(steps, _attrname##_steps, STEPS_VAL, _attr_id);  \
	LEGION_WMI_RO_CAP01(supported, _attrname##_supported, SUPPORTED,     \
			    _attr_id);                                       \
	LEGION_WMI_CURRENT_VALUE(current_value, _attrname##_current_value,   \
				 _attr_id, _name_notify);                    \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(type, "integer",               \
					      _attrname##_type);             \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name, _name,           \
					      _attrname##_display_name);     \
									     \
	static struct attribute *legion_sysfs_##_attrname##_attributes[] = { \
		&kobj_attr_##_attrname##_default_value_prop.attr,            \
		&kobj_attr_##_attrname##_max_value_prop.attr,                \
		&kobj_attr_##_attrname##_min_value_prop.attr,                \
		&kobj_attr_##_attrname##_scalar_increment_prop.attr,         \
		&kobj_attr_##_attrname##_steps_prop.attr,                    \
		&kobj_attr_##_attrname##_supported_prop.attr,                \
		&kobj_attr_##_attrname##_current_value_prop.attr,            \
		&kobj_attr_##_attrname##_type.attr,                          \
		&kobj_attr_##_attrname##_display_name.attr,                  \
		NULL                                                         \
	};

#define LEGION_WMI_OTHER_CAP0(_attrname, _attr_id, _name, _value_type, _min, \
			      _max, _name_notify)                            \
	LEGION_WMI_RO_CAP00(default_value, _attrname##_default_value,        \
			    DEFAULT_VAL, _attr_id);                          \
	LEGION_WMI_RO_CAP00(supported, _attrname##_supported, SUPPORTED,     \
			    _attr_id);                                       \
	LEGION_WMI_CURRENT_MIN_MAX(current_value, _attrname##_current_value, \
				   _attr_id, _min, _max, _name_notify);      \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(type, _value_type,             \
					      _attrname##_type);             \
	LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name, _name,           \
					      _attrname##_display_name);     \
									     \
	static struct attribute *legion_sysfs_##_attrname##_attributes[] = { \
		&kobj_attr_##_attrname##_default_value_prop.attr,            \
		&kobj_attr_##_attrname##_supported_prop.attr,                \
		&kobj_attr_##_attrname##_current_value_prop.attr,            \
		&kobj_attr_##_attrname##_type.attr,                          \
		&kobj_attr_##_attrname##_display_name.attr,                  \
		NULL                                                         \
	};

#define LEGION_WMI_OTHER_RO_VALUE(_attrname, _func, _attr_id)                  \
	static ssize_t func_##_func##_show(                                    \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) \
	{                                                                      \
		struct legion_wmi_other_priv *priv =                           \
			dev_get_drvdata(kobj_to_dev(kobj->parent));            \
		u32 retval;                                                    \
		int ret;                                                       \
									       \
		ret = legion_wmi_other_feature_get(priv, _attr_id, &retval);   \
		if (ret)                                                       \
			return ret;                                            \
									       \
		return sysfs_emit(buf, "%u\n", retval);                        \
	}                                                                      \
	static struct kobj_attribute kobj_attr_##_func##_prop =                \
		LEGION_WMI_ATTR_RO(_attrname, _func)

/*
 * CPU CAP01
 */
LEGION_WMI_OTHER_CAP01(cpu_stp_limit, CPUShortTermPowerLimit,
		       "Set the CPU short term power limit",
		       LENOVO_WMI_OTHER_CPU_STP);
LEGION_WMI_OTHER_CAP01(cpu_ltp_limit, CPULongTermPowerLimit,
		       "Set the CPU long term power limit",
		       LENOVO_WMI_OTHER_CPU_LTP);
LEGION_WMI_OTHER_CAP01(cpu_pp_limit, CPUPeakPowerLimit,
		       "Set the CPU peak power limit", LENOVO_WMI_OTHER_CPU_PP);
LEGION_WMI_OTHER_CAP01(cpu_tmp_limit, CPUTemperatureLimit,
		       "Set the CPU temperature limit",
		       LENOVO_WMI_OTHER_CPU_TMP);
LEGION_WMI_OTHER_CAP01(apus_pptp_limit, APUsPPTPowerLimit,
		       "Set the APUs ppt power limit",
		       LENOVO_WMI_OTHER_APU_PPT);
LEGION_WMI_OTHER_CAP01(cpu_clp_limit, CPUCrossLoadingPowerLimit,
		       "Set the CPU cross loading power limit",
		       LENOVO_WMI_OTHER_CPU_CLP);
LEGION_WMI_OTHER_CAP01(cpu_pl1_tau, CPUPL1Tau, "Set the CPU PL1 Tau limit",
		       LENOVO_WMI_OTHER_CPU_PL1_TAU);

/*
 * GPU CAP01
 */
LEGION_WMI_OTHER_CAP01(gpu_power_boost, GPUPowerBoost,
		       "Set the GPU power boost", LENOVO_WMI_OTHER_GPU_PB);
LEGION_WMI_OTHER_CAP01(gpu_configurable_tgp, GPUConfigurableTGP,
		       "Set the GPU configurable TGP",
		       LENOVO_WMI_OTHER_GPU_TGP);
LEGION_WMI_OTHER_CAP01(gpu_temperature_limit, GPUTemperatureLimit,
		       "Set the GPU temperature limit",
		       LENOVO_WMI_OTHER_GPU_TMP);
LEGION_WMI_OTHER_CAP01(
	gpu_total_onac, GPUTotalProcessingPowerTargetOnAcOffsetFromBaseline,
	"Set the GPU total processing power target on AC offset from base line limit",
	LENOVO_WMI_OTHER_GPU_TAC);
LEGION_WMI_OTHER_CAP01(gpu_to_cpu_dynamic_boost, GPUToCPUDynamicBoost,
		       "Set the GPU To CPU dynamic boost",
		       LENOVO_WMI_OTHER_GC_DB);

/*
 * CAP00
 */
LEGION_WMI_OTHER_CAP0(igpu_mode, IGPUMode, "IGPU mode", "integer", 0, 2,
		      LENOVO_WMI_OTHER_IGPU_M)
LEGION_WMI_OTHER_CAP0(instant_boot_ac, InstantBootAc, "Instant Boot", "boolean",
		      0, 1, LENOVO_WMI_OTHER_IB_AC)
LEGION_WMI_OTHER_CAP0(instant_boot_usb_power_delivery,
		      InstantBootUsbPowerDelivery,
		      "Instant boot USB power delivery", "boolean", 0, 1,
		      LENOVO_WMI_OTHER_IB_UPD)
LEGION_WMI_OTHER_CAP0(fan_full_speed, FanFullSpeed, "Fan Full Speed", "boolean",
		      0, 1, LENOVO_WMI_OTHER_FF_S)
LEGION_WMI_OTHER_CAP0(over_drive, OverDrive, "Over Drive", "boolean", 0, 1,
		      LENOVO_WMI_OTHER_OD)
LEGION_WMI_OTHER_CAP0(god_mode_fnq_switchable, GodModeFnQSwitchable,
		      "God mode FN+Q switchable", "boolean", 0, 1,
		      LENOVO_WMI_OTHER_GM_FNQ)
LEGION_WMI_OTHER_CAP0(nvidia_gpu_dd_switching, NvidiaGPUDynamicDisplaySwitching,
		      "NVIDIDA GPU dynamic switching", "boolean", 0, 1,
		      LENOVO_WMI_OTHER_NG_DDS)
LEGION_WMI_OTHER_CAP0(flip_to_start, FlipToStart, "Flip to start", "boolean", 0,
		      1, LENOVO_WMI_OTHER_FL_ST)
LEGION_WMI_OTHER_CAP0(skin_temperature_tracking, AMDSkinTemperatureTracking,
		      "AMD Skin temperature tracking", "boolean", 0, 1,
		      LENOVO_WMI_OTHER_AMD_STT)
LEGION_WMI_OTHER_CAP0(smart_shiftmode, AMDSmartShiftMode,
		      "AMD Smart shift mode", "boolean", 0, 1,
		      LENOVO_WMI_OTHER_SS_M)

/*
 * Other read-only values
 */
LEGION_WMI_OTHER_RO_VALUE(gpu_status, gpu_status, GPUStatus);
LEGION_WMI_OTHER_RO_VALUE(gpu_did_vid, gpu_did_vid, GPUDidVid);
LEGION_WMI_OTHER_RO_VALUE(igpu_mode_change_status, igpu_mode_change_status,
			  IGPUModeChangeStatus);
LEGION_WMI_OTHER_RO_VALUE(zone_support_version, zone_support_version,
			  LegionZoneSupportVersion);
LEGION_WMI_OTHER_RO_VALUE(supported_power_modes, supported_power_modes,
			  SupportedPowerModes);

LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name, "Other related settings",
				      gpu_other_display_name);

static struct attribute *legion_sysfs_others_attributes[] = {
	&kobj_attr_gpu_status_prop.attr,
	&kobj_attr_gpu_did_vid_prop.attr,
	&kobj_attr_igpu_mode_change_status_prop.attr,
	&kobj_attr_zone_support_version_prop.attr,
	&kobj_attr_supported_power_modes_prop.attr,
	&kobj_attr_gpu_other_display_name.attr,
	NULL
};

/*
 * Feature of the firmware of an attribute group: its ID and the capability data
 */
struct legion_wmi_other_group_feature {
	const char *name; /* name of the group */
	struct attribute **attrs; /* attributes of the group */
	u32 id; /* capability ID */
	bool cd01; /* capability data 01 (by the power modes), otherwise 00 */
};

#define LEGION_WMI_OTHER_GROUP_FEATURE(_name, _id, _cd01)                   \
	{ .name = #_name, .attrs = legion_sysfs_##_name##_attributes, \
	  .id = (_id), .cd01 = (_cd01) }

static const struct legion_wmi_other_group_feature legion_wmi_other_group_features[] = {
	LEGION_WMI_OTHER_GROUP_FEATURE(cpu_stp_limit, CPUShortTermPowerLimit, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(cpu_ltp_limit, CPULongTermPowerLimit, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(cpu_pp_limit, CPUPeakPowerLimit, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(cpu_clp_limit, CPUCrossLoadingPowerLimit, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(cpu_tmp_limit, CPUTemperatureLimit, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(apus_pptp_limit, APUsPPTPowerLimit, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(cpu_pl1_tau, CPUPL1Tau, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(gpu_power_boost, GPUPowerBoost, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(gpu_configurable_tgp, GPUConfigurableTGP, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(gpu_temperature_limit, GPUTemperatureLimit, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(gpu_total_onac,
				       GPUTotalProcessingPowerTargetOnAcOffsetFromBaseline, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(gpu_to_cpu_dynamic_boost, GPUToCPUDynamicBoost, true),
	LEGION_WMI_OTHER_GROUP_FEATURE(igpu_mode, IGPUMode, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(instant_boot_ac, InstantBootAc, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(instant_boot_usb_power_delivery,
				       InstantBootUsbPowerDelivery, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(fan_full_speed, FanFullSpeed, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(over_drive, OverDrive, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(god_mode_fnq_switchable, GodModeFnQSwitchable, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(nvidia_gpu_dd_switching,
				       NvidiaGPUDynamicDisplaySwitching, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(flip_to_start, FlipToStart, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(skin_temperature_tracking,
				       AMDSkinTemperatureTracking, false),
	LEGION_WMI_OTHER_GROUP_FEATURE(smart_shiftmode, AMDSmartShiftMode, false),
};

/* Power modes of the capability data 01 */
static const u8 legion_wmi_other_group_modes[] = {
	LEGION_WMI_GZ_THERMAL_MODE_QUIET,
	LEGION_WMI_GZ_THERMAL_MODE_BALANCED,
	LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE,
	LEGION_WMI_GZ_THERMAL_MODE_CUSTOM,
	LEGION_WMI_GZ_THERMAL_MODE_EXTREME,
};

/*
 * The firmware has the feature: its capability data 00, or the capability data 01
 * of a power mode, has the valid bit (e.g. 16IAX10H: no peak power limit and
 * APU sPPT, capability 0 of the AMD features and overdrive)
 */
static bool
legion_wmi_other_feature_present(const struct legion_wmi_other_priv *priv,
				 const struct legion_wmi_other_group_feature *feature)
{
	const u32 id = feature->id & ~LEGION_WMI_MODE_ID_MASK;

	if (!feature->cd01) {
		struct legion_wmi_capdata00 capdata;

		return !legion_wmi_cd00_get_data(priv->cd00_list, id, &capdata) &&
		       (capdata.supported & LEGION_WMI_SUPP_VALID);
	}

	for (size_t i = 0; i < ARRAY_SIZE(legion_wmi_other_group_modes); i++) {
		const u32 mode_id =
			id | FIELD_PREP(LEGION_WMI_MODE_ID_MASK,
					legion_wmi_other_group_modes[i]);
		struct legion_wmi_capdata01 capdata;

		if (!legion_wmi_cd01_get_data(priv->cd01_list, mode_id, &capdata) &&
		    (capdata.supported & LEGION_WMI_SUPP_VALID))
			return true;
	}

	return false;
}

/*
 * A group of a feature the firmware does not have is not created (the same as
 * the limits of the CPU / GPU method and the attributes of the game zone); the
 * capability data do not change, decided once
 */
static umode_t legion_wmi_other_group_visible(struct kobject *kobj,
					      struct attribute *attr, int n)
{
	const struct legion_wmi_other_priv *priv =
		dev_get_drvdata(kobj_to_dev(kobj->parent));

	if (n)
		return attr->mode;

	for (size_t i = 0; i < ARRAY_SIZE(legion_wmi_other_group_features); i++) {
		const struct legion_wmi_other_group_feature *feature =
			&legion_wmi_other_group_features[i];

		if (feature->attrs[0] != attr)
			continue;

		if (legion_wmi_other_feature_present(priv, feature))
			return attr->mode;

		dev_dbg(&priv->wdev->dev, "%s is not supported\n",
			feature->name);
		return SYSFS_GROUP_INVISIBLE;
	}

	return attr->mode;
}

static const struct attribute_group legion_sysfs_other_group[] = {
	/*
	 * CPU CAP01
	 */
	{ .name = "cpu_stp_limit",
	  .attrs = legion_sysfs_cpu_stp_limit_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "cpu_ltp_limit",
	  .attrs = legion_sysfs_cpu_ltp_limit_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "cpu_pp_limit",
	  .attrs = legion_sysfs_cpu_pp_limit_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "cpu_clp_limit",
	  .attrs = legion_sysfs_cpu_clp_limit_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "cpu_tmp_limit",
	  .attrs = legion_sysfs_cpu_tmp_limit_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "apus_pptp_limit",
	  .attrs = legion_sysfs_apus_pptp_limit_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "cpu_pl1_tau", .attrs = legion_sysfs_cpu_pl1_tau_attributes,
	  .is_visible = legion_wmi_other_group_visible },

	/*
	 * GPU CAP01
	 */
	{ .name = "gpu_power_boost",
	  .attrs = legion_sysfs_gpu_power_boost_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "gpu_configurable_tgp",
	  .attrs = legion_sysfs_gpu_configurable_tgp_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "gpu_temperature_limit",
	  .attrs = legion_sysfs_gpu_temperature_limit_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "gpu_total_onac",
	  .attrs = legion_sysfs_gpu_total_onac_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "gpu_to_cpu_dynamic_boost",
	  .attrs = legion_sysfs_gpu_to_cpu_dynamic_boost_attributes,
	  .is_visible = legion_wmi_other_group_visible },

	/*
	 * GPU CAP00
	 */
	{ .name = "igpu_mode", .attrs = legion_sysfs_igpu_mode_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "instant_boot_ac",
	  .attrs = legion_sysfs_instant_boot_ac_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "instant_boot_usb_power_delivery",
	  .attrs = legion_sysfs_instant_boot_usb_power_delivery_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "fan_full_speed",
	  .attrs = legion_sysfs_fan_full_speed_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "over_drive", .attrs = legion_sysfs_over_drive_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "god_mode_fnq_switchable",
	  .attrs = legion_sysfs_god_mode_fnq_switchable_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "nvidia_gpu_dd_switching",
	  .attrs = legion_sysfs_nvidia_gpu_dd_switching_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "flip_to_start",
	  .attrs = legion_sysfs_flip_to_start_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "skin_temperature_tracking",
	  .attrs = legion_sysfs_skin_temperature_tracking_attributes,
	  .is_visible = legion_wmi_other_group_visible },
	{ .name = "smart_shiftmode",
	  .attrs = legion_sysfs_smart_shiftmode_attributes,
	  .is_visible = legion_wmi_other_group_visible },

	{ .name = "others", .attrs = legion_sysfs_others_attributes },
	{}
};

/**
 * legion_wmi_other_fw_attr_add() - Register all firmware attributes of the other mode
 * @priv: The Other Mode driver data.
 *
 * Return: Either 0, or an error code.
 */
static int legion_wmi_other_fw_attr_add(struct legion_wmi_other_priv *priv)
{
	int err = 0;
	unsigned int i = 0;

	priv->ida_id = ida_alloc(&legion_wmi_other_ida, GFP_KERNEL);
	if (priv->ida_id < 0)
		return priv->ida_id;

	priv->fw_attr_dev = device_create(&legion_firmware_attributes_class,
					  NULL, MKDEV(0, 0), priv, "%s-%u",
					  LEGION_WMI_OTHER_FW_ATTR_BASE_PATH,
					  priv->ida_id);
	if (IS_ERR(priv->fw_attr_dev)) {
		err = (int)PTR_ERR(priv->fw_attr_dev);
		goto err_free_ida;
	}

	priv->fw_attr_kset = kset_create_and_add(LEGION_FW_ATTR_KSET_NAME, NULL,
						 &priv->fw_attr_dev->kobj);
	if (!priv->fw_attr_kset) {
		err = -ENOMEM;
		goto err_unregister_fw_attr_dev;
	}

	for (i = 0; i < ARRAY_SIZE(legion_sysfs_other_group) - 1; i++) {
		err = sysfs_create_group(&priv->fw_attr_kset->kobj,
					 &legion_sysfs_other_group[i]);
		if (err)
			goto err_remove_groups;
	}

	return 0;
err_remove_groups:
	while (i--)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,
				   &legion_sysfs_other_group[i]);
	kset_unregister(priv->fw_attr_kset);
err_unregister_fw_attr_dev:
	device_unregister(priv->fw_attr_dev);
err_free_ida:
	ida_free(&legion_wmi_other_ida, priv->ida_id);
	return err;
}

/**
 * legion_wmi_other_fw_attr_remove() - Unregister all firmware attributes of the other mode
 * @priv: the lenovo-wmi-other driver data.
 */
static void legion_wmi_other_fw_attr_remove(struct legion_wmi_other_priv *priv)
{
	/* Remove sysfs group first */
	for (unsigned int i = 0; i < ARRAY_SIZE(legion_sysfs_other_group) - 1;
	     i++)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,
				   &legion_sysfs_other_group[i]);

	/* Unregister kset before device since kset is child of device's kobj */
	kset_unregister(priv->fw_attr_kset);

	/* Device unregister will clean up remaining kobject hierarchy */
	device_unregister(priv->fw_attr_dev);

	ida_free(&legion_wmi_other_ida, priv->ida_id);
}

int legion_wmi_other_sysfs_init(struct legion_wmi_other_priv *priv)
{
	return legion_wmi_other_fw_attr_add(priv);
}

void legion_wmi_other_sysfs_exit(struct legion_wmi_other_priv *priv)
{
	legion_wmi_other_fw_attr_remove(priv);
}
