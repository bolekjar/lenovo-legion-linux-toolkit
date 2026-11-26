// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Default values of the power modes of the older Legion firmware (e.g. Legion 5 15IAH7H): WMI data block
 * LENOVO_DEFAULT_VALUE_IN_DIFFERENT_MODE_DATA, one instance for each power mode (quiet, balanced, performance) and
 * fan / sensor, the default power limits are the same in the instances of a mode. The values come from tables of
 * the firmware chosen by the CPU and the GPU of the notebook (ASL WQA5, SFTW), Lenovo Legion Toolkit uses them as
 * the defaults of the power modes of God mode V1 (GodModeControllerV1.GetDefaultsAsync).
 *
 * The CPU / GPU method driver shows them in default_value of its limits (legion_wmi_mode_default_get()).
 */
#include "legion-wmi-mode-defaults.h"
#include "legion-wmi-helpers.h"

#include <linux/acpi.h>
#include <linux/build_bug.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/wmi.h>

#define LEGION_WMI_MODE_DEFAULTS_GUID "ECAF3CE6-DCB2-473D-94C3-7BF4D96E0D9A"

#define LEGION_WMI_MODE_DEFAULTS_FAN_TABLE_SIZE 10

/*
 * Instance of the data block (binary MOF and ASL LDVD, packed, little endian)
 */
struct legion_wmi_mode_defaults_data {
	__le32 mode; /* 1 quiet, 2 balanced, 3 performance */
	u8 fan_id;
	u8 sensor_id;
	u8 max_fan_speed_number;
	u8 cpu_clp; /* DefaultCpuCrossLoading */
	__le32 fan_table_size;
	__le16 fan_table[LEGION_WMI_MODE_DEFAULTS_FAN_TABLE_SIZE]; /* rpm */
	__le32 cpu_ltp;
	__le32 cpu_stp;
	__le32 cpu_pp; /* not set by the firmware */
	__le32 apu_sppt; /* not set by the firmware */
	__le32 gpu_ppab;
	__le32 gpu_ctgp;
	u8 cpu_tmp; /* DefaultTemperatueControl */
	u8 gpu_tmp; /* DefaultTemperatueLimit */
	__le16 reserved;
} __packed;
static_assert(sizeof(struct legion_wmi_mode_defaults_data) == 0x3c);

/**
 * struct legion_wmi_mode_defaults_priv - The driver data
 * @wdev: The WMI device.
 * @data: Instances of the data block (read once, the values of the firmware do not change).
 * @count: Number of the read instances.
 * @nb: Answers legion_wmi_mode_default_get().
 */
struct legion_wmi_mode_defaults_priv {
	struct wmi_device *wdev;
	struct legion_wmi_mode_defaults_data *data;
	unsigned int count;
	struct notifier_block nb;
};

/*
 * Request of legion_wmi_mode_default_get()
 */
struct legion_wmi_mode_defaults_request {
	u32 mode;
	enum legion_wmi_mode_default field;
	u32 value;
};

static BLOCKING_NOTIFIER_HEAD(legion_wmi_mode_defaults_chain);

static bool
legion_wmi_mode_defaults_value(const struct legion_wmi_mode_defaults_data *data,
			       enum legion_wmi_mode_default field, u32 *value)
{
	switch (field) {
	case LEGION_WMI_MODE_DEFAULT_CPU_LTP:
		*value = le32_to_cpu(data->cpu_ltp);
		return true;
	case LEGION_WMI_MODE_DEFAULT_CPU_STP:
		*value = le32_to_cpu(data->cpu_stp);
		return true;
	case LEGION_WMI_MODE_DEFAULT_CPU_CLP:
		*value = data->cpu_clp;
		return true;
	case LEGION_WMI_MODE_DEFAULT_CPU_TMP:
		*value = data->cpu_tmp;
		return true;
	case LEGION_WMI_MODE_DEFAULT_GPU_PPAB:
		*value = le32_to_cpu(data->gpu_ppab);
		return true;
	case LEGION_WMI_MODE_DEFAULT_GPU_CTGP:
		*value = le32_to_cpu(data->gpu_ctgp);
		return true;
	case LEGION_WMI_MODE_DEFAULT_GPU_TMP:
		*value = data->gpu_tmp;
		return true;
	default:
		return false;
	}
}

static int legion_wmi_mode_defaults_call(struct notifier_block *nb,
					 unsigned long action, void *data)
{
	const struct legion_wmi_mode_defaults_priv *priv =
		container_of(nb, struct legion_wmi_mode_defaults_priv, nb);
	struct legion_wmi_mode_defaults_request *request = data;

	/* The power limits are the same in all instances of a mode (fan / sensor), the first one is used */
	for (unsigned int i = 0; i < priv->count; i++) {
		if (le32_to_cpu(priv->data[i].mode) != request->mode)
			continue;

		return legion_wmi_mode_defaults_value(&priv->data[i],
						      request->field,
						      &request->value) ?
			       NOTIFY_STOP :
			       NOTIFY_DONE;
	}

	return NOTIFY_DONE;
}

/**
 * legion_wmi_mode_default_get() - Default value of a power mode of the older firmware
 * @mode: The power mode (1 quiet, 2 balanced, 3 performance).
 * @field: The value.
 * @value: The default value.
 *
 * Return: 0, or -ENODATA when the firmware has no such data (e.g. the newer firmware).
 */
int legion_wmi_mode_default_get(u32 mode, enum legion_wmi_mode_default field,
				u32 *value)
{
	struct legion_wmi_mode_defaults_request request = {
		.mode = mode,
		.field = field,
	};

	if (blocking_notifier_call_chain(&legion_wmi_mode_defaults_chain, 0,
					 &request) != NOTIFY_STOP)
		return -ENODATA;

	*value = request.value;
	return 0;
}

static void legion_wmi_mode_defaults_unregister(void *data)
{
	blocking_notifier_chain_unregister(&legion_wmi_mode_defaults_chain,
					   data);
}

/*
 * Read the instances of the data block, an instance of another size or fan table size is skipped
 */
static int legion_wmi_mode_defaults_read(struct legion_wmi_mode_defaults_priv *priv)
{
	const int count = wmidev_instance_count(priv->wdev);

	if (count <= 0)
		return -ENODEV;

	priv->data = devm_kcalloc(&priv->wdev->dev, count, sizeof(*priv->data),
				  GFP_KERNEL);
	if (!priv->data)
		return -ENOMEM;

	for (int idx = 0; idx < count; idx++) {
		union acpi_object *obj __free(kfree) =
			wmidev_block_query(priv->wdev, idx);
		struct legion_wmi_mode_defaults_data *data =
			&priv->data[priv->count];

		/* Arrays in the class (binary MOF): a buffer only */
		if (legion_wmi_block_read(obj, data, sizeof(*data), NULL, 0))
			continue;

		if (le32_to_cpu(data->fan_table_size) !=
		    LEGION_WMI_MODE_DEFAULTS_FAN_TABLE_SIZE)
			continue;

		dev_dbg(&priv->wdev->dev,
			"mode %u, fan %u, sensor %u: LTP %u, STP %u, CLP %u, TMP %u, PPAB %u, cTGP %u, GPU TMP %u\n",
			le32_to_cpu(data->mode), data->fan_id, data->sensor_id,
			le32_to_cpu(data->cpu_ltp), le32_to_cpu(data->cpu_stp),
			data->cpu_clp, data->cpu_tmp,
			le32_to_cpu(data->gpu_ppab),
			le32_to_cpu(data->gpu_ctgp), data->gpu_tmp);

		priv->count++;
	}

	return priv->count ? 0 : -ENODEV;
}

static int legion_wmi_mode_defaults_probe(struct wmi_device *wdev,
					  const void *context)
{
	struct legion_wmi_mode_defaults_priv *priv;
	int err;

	priv = devm_kzalloc(&wdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->wdev = wdev;

	err = legion_wmi_mode_defaults_read(priv);
	if (err)
		return err;

	dev_set_drvdata(&wdev->dev, priv);

	priv->nb.notifier_call = legion_wmi_mode_defaults_call;
	err = blocking_notifier_chain_register(&legion_wmi_mode_defaults_chain,
					       &priv->nb);
	if (err)
		return err;

	return devm_add_action_or_reset(&wdev->dev,
					legion_wmi_mode_defaults_unregister,
					&priv->nb);
}

static const struct wmi_device_id legion_wmi_mode_defaults_id_table[] = {
	{ LEGION_WMI_MODE_DEFAULTS_GUID, NULL },
	{}
};

static struct wmi_driver legion_wmi_mode_defaults_driver = {
	.driver = {
		.name = "legion_wmi_mode_defaults",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_mode_defaults_id_table,
	.probe = legion_wmi_mode_defaults_probe,
	.no_singleton = true,
};

int legion_wmi_mode_defaults_driver_init(void)
{
	return wmi_driver_register(&legion_wmi_mode_defaults_driver);
}

void legion_wmi_mode_defaults_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_mode_defaults_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_mode_defaults_id_table);
