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
	struct notifier_block *nb = data;

	legion_hwmon_other_unregister_notifier_dev(nb);
}

int legion_hwmon_other_register_notifier(struct device *dev,struct notifier_block *nb)
{
	int ret;

	ret = legion_hwmon_other_register_notifier_dev(nb);
	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(dev, legion_hwmon_wmi_other_unregister_notifier,nb);
}

static int legion_hwmon_other_notifier_call(void *data,enum CapabilityID capability_id)
{
	int ret;

	struct hwmon_capability req_data = {capability_id,data};

	ret = blocking_notifier_call_chain(&legion_hwmon_chain_head,LEGION_WMI_FM_GET_CAPABILITY_VALUE, &req_data);
	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

static int legion_hwmon_other_data00_notifier_call(void *data,enum CapabilityID capability_id)
{
	int ret;

	struct hwmon_capability req_data = {capability_id,data};

	ret = blocking_notifier_call_chain(&legion_hwmon_chain_head,LEGION_WMI_FM_GET_CAPABILITY_DATA, &req_data);
	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}




static int legion_hwmon_fm_register_notifier_dev(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&legion_hwmon_fm_chain_head, nb);
}

static int legion_hwmon_fm_unregister_notifier_dev(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&legion_hwmon_fm_chain_head, nb);
}

static void legion_hwmon_fm_unregister_notifier(void *data)
{
	struct notifier_block *nb = data;

	legion_hwmon_fm_unregister_notifier_dev(nb);
}

int legion_hwmon_fm_register_notifier(struct device *dev,struct notifier_block *nb)
{
	int ret;

	ret = legion_hwmon_fm_register_notifier_dev(nb);
	if (ret < 0)
		return ret;

	return devm_add_action_or_reset(dev, legion_hwmon_fm_unregister_notifier,nb);
}

static int legion_hwmon_fm_notifier_call(void *data,enum fm_events_type events_type)
{
	int ret;

	ret = blocking_notifier_call_chain(&legion_hwmon_fm_chain_head,events_type, data);
	if ((ret & ~NOTIFY_STOP_MASK) != NOTIFY_OK)
		return -EINVAL;

	return 0;
}

enum SENSOR_CHANNELS {
	SENSOR_SYS_TEMP_ID        = 0,
    SENSOR_CPU_TEMP_ID        = 1,
    SENSOR_GPU_TEMP_ID        = 2,

    SENSOR_CPU_FAN_RPM_ID     = 3,
    SENSOR_GPU_FAN_RPM_ID     = 4,
	SENSOR_SYS_FAN_RPM_ID     = 5,

    SENSOR_CPU_FAN_MIN_ID     = 6,
    SENSOR_GPU_FAN_MIN_ID     = 7,
	SENSOR_SYS_FAN_MIN_ID     = 8,

    SENSOR_CPU_FAN_MAX_ID     = 9,
    SENSOR_GPU_FAN_MAX_ID     = 10,
	SENSOR_SYS_FAN_MAX_ID     = 11,


    NOT_SUPPORTED             = 12
};



static const char * const sensors_names[] = {
	[SENSOR_SYS_TEMP_ID]	       =	"SYS Temperature",
    [SENSOR_CPU_TEMP_ID]	       =	"CPU Temperature",
    [SENSOR_GPU_TEMP_ID]	       =	"GPU Temperature",
    [SENSOR_CPU_FAN_RPM_ID]	       =	"CPU Fan",
    [SENSOR_GPU_FAN_RPM_ID]	       =	"GPU Fan",
	[SENSOR_SYS_FAN_RPM_ID]		   =    "SYS Fan"
};

static ssize_t legion_read_cpu_temprature(u32 *temperature)
{
    ssize_t ret = legion_hwmon_other_notifier_call(temperature,CpuCurrentTemperature);
	if (ret) {
		return ret;
	}

    *temperature *= 1000;

    return ret;
}

static ssize_t legion_read_gpu_temprature(u32 *temperature)
{
    ssize_t ret = legion_hwmon_other_notifier_call(temperature,GpuCurrentTemperature);
	if (ret) {
		return ret;
	}

    *temperature *= 1000;

    return ret;
}

static ssize_t legion_read_sys_temprature(u32 *temperature)
{
    ssize_t ret = legion_hwmon_other_notifier_call(temperature,SysCurrentTemperature);
	if (ret) {
		return ret;
	}

    *temperature *= 1000;

    return ret;
}

static ssize_t legion_read_cpu_fan_speed(u32 *temperature)
{
    return legion_hwmon_other_notifier_call(temperature,CpuCurrentFanSpeed);
}

static ssize_t legion_read_gpu_fan_speed(u32 *temperature)
{
    return legion_hwmon_other_notifier_call(temperature,GpuCurrentFanSpeed);
}

static ssize_t legion_read_sys_fan_speed(u32 *temperature)
{
    return legion_hwmon_other_notifier_call(temperature,SysCurrentFanSpeed);
}

static ssize_t legion_read_cpu_fan_min(u32 *temperature)
{
    *temperature = 0;
    return 0;
}

static ssize_t legion_read_gpu_fan_min(u32 *temperature)
{
    *temperature = 0;
    return 0;
}

static ssize_t legion_read_sys_fan_min(u32 *temperature)
{
    *temperature = 0;
    return 0;
}

static ssize_t legion_read_cpu_fan_max(u32 *max_speed)
{
    return legion_hwmon_fm_notifier_call(max_speed,LEGION_WMI_FM_GET_CPU_FAN_MAX_SPEED);
}

static ssize_t legion_read_gpu_fan_max(u32 *max_speed)
{
    return legion_hwmon_fm_notifier_call(max_speed,LEGION_WMI_FM_GET_GPU_FAN_MAX_SPEED);
}

static ssize_t legion_read_sys_fan_max(u32 *max_speed)
{
    return legion_hwmon_fm_notifier_call(max_speed,LEGION_WMI_FM_GET_SYS_FAN_MAX_SPEED);
}

static ssize_t legion_read_not_supported(u32 *temperature)
{
    return -EOPNOTSUPP;
}



typedef ssize_t (*legion_read_hwmon)(u32 *temperature);

legion_read_hwmon  sensors_handlers[] = {
	[SENSOR_SYS_TEMP_ID]           =    legion_read_sys_temprature,
    [SENSOR_CPU_TEMP_ID]	       =	legion_read_cpu_temprature,
    [SENSOR_GPU_TEMP_ID]	       =	legion_read_gpu_temprature,
    [SENSOR_CPU_FAN_RPM_ID]	       =	legion_read_cpu_fan_speed,
    [SENSOR_GPU_FAN_RPM_ID]	       =    legion_read_gpu_fan_speed,
	[SENSOR_SYS_FAN_RPM_ID]        = 	legion_read_sys_fan_speed,
    [SENSOR_CPU_FAN_MIN_ID]        =    legion_read_cpu_fan_min,
    [SENSOR_GPU_FAN_MIN_ID]        =    legion_read_gpu_fan_min,
    [SENSOR_SYS_FAN_MIN_ID]        =    legion_read_sys_fan_min,
    [SENSOR_CPU_FAN_MAX_ID]        =    legion_read_cpu_fan_max,
    [SENSOR_GPU_FAN_MAX_ID]        =    legion_read_gpu_fan_max,
	[SENSOR_SYS_FAN_MAX_ID]        =    legion_read_sys_fan_max,
    [NOT_SUPPORTED]                =    legion_read_not_supported
};


static ssize_t sensor_label_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n",sensors_names[to_sensor_dev_attr(attr)->index]);
}


static ssize_t sensor_show(struct device *dev, struct device_attribute *devattr,
                           char *buf)
{

    int outval = 0;

    if(sensors_handlers[( (to_sensor_dev_attr(devattr))->index >= NOT_SUPPORTED ? NOT_SUPPORTED : (to_sensor_dev_attr(devattr))->index)](&outval) != 0)
    {
        return -EIO;
    }

    return sysfs_emit(buf, "%d\n", outval);
}


/*
 * Temp
 */
static SENSOR_DEVICE_ATTR_RO(temp1_input, sensor, SENSOR_CPU_TEMP_ID);
static SENSOR_DEVICE_ATTR_RO(temp1_label, sensor_label, SENSOR_CPU_TEMP_ID);
static SENSOR_DEVICE_ATTR_RO(temp2_input, sensor, SENSOR_GPU_TEMP_ID);
static SENSOR_DEVICE_ATTR_RO(temp2_label, sensor_label, SENSOR_GPU_TEMP_ID);
static SENSOR_DEVICE_ATTR_RO(temp3_input, sensor, SENSOR_SYS_TEMP_ID);
static SENSOR_DEVICE_ATTR_RO(temp3_label, sensor_label, SENSOR_SYS_TEMP_ID);

/*static struct attribute *sensor_hwmon_attributes_temp[] = {
    &sensor_dev_attr_temp1_input.dev_attr.attr,
    &sensor_dev_attr_temp1_label.dev_attr.attr,
    &sensor_dev_attr_temp2_input.dev_attr.attr,
    &sensor_dev_attr_temp2_label.dev_attr.attr,
    &sensor_dev_attr_temp3_input.dev_attr.attr,
    &sensor_dev_attr_temp3_label.dev_attr.attr,
    NULL
};*/

static struct attribute *sensor_hwmon_attributes_temp[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

    NULL
};

static const struct attribute_group legion_hwmon_sensor_group_temp = {
    .attrs = sensor_hwmon_attributes_temp
};


/*
 * Fan 1
 */
static SENSOR_DEVICE_ATTR_RO(fan1_input, sensor, SENSOR_CPU_FAN_RPM_ID);
static SENSOR_DEVICE_ATTR_RO(fan1_label, sensor_label, SENSOR_CPU_FAN_RPM_ID);
static SENSOR_DEVICE_ATTR_RO(fan1_min, sensor, SENSOR_SYS_FAN_MIN_ID);
static SENSOR_DEVICE_ATTR_RO(fan1_max, sensor, SENSOR_CPU_FAN_MAX_ID);

static struct attribute *sensor_hwmon_attributes_fan_cpu[] = {
    &sensor_dev_attr_fan1_input.dev_attr.attr,
    &sensor_dev_attr_fan1_label.dev_attr.attr,
    &sensor_dev_attr_fan1_min.dev_attr.attr,
    &sensor_dev_attr_fan1_max.dev_attr.attr,
    NULL
};

static const struct attribute_group legion_hwmon_sensor_group_fan_cpu = {
    .attrs = sensor_hwmon_attributes_fan_cpu
};

/*
 * Fan 2
 */
static SENSOR_DEVICE_ATTR_RO(fan2_input, sensor, SENSOR_GPU_FAN_RPM_ID);
static SENSOR_DEVICE_ATTR_RO(fan2_label, sensor_label, SENSOR_GPU_FAN_RPM_ID);
static SENSOR_DEVICE_ATTR_RO(fan2_min, sensor, SENSOR_GPU_FAN_MIN_ID);
static SENSOR_DEVICE_ATTR_RO(fan2_max, sensor, SENSOR_GPU_FAN_MAX_ID);

static struct attribute *sensor_hwmon_attributes_fan_gpu[] = {
    &sensor_dev_attr_fan2_input.dev_attr.attr,
    &sensor_dev_attr_fan2_label.dev_attr.attr,
    &sensor_dev_attr_fan2_min.dev_attr.attr,
    &sensor_dev_attr_fan2_max.dev_attr.attr,
    NULL
};


/*
 * Fan 3
 */

/*
 * Fan 2
 */
static SENSOR_DEVICE_ATTR_RO(fan3_input, sensor, SENSOR_SYS_FAN_RPM_ID);
static SENSOR_DEVICE_ATTR_RO(fan3_label, sensor_label, SENSOR_SYS_FAN_RPM_ID);
static SENSOR_DEVICE_ATTR_RO(fan3_min, sensor, SENSOR_SYS_FAN_MIN_ID);
static SENSOR_DEVICE_ATTR_RO(fan3_max, sensor, SENSOR_SYS_FAN_MAX_ID);


static struct attribute *sensor_hwmon_attributes_fan_sys[] = {
    &sensor_dev_attr_fan3_input.dev_attr.attr,
    &sensor_dev_attr_fan3_label.dev_attr.attr,
    &sensor_dev_attr_fan3_min.dev_attr.attr,
    &sensor_dev_attr_fan3_max.dev_attr.attr,
    NULL
};


static const struct attribute_group legion_hwmon_sensor_group_fan_gpu = {
    .attrs = sensor_hwmon_attributes_fan_gpu
};

static const struct attribute_group legion_hwmon_sensor_group_fan_sys = {
    .attrs = sensor_hwmon_attributes_fan_sys
};

static const struct attribute_group *legion_hwmon_groups[] = {
	NULL,
	NULL,
	NULL,
	NULL,

    NULL
};

static void insert_hwmon_attributes_temp(struct attribute * attribute_input,struct attribute * attribute_label)
{
	for(int i = 0; i < ((sizeof(sensor_hwmon_attributes_temp) / sizeof(sensor_hwmon_attributes_temp[0])) - 1);++i)
	{
		if(sensor_hwmon_attributes_temp[i] == NULL)
		{
			sensor_hwmon_attributes_temp[i] = attribute_input;
			sensor_hwmon_attributes_temp[i + 1] = attribute_label;
			return;
		}
	}
}

static void insert_hwmon_attributes_group(const struct attribute_group * attribute_group)
{
	for(int i = 0; i < ((sizeof(legion_hwmon_groups) / sizeof(legion_hwmon_groups[0])) - 1);++i)
	{
		if(legion_hwmon_groups[i] == NULL)
		{
			legion_hwmon_groups[i] = attribute_group;
			return;
		}
	}
}


int legion_hwmon_init(struct device *parent)
{
	struct legion_data* 		  data 					= dev_get_drvdata(parent);
	struct legion_wmi_capdata00   cap_data00 			= {0,0,0};
	int err 											= 0;

	if (!data)
	    return -ENODEV;

	err = legion_hwmon_other_data00_notifier_call(&cap_data00,CpuCurrentTemperature);
	if (err) {
		return err;
	}
	if(cap_data00.id == CpuCurrentTemperature && cap_data00.supported)
	{
		insert_hwmon_attributes_temp(&sensor_dev_attr_temp1_input.dev_attr.attr,&sensor_dev_attr_temp1_label.dev_attr.attr);
	}


	cap_data00.id 			 = 0;
	cap_data00.default_value = 0;
	cap_data00.supported 	 = 0;
	err = legion_hwmon_other_data00_notifier_call(&cap_data00,GpuCurrentTemperature);
	if (err) {
		return err;
	}
	if(cap_data00.id == GpuCurrentTemperature && cap_data00.supported)
	{
		insert_hwmon_attributes_temp(&sensor_dev_attr_temp2_input.dev_attr.attr,&sensor_dev_attr_temp2_label.dev_attr.attr);
	}


	cap_data00.id 			 = 0;
	cap_data00.default_value = 0;
	cap_data00.supported 	 = 0;
	err = legion_hwmon_other_data00_notifier_call(&cap_data00,SysCurrentTemperature);
	if (err) {
		return err;
	}
	if(cap_data00.id == SysCurrentTemperature && cap_data00.supported)
	{
		insert_hwmon_attributes_temp(&sensor_dev_attr_temp3_input.dev_attr.attr,&sensor_dev_attr_temp3_label.dev_attr.attr);
	}


	if(sensor_hwmon_attributes_temp[0] != NULL)
	{
		insert_hwmon_attributes_group(&legion_hwmon_sensor_group_temp);
	}



	cap_data00.id 			 = 0;
	cap_data00.default_value = 0;
	cap_data00.supported 	 = 0;
	err = legion_hwmon_other_data00_notifier_call(&cap_data00,CpuCurrentFanSpeed);
	if (err) {
		return err;
	}
	if(cap_data00.id == CpuCurrentFanSpeed && cap_data00.supported)
	{
		insert_hwmon_attributes_group(&legion_hwmon_sensor_group_fan_cpu);
	}


	cap_data00.id 			 = 0;
	cap_data00.default_value = 0;
	cap_data00.supported 	 = 0;
	err = legion_hwmon_other_data00_notifier_call(&cap_data00,GpuCurrentFanSpeed);
	if (err) {
		return err;
	}
	if(cap_data00.id == GpuCurrentFanSpeed && cap_data00.supported)
	{
		insert_hwmon_attributes_group(&legion_hwmon_sensor_group_fan_gpu);
	}


	cap_data00.id 			 = 0;
	cap_data00.default_value = 0;
	cap_data00.supported 	 = 0;
	err = legion_hwmon_other_data00_notifier_call(&cap_data00,SysCurrentFanSpeed);
	if (err) {
		return err;
	}
	if(cap_data00.id == SysCurrentFanSpeed && cap_data00.supported)
	{
		insert_hwmon_attributes_group(&legion_hwmon_sensor_group_fan_sys);
	}

	data->hwmon_dev = hwmon_device_register_with_groups(parent, "legion", NULL,legion_hwmon_groups);
	if (IS_ERR_OR_NULL(data->hwmon_dev)) {
		return PTR_ERR(data->hwmon_dev) ? : -ENODEV;
	}

	return 0;
}

void legion_hwmon_exit(struct device *parent)
{
	struct legion_data* data = dev_get_drvdata(parent);

	if (!data)
		return;

	if(data->hwmon_dev) {
		hwmon_device_unregister(data->hwmon_dev);
	}
}
