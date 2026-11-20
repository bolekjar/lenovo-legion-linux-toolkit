// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * CPU and GPU power limits of the older Legion firmware (SmartFan version 4 / 5 or LegionZone version 1 / 2, e.g.
 * Legion 5 15IAH7H with BIOS J2CN): WMI classes LENOVO_CPU_METHOD and LENOVO_GPU_METHOD instead of the other mode and
 * the capability data of the newer firmware. The newer firmware keeps the GUID of LENOVO_CPU_METHOD with other
 * methods only (e.g. Legion Pro 7 16IAX10H: CPU_Set_OC_Data), so the limits are registered only on the older platform
 * (SmartFan version from the game zone, LegionZone version from the other mode, the same rule as
 * GetSupportsGodModeV1Internal of Lenovo Legion Toolkit).
 *
 * The methods and the order of their output values are taken from the binary MOF of the firmware (15IAH7H): a get
 * method returns the values in a buffer (an integer for a short result), e.g. CPU_Get_LongTerm_PowerLimit: uint32
 * Current, step, Min, Max. The limits of a value come from the firmware: a write out of them, out of the step or not
 * in the custom power mode is refused, the written value is read back.
 *
 * The firmware attributes of the limits are in legion-wmi-cpu-gpu-sysfs.c.
 */
#include "legion-common.h"
#include "legion-wmi-gamezone.h"
#include "legion-wmi-helpers.h"
#include "legion-wmi-cpu-gpu-sysfs.h"
#include "legion-wmi-cpu-gpu.h"
#include "legion-wmi-other.h"
#include "legion-wmi-platform.h"

#include <linux/device.h>
#include <linux/idr.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/unaligned.h>
#include <linux/wmi.h>

#define LEGION_WMI_CPU_GPU_CPU_GUID "14AFD777-106F-4C9B-B334-D388DC7809BE"
#define LEGION_WMI_CPU_GPU_GPU_GUID "DA7547F1-824D-405F-BE79-D9903E29CED7"

/* Largest result of a get method: 5 uint32 values */
#define LEGION_WMI_CPU_GPU_RESULT_SIZE 32

/* Layouts of the get methods: uint32 Current, step, Min, Max / uint32 Current, Default, step, Min, Max */
#define CPU_GPU_LIMIT_4(_name, _label, _get, _set, _def, _event)                       \
	{ .name = _name, .display_name = _label, .get_id = _get, .set_id = _set,        \
	  .item_size = 4, .current_index = 0, .default_index = LEGION_WMI_CPU_GPU_NONE,   \
	  .step_index = 1, .min_index = 2, .max_index = 3,                              \
	  .default_method_index = _def, .event = _event }

#define CPU_GPU_LIMIT_5(_name, _label, _get, _set, _size, _event)                      \
	{ .name = _name, .display_name = _label, .get_id = _get, .set_id = _set,        \
	  .item_size = _size, .current_index = 0, .default_index = 1,                   \
	  .step_index = 2, .min_index = 3, .max_index = 4,                              \
	  .default_method_index = LEGION_WMI_CPU_GPU_NONE, .event = _event }

static const struct legion_wmi_cpu_gpu_limit legion_wmi_cpu_gpu_cpu_limits[] = {
	/* CPU_Get_LongTerm_PowerLimit (5) / CPU_Set_LongTerm_PowerLimit (6), default: CPU_Get_Default_PowerLimit [0] */
	CPU_GPU_LIMIT_4("cpu_ltp_limit", "Set the CPU long term power limit", 5, 6, 0,
		       LEGION_WMI_CPU_LTP),
	/* CPU_Get_ShortTerm_PowerLimit (3) / CPU_Set_ShortTerm_PowerLimit (4), default: CPU_Get_Default_PowerLimit [1] */
	CPU_GPU_LIMIT_4("cpu_stp_limit", "Set the CPU short term power limit", 3, 4, 1,
		       LEGION_WMI_CPU_STP),
	/* CPU_Get_Peak_PowerLimit (8) / CPU_Set_Peak_PowerLimit (9) */
	CPU_GPU_LIMIT_5("cpu_pp_limit", "Set the CPU peak power limit", 8, 9, 4,
		       LEGION_WMI_CPU_PP),
	/* CPU_Get_Cross_Loading_PowerLimit (16) / CPU_Set_Cross_Loading_PowerLimit (17) */
	CPU_GPU_LIMIT_5("cpu_clp_limit", "Set the CPU cross loading power limit", 16, 17, 4,
		       LEGION_WMI_CPU_CLP),
	/* CPU_Get_Temperature_Control (10) / CPU_Set_Temperature_Control (11), uint8 */
	CPU_GPU_LIMIT_5("cpu_tmp_limit", "Set the CPU temperature limit", 10, 11, 1,
		       LEGION_WMI_CPU_TMP),
	/* Get_APU_sPPT_PowerLimit (12) / Set_APU_sPPT_PowerLimit (13) */
	CPU_GPU_LIMIT_5("apus_pptp_limit", "Set the APU sPPT power limit", 12, 13, 4,
		       LEGION_WMI_CPU_APU_SPPT),
};

static const struct legion_wmi_cpu_gpu_limit legion_wmi_cpu_gpu_gpu_limits[] = {
	/* GPU_Get_PPAB_PowerLimit (3) / GPU_Set_PPAB_PowerLimit (4), default: GPU_Get_Default_PPAB_cTGP_PowerLimit [0] */
	CPU_GPU_LIMIT_4("gpu_power_boost", "Set the GPU power boost (Dynamic Boost)", 3, 4, 0,
		       LEGION_WMI_GPU_PB),
	/* GPU_Get_cTGP_PowerLimit (5) / GPU_Set_cTGP_PowerLimit (6), default: GPU_Get_Default_PPAB_cTGP_PowerLimit [1] */
	CPU_GPU_LIMIT_4("gpu_configurable_tgp", "Set the GPU configurable TGP", 5, 6, 1,
		       LEGION_WMI_GPU_TGP),
	/* GPU_Get_Temperature_Limit (8) / GPU_Set_Temperature_Limit (9), uint8 */
	CPU_GPU_LIMIT_5("gpu_temperature_limit", "Set the GPU temperature limit", 8, 9, 1,
		       LEGION_WMI_GPU_TMP),
};

/* Own numbers of the devices of each class (legion-wmi-cpu-0, legion-wmi-gpu-0) */
static DEFINE_IDA(legion_wmi_cpu_gpu_cpu_ida);
static DEFINE_IDA(legion_wmi_cpu_gpu_gpu_ida);

static const struct legion_wmi_cpu_gpu_class legion_wmi_cpu_gpu_cpu = {
	.base_path = "legion-wmi-cpu",
	.limits = legion_wmi_cpu_gpu_cpu_limits,
	.count = ARRAY_SIZE(legion_wmi_cpu_gpu_cpu_limits),
	.default_method = 7, /* CPU_Get_Default_PowerLimit: uint32 DefaultLongTerm, DefaultShortTerm */
	.ida = &legion_wmi_cpu_gpu_cpu_ida,
};

static const struct legion_wmi_cpu_gpu_class legion_wmi_cpu_gpu_gpu = {
	.base_path = "legion-wmi-gpu",
	.limits = legion_wmi_cpu_gpu_gpu_limits,
	.count = ARRAY_SIZE(legion_wmi_cpu_gpu_gpu_limits),
	.default_method = 7, /* GPU_Get_Default_PPAB_cTGP_PowerLimit: uint32 DefaultPPAB, DefaultcTGP */
	.ida = &legion_wmi_cpu_gpu_gpu_ida,
};

/*
 * Result of a get method as bytes: a buffer as it is, an integer as its little endian bytes (a short result)
 */
static int legion_wmi_cpu_gpu_call(struct legion_wmi_cpu_gpu_priv *priv,
				  u32 method_id, u8 *result, size_t *length)
{
	int err;

	lockdep_assert_held(&priv->lock);

	err = legion_wmi_dev_evaluate_bytes(priv->wdev, 0, method_id, NULL, 0,
					    result,
					    LEGION_WMI_CPU_GPU_RESULT_SIZE,
					    length);
	if (err)
		return err;

	dev_dbg(&priv->wdev->dev, "method %u: %*ph\n", method_id, (int)*length,
		result);

	return 0;
}

/*
 * Value of a result, false when it is not in it
 */
static bool legion_wmi_cpu_gpu_value(const u8 *result, size_t length,
				    u8 item_size, s8 index, u32 *value)
{
	const size_t offset = (size_t)index * item_size;

	if (index < 0 || offset + item_size > length)
		return false;

	*value = item_size == 1 ? result[offset] :
				  get_unaligned_le32(result + offset);
	return true;
}

/**
 * legion_wmi_cpu_gpu_read() - Read a value of a limit from the firmware
 * @priv: The driver data.
 * @limit: The limit.
 * @field: CPU_GPU_FIELD_CURRENT, _DEFAULT, _MIN, _MAX or _STEP.
 * @value: The value.
 *
 * Context: Caller holds priv->lock.
 * Return: 0, -ENODATA when the firmware does not provide the value, or an error of the WMI method.
 */
int legion_wmi_cpu_gpu_read(struct legion_wmi_cpu_gpu_priv *priv,
			   const struct legion_wmi_cpu_gpu_limit *limit,
			   enum legion_wmi_cpu_gpu_field field, u32 *value)
{
	u8 result[LEGION_WMI_CPU_GPU_RESULT_SIZE] = { 0 };
	size_t length = 0;
	s8 index;
	int err;

	/* Default value of the class (e.g. CPU_Get_Default_PowerLimit) */
	if (field == CPU_GPU_FIELD_DEFAULT &&
	    limit->default_index == LEGION_WMI_CPU_GPU_NONE) {
		if (limit->default_method_index == LEGION_WMI_CPU_GPU_NONE)
			return -ENODATA;

		err = legion_wmi_cpu_gpu_call(priv, priv->wmi_class->default_method,
					     result, &length);
		if (err)
			return err;

		return legion_wmi_cpu_gpu_value(result, length, sizeof(u32),
					       limit->default_method_index,
					       value) ?
			       0 :
			       -ENODATA;
	}

	switch (field) {
	case CPU_GPU_FIELD_CURRENT:
		index = limit->current_index;
		break;
	case CPU_GPU_FIELD_DEFAULT:
		index = limit->default_index;
		break;
	case CPU_GPU_FIELD_MIN:
		index = limit->min_index;
		break;
	case CPU_GPU_FIELD_MAX:
		index = limit->max_index;
		break;
	case CPU_GPU_FIELD_STEP:
		index = limit->step_index;
		break;
	default:
		return -EINVAL;
	}

	err = legion_wmi_cpu_gpu_call(priv, limit->get_id, result, &length);
	if (err)
		return err;

	return legion_wmi_cpu_gpu_value(result, length, limit->item_size, index,
				       value) ?
		       0 :
		       -ENODATA;
}

/**
 * legion_wmi_cpu_gpu_write() - Write a limit and verify it
 * @priv: The driver data.
 * @limit: The limit.
 * @value: The value.
 *
 * Only in the custom power mode (the same as the custom mode of Lenovo Legion Toolkit) and within the limits of the
 * firmware (minimum, maximum, step).
 *
 * Context: Takes priv->lock.
 * Return: 0, -EBUSY not in the custom mode, -EINVAL out of the limits, -EIO when the firmware did not apply it, or an
 * error of the WMI method.
 */
int legion_wmi_cpu_gpu_write(struct legion_wmi_cpu_gpu_priv *priv,
			    const struct legion_wmi_cpu_gpu_limit *limit,
			    u32 value)
{
	u8 argument[sizeof(u32)];
	enum thermal_mode mode;
	u32 min, max, step, current_value;
	int err;

	err = legion_wmi_other_notifier_call(&mode,
					     LEGION_WMI_GZ_GET_THERMAL_MODE);
	if (err)
		return err;

	if (mode != LEGION_WMI_GZ_THERMAL_MODE_CUSTOM)
		return -EBUSY;

	guard(mutex)(&priv->lock);

	err = legion_wmi_cpu_gpu_read(priv, limit, CPU_GPU_FIELD_MIN, &min);
	if (!err)
		err = legion_wmi_cpu_gpu_read(priv, limit, CPU_GPU_FIELD_MAX, &max);
	if (!err)
		err = legion_wmi_cpu_gpu_read(priv, limit, CPU_GPU_FIELD_STEP,
					     &step);
	if (err)
		return err;

	if (value < min || value > max || (step && (value - min) % step))
		return -EINVAL;

	if (limit->item_size == 1)
		argument[0] = value;
	else
		put_unaligned_le32(value, argument);

	err = legion_wmi_dev_evaluate_int(priv->wdev, 0, limit->set_id,
					  argument, limit->item_size, NULL);
	if (err)
		return err;

	err = legion_wmi_cpu_gpu_read(priv, limit, CPU_GPU_FIELD_CURRENT,
				     &current_value);
	if (err)
		return err;

	if (current_value != value) {
		dev_warn(&priv->wdev->dev,
			 "%s: write of %u was not applied by the firmware (reads %u)\n",
			 limit->name, value, current_value);
		return -EIO;
	}

	return 0;
}

/**
 * legion_wmi_cpu_gpu_supported() - Whether the firmware supports a limit
 * @priv: The driver data.
 * @limit: The limit.
 *
 * Context: Takes priv->lock.
 * Return: true when the firmware returns the current value, the minimum and the maximum.
 */
bool legion_wmi_cpu_gpu_supported(struct legion_wmi_cpu_gpu_priv *priv,
				 const struct legion_wmi_cpu_gpu_limit *limit)
{
	u32 value;

	guard(mutex)(&priv->lock);

	return !legion_wmi_cpu_gpu_read(priv, limit, CPU_GPU_FIELD_CURRENT,
				       &value) &&
	       !legion_wmi_cpu_gpu_read(priv, limit, CPU_GPU_FIELD_MIN, &value) &&
	       !legion_wmi_cpu_gpu_read(priv, limit, CPU_GPU_FIELD_MAX, &value);
}

static int legion_wmi_cpu_gpu_probe(struct wmi_device *wdev, const void *context)
{
	struct legion_wmi_cpu_gpu_priv *priv;
	int err = 0;

	if (!legion_wmi_platform_legacy(&wdev->dev, &err))
		return err ? err : -ENODEV;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;
	priv->wmi_class = context;

	err = devm_mutex_init(&wdev->dev, &priv->lock);
	if (err)
		return err;

	dev_set_drvdata(&wdev->dev, priv);

	/* Not with an old BIOS (Lenovo Legion Toolkit: no God mode there), -ENODEV: no limit supported */
	if (!legion_wmi_platform_bios_supported(&wdev->dev))
		return -ENODEV;

	return legion_wmi_cpu_gpu_sysfs_init(priv);
}

static void legion_wmi_cpu_gpu_remove(struct wmi_device *wdev)
{
	legion_wmi_cpu_gpu_sysfs_exit(dev_get_drvdata(&wdev->dev));
}

static const struct wmi_device_id legion_wmi_cpu_gpu_cpu_id_table[] = {
	{ LEGION_WMI_CPU_GPU_CPU_GUID, &legion_wmi_cpu_gpu_cpu },
	{}
};

static const struct wmi_device_id legion_wmi_cpu_gpu_gpu_id_table[] = {
	{ LEGION_WMI_CPU_GPU_GPU_GUID, &legion_wmi_cpu_gpu_gpu },
	{}
};

static struct wmi_driver legion_wmi_cpu_gpu_cpu_driver = {
	.driver = {
		.name = "legion_wmi_cpu",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_cpu_gpu_cpu_id_table,
	.probe = legion_wmi_cpu_gpu_probe,
	.remove = legion_wmi_cpu_gpu_remove,
	.no_singleton = true,
};

static struct wmi_driver legion_wmi_cpu_gpu_gpu_driver = {
	.driver = {
		.name = "legion_wmi_gpu",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_cpu_gpu_gpu_id_table,
	.probe = legion_wmi_cpu_gpu_probe,
	.remove = legion_wmi_cpu_gpu_remove,
	.no_singleton = true,
};

int legion_wmi_cpu_gpu_driver_init(void)
{
	int err;

	err = wmi_driver_register(&legion_wmi_cpu_gpu_cpu_driver);
	if (err)
		return err;

	err = wmi_driver_register(&legion_wmi_cpu_gpu_gpu_driver);
	if (err)
		wmi_driver_unregister(&legion_wmi_cpu_gpu_cpu_driver);

	return err;
}

void legion_wmi_cpu_gpu_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_cpu_gpu_gpu_driver);
	wmi_driver_unregister(&legion_wmi_cpu_gpu_cpu_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_cpu_gpu_cpu_id_table);
MODULE_DEVICE_TABLE(wmi, legion_wmi_cpu_gpu_gpu_id_table);
