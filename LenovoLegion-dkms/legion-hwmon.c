// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-hwmon.h"
#include "legion-wmi-other.h"
#include "legion-wmi-fm.h"
#include "legion-wmi-gamezone.h"
#include "legion-wmi-capdata00.h"

#include <asm/errno.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>
#include <linux/sysfs.h>
#include <linux/notifier.h>
#include <linux/component.h>
#include <linux/err.h>
#include <linux/slab.h>

static BLOCKING_NOTIFIER_HEAD(legion_hwmon_fm_chain_head);

/*
 * Request to the source of the sensors of the hwmon (its notifier block is called directly, no chain: one source)
 */
static int legion_hwmon_source_call(struct notifier_block *source,
				    unsigned long action,
				    enum CapabilityID capability_id, void *data)
{
	struct hwmon_capability request = { capability_id, data };
	const int ret = source->notifier_call(source, action, &request);

	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

int legion_hwmon_fm_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&legion_hwmon_fm_chain_head,
						nb);
}

void legion_hwmon_fm_unregister_notifier(struct notifier_block *nb)
{
	blocking_notifier_chain_unregister(&legion_hwmon_fm_chain_head, nb);
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
 * does not have (capability data 00 without the GET bit) is hidden, the channel numbers stay (temp3 without temp2).
 * A temperature which reads 0 when the hwmon is registered is hidden too: the firmware may report a sensor it does
 * not have (e.g. SYS on an AMD Legion of generation 8 always returns 0).
 * Fans: input from the other mode, max from the fan method, min is 0 (no data).
 */
#define LEGION_HWMON_CHANNELS 3

struct legion_hwmon {
	struct device *dev;
	struct notifier_block *source; /* source of the sensors, owner of the hwmon */
	u8 temperatures; /* supported temperature channels (bits) */
	u8 fans; /* supported fan channels (bits) */
};

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
	const struct legion_hwmon *hwmon = drvdata;

	if (channel < 0 || channel >= LEGION_HWMON_CHANNELS)
		return 0;

	switch (type) {
	case hwmon_temp:
		return hwmon->temperatures & BIT(channel) ? 0444 : 0;
	case hwmon_fan:
		return hwmon->fans & BIT(channel) ? 0444 : 0;
	default:
		return 0;
	}
}

static int legion_hwmon_read(struct device *dev, enum hwmon_sensor_types type,
			     u32 attr, int channel, long *val)
{
	const struct legion_hwmon *hwmon = dev_get_drvdata(dev);
	u32 value = 0;

	switch (type) {
	case hwmon_temp:
		if (attr != hwmon_temp_input)
			return -EOPNOTSUPP;

		if (legion_hwmon_source_call(hwmon->source,
					     LEGION_WMI_FM_GET_CAPABILITY_VALUE,
					     temperature_ids[channel], &value))
			return -EIO;

		*val = (long)value * 1000; /* millidegree Celsius */
		return 0;

	case hwmon_fan:
		switch (attr) {
		case hwmon_fan_input:
			if (legion_hwmon_source_call(
				    hwmon->source,
				    LEGION_WMI_FM_GET_CAPABILITY_VALUE,
				    fan_ids[channel], &value))
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
 * Bit of the channel when the sensor can be read (capability data 00 with the GET bit)
 */
static int legion_hwmon_supported(struct notifier_block *source,
				  const enum CapabilityID ids[], u8 *mask)
{
	*mask = 0;

	for (int channel = 0; channel < LEGION_HWMON_CHANNELS; ++channel) {
		struct legion_wmi_capdata00 cap_data00 = { 0, 0, 0 };
		const int err = legion_hwmon_source_call(
			source, LEGION_WMI_FM_GET_CAPABILITY_DATA, ids[channel],
			&cap_data00);

		if (err)
			return err;

		if (cap_data00.id == ids[channel] &&
		    (cap_data00.supported & LEGION_WMI_SUPP_GET))
			*mask |= BIT(channel);
	}

	return 0;
}

/*
 * Clear the bits of the temperatures which can not be read or read 0 (a sensor the firmware does not have)
 */
static void legion_hwmon_temperatures_present(struct device *dev,
					      struct legion_hwmon *hwmon)
{
	for (int channel = 0; channel < LEGION_HWMON_CHANNELS; ++channel) {
		u32 value = 0;

		if (!(hwmon->temperatures & BIT(channel)))
			continue;

		if (legion_hwmon_source_call(hwmon->source,
					     LEGION_WMI_FM_GET_CAPABILITY_VALUE,
					     temperature_ids[channel], &value) ||
		    !value) {
			dev_dbg(dev, "%s not present, hidden\n",
				temperature_labels[channel]);
			hwmon->temperatures &= ~BIT(channel);
		}
	}
}

/**
 * legion_hwmon_register() - Create the hwmon device "legion" of a source of the sensors
 * @parent: Device of the source (owner of the hwmon), parent of the hwmon device.
 * @source: Notifier of the source (LEGION_WMI_FM_GET_CAPABILITY_VALUE, LEGION_WMI_FM_GET_CAPABILITY_DATA), called
 *          directly by the hwmon.
 *
 * The owner keeps the hwmon (registered while its components are bound, not device managed: bind/unbind may
 * repeat). The maximal fan speeds come from the fan method (legion_hwmon_fm_register_notifier).
 *
 * Return: The hwmon, or ERR_PTR on an error.
 */
struct legion_hwmon *legion_hwmon_register(struct device *parent,
					   struct notifier_block *source)
{
	struct legion_hwmon *hwmon;
	int err;

	hwmon = kzalloc(sizeof(*hwmon), GFP_KERNEL);
	if (!hwmon)
		return ERR_PTR(-ENOMEM);

	hwmon->source = source;

	err = legion_hwmon_supported(source, temperature_ids,
				     &hwmon->temperatures);
	if (err)
		goto err_free;

	legion_hwmon_temperatures_present(parent, hwmon);

	err = legion_hwmon_supported(source, fan_ids, &hwmon->fans);
	if (err)
		goto err_free;

	hwmon->dev = hwmon_device_register_with_info(
		parent, "legion", hwmon, &legion_hwmon_chip_info, NULL);
	if (IS_ERR(hwmon->dev)) {
		err = PTR_ERR(hwmon->dev);
		goto err_free;
	}

	return hwmon;

err_free:
	kfree(hwmon);
	return ERR_PTR(err);
}

/**
 * legion_hwmon_unregister() - Remove the hwmon device of a source
 * @hwmon: The hwmon (NULL or ERR_PTR: nothing).
 */
void legion_hwmon_unregister(struct legion_hwmon *hwmon)
{
	if (IS_ERR_OR_NULL(hwmon))
		return;

	hwmon_device_unregister(hwmon->dev);
	kfree(hwmon);
}
