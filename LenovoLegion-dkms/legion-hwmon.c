// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-hwmon.h"
#include "legion-wmi-other.h"
#include "legion-common.h"
#include "legion-wmi-fm.h"
#include "legion-wmi-gamezone.h"
#include "legion-wmi-capdata00.h"

#include <asm/errno.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>
#include <linux/sysfs.h>
#include <linux/notifier.h>
#include <linux/component.h>

static BLOCKING_NOTIFIER_HEAD(legion_hwmon_chain_head);
static BLOCKING_NOTIFIER_HEAD(legion_hwmon_fm_chain_head);

static int legion_hwmon_other_register_notifier_dev(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&legion_hwmon_chain_head, nb);
}

static int legion_hwmon_other_unregister_notifier_dev(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&legion_hwmon_chain_head, nb);
}

static void legion_hwmon_wmi_other_unregister_notifier(void *data)
{
	struct notifier_block *const nb = data;

	legion_hwmon_other_unregister_notifier_dev(nb);
}

int legion_hwmon_other_register_notifier(struct device *dev,
					 struct notifier_block *nb)
{
	const int ret = legion_hwmon_other_register_notifier_dev(nb);

	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(
		dev, legion_hwmon_wmi_other_unregister_notifier, nb);
}

static ssize_t legion_hwmon_other_notifier_call(void *data,
						enum CapabilityID capability_id)
{
	struct hwmon_capability req_data = { capability_id, data };
	const int ret = blocking_notifier_call_chain(
		&legion_hwmon_chain_head, LEGION_WMI_FM_GET_CAPABILITY_VALUE,
		&req_data);

	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

static int
legion_hwmon_other_data00_notifier_call(void *data,
					enum CapabilityID capability_id)
{
	struct hwmon_capability req_data = { capability_id, data };
	const int ret = blocking_notifier_call_chain(
		&legion_hwmon_chain_head, LEGION_WMI_FM_GET_CAPABILITY_DATA,
		&req_data);

	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

static int legion_hwmon_fm_register_notifier_dev(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&legion_hwmon_fm_chain_head,
						nb);
}

static int legion_hwmon_fm_unregister_notifier_dev(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&legion_hwmon_fm_chain_head,
						  nb);
}

static void legion_hwmon_fm_unregister_notifier(void *data)
{
	struct notifier_block *const nb = data;

	legion_hwmon_fm_unregister_notifier_dev(nb);
}

int legion_hwmon_fm_register_notifier(struct device *dev,
				      struct notifier_block *nb)
{
	const int ret = legion_hwmon_fm_register_notifier_dev(nb);

	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(
		dev, legion_hwmon_fm_unregister_notifier, nb);
}

static int legion_hwmon_fm_notifier_call(void *data,
					 const enum fm_events_type events_type)
{
	const int ret = blocking_notifier_call_chain(
		&legion_hwmon_fm_chain_head, events_type, data);
	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

/*
 * Sensors of the Legion WMI other mode (capability data 00): channels 1..3 = CPU, GPU, SYS, a sensor the notebook
 * does not have (capability data 00 not supported) is hidden, the channel numbers stay (temp3 without temp2).
 * Fans: input from the other mode, max from the fan method, min is 0 (no data).
 */
#define LEGION_HWMON_CHANNELS 3

static const enum CapabilityID temperature_ids[LEGION_HWMON_CHANNELS] = {
	CpuCurrentTemperature, GpuCurrentTemperature, SysCurrentTemperature
};

static const enum CapabilityID fan_ids[LEGION_HWMON_CHANNELS] = {
	CpuCurrentFanSpeed, GpuCurrentFanSpeed, SysCurrentFanSpeed
};

static const enum fm_events_type fan_max_events[LEGION_HWMON_CHANNELS] = {
	LEGION_WMI_FM_GET_CPU_FAN_MAX_SPEED,
	LEGION_WMI_FM_GET_GPU_FAN_MAX_SPEED, LEGION_WMI_FM_GET_SYS_FAN_MAX_SPEED
};

static const char *const temperature_labels[LEGION_HWMON_CHANNELS] = {
	"CPU Temperature", "GPU Temperature", "SYS Temperature"
};

static const char *const fan_labels[LEGION_HWMON_CHANNELS] = { "CPU Fan",
							       "GPU Fan",
							       "SYS Fan" };

static umode_t legion_hwmon_is_visible(const void *drvdata,
				       enum hwmon_sensor_types type, u32 attr,
				       int channel)
{
	const struct legion_data *data = drvdata;

	if (channel < 0 || channel >= LEGION_HWMON_CHANNELS)
		return 0;

	switch (type) {
	case hwmon_temp:
		return data->hwmon_temperatures & BIT(channel) ? 0444 : 0;
	case hwmon_fan:
		return data->hwmon_fans & BIT(channel) ? 0444 : 0;
	default:
		return 0;
	}
}

static int legion_hwmon_read(struct device *dev, enum hwmon_sensor_types type,
			     u32 attr, int channel, long *val)
{
	u32 value = 0;

	switch (type) {
	case hwmon_temp:
		if (attr != hwmon_temp_input)
			return -EOPNOTSUPP;

		if (legion_hwmon_other_notifier_call(&value,
						     temperature_ids[channel]))
			return -EIO;

		*val = (long)value * 1000; /* millidegree Celsius */
		return 0;

	case hwmon_fan:
		switch (attr) {
		case hwmon_fan_input:
			if (legion_hwmon_other_notifier_call(&value,
							     fan_ids[channel]))
				return -EIO;
			break;
		case hwmon_fan_min:
			value = 0;
			break;
		case hwmon_fan_max:
			if (legion_hwmon_fm_notifier_call(
				    &value, fan_max_events[channel]))
				return -EIO;
			break;
		default:
			return -EOPNOTSUPP;
		}

		*val = value;
		return 0;

	default:
		return -EOPNOTSUPP;
	}
}

static int legion_hwmon_read_string(struct device *dev,
				    enum hwmon_sensor_types type, u32 attr,
				    int channel, const char **str)
{
	switch (type) {
	case hwmon_temp:
		*str = temperature_labels[channel];
		return 0;
	case hwmon_fan:
		*str = fan_labels[channel];
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static const struct hwmon_channel_info *const legion_hwmon_info[] = {
	HWMON_CHANNEL_INFO(temp, HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL,
			   HWMON_T_INPUT | HWMON_T_LABEL),
	HWMON_CHANNEL_INFO(
		fan, HWMON_F_INPUT | HWMON_F_LABEL | HWMON_F_MIN | HWMON_F_MAX,
		HWMON_F_INPUT | HWMON_F_LABEL | HWMON_F_MIN | HWMON_F_MAX,
		HWMON_F_INPUT | HWMON_F_LABEL | HWMON_F_MIN | HWMON_F_MAX),
	NULL
};

static const struct hwmon_ops legion_hwmon_ops = {
	.is_visible = legion_hwmon_is_visible,
	.read = legion_hwmon_read,
	.read_string = legion_hwmon_read_string,
};

static const struct hwmon_chip_info legion_hwmon_chip_info = {
	.ops = &legion_hwmon_ops,
	.info = legion_hwmon_info,
};

/*
 * Bit of the channel when the sensor is supported (capability data 00)
 */
static int legion_hwmon_supported(const enum CapabilityID ids[], u8 *mask)
{
	*mask = 0;

	for (int channel = 0; channel < LEGION_HWMON_CHANNELS; ++channel) {
		struct legion_wmi_capdata00 cap_data00 = { 0, 0, 0 };
		const int err = legion_hwmon_other_data00_notifier_call(
			&cap_data00, ids[channel]);

		if (err)
			return err;

		if (cap_data00.id == ids[channel] && cap_data00.supported)
			*mask |= BIT(channel);
	}

	return 0;
}

/*
 * Not device managed: the hwmon device is removed before the WMI drivers it reads (legion_remove)
 */
int legion_hwmon_init(struct device *parent)
{
	struct legion_data *data = dev_get_drvdata(parent);
	int err;

	if (!data)
		return -ENODEV;

	err = legion_hwmon_supported(temperature_ids,
				     &data->hwmon_temperatures);
	if (err)
		return err;

	err = legion_hwmon_supported(fan_ids, &data->hwmon_fans);
	if (err)
		return err;

	data->hwmon_dev = hwmon_device_register_with_info(
		parent, "legion", data, &legion_hwmon_chip_info, NULL);
	if (IS_ERR(data->hwmon_dev)) {
		err = (int)PTR_ERR(data->hwmon_dev);
		data->hwmon_dev = NULL;
		return err;
	}

	return 0;
}

void legion_hwmon_exit(const struct device *parent)
{
	struct legion_data *data = dev_get_drvdata(parent);

	if (!data || !data->hwmon_dev)
		return;

	hwmon_device_unregister(data->hwmon_dev);
	data->hwmon_dev = NULL;
}
