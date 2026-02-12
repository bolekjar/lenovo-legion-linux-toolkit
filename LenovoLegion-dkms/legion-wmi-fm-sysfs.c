// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */


#include "legion-wmi-fm-sysfs.h"
#include "legion-wmi-fm.h"
#include "legion-wmi-ftable.h"
#include "legion-firmware-attributes-class.h"
#include "legion-wmi-helpers.h"

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/component.h>
#include <linux/container_of.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/gfp_types.h>
#include <linux/idr.h>
#include <linux/wmi.h>

// set fan to maximal speed; dust cleaning mode
// only works in custom power mode
#define WMI_METHOD_ID_FAN_GET_FULLSPEED 1
#define WMI_METHOD_ID_FAN_SET_FULLSPEED 2

// max speed of fan
#define WMI_METHOD_ID_FAN_GET_MAXSPEED 3
#define WMI_METHOD_ID_FAN_SET_MAXSPEED 4

// fan table in custom mode
#define WMI_METHOD_ID_FAN_GET_TABLE 5
#define WMI_METHOD_ID_FAN_SET_TABLE 6

// get speed of fans
#define WMI_METHOD_ID_FAN_GET_CURRENT_FAN_SPEED 7
// get temperatures of CPU and GPU used for controlling cooling
#define WMI_METHOD_ID_FAN_GET_CURRENT_SENSOR_TEMPERATURE 8

#define LEGION_WMI_FM_FW_ATTR_BASE_PATH "legion-wmi-fan-mode"


#define MAX_FANCURVE_SIZE 10

enum FanTableType
{
    CPU			= 0,
    CPUSensor	= 1,
    GPU			= 2,
    SYS			= 3
};

enum TableType
{
	FAN			= 0,
	SENSOR		= 1
};


static const int fan_table_id_v1[]  = {1,1,2,0};
static const int fan_sensor_id_v1[] = {4,1,5,0};


static const int fan_table_id_v2[]  = {1,0,2,4};
static const int fan_sensor_id_v2[] = {1,0,5,4};


static DEFINE_IDA(legion_wmi_fm_ida);


struct WMIFanTableWrite {
    u8  FSTM;  //FSMD
    u8  FSID;
    u32 FSTL;  //FSST
    u16 FSS0;
    u16 FSS1;
    u16 FSS2;
    u16 FSS3;
    u16 FSS4;
    u16 FSS5;
    u16 FSS6;
    u16 FSS7;
    u16 FSS8;
    u16 FSS9;
} __packed;

struct WMIFanTableRead {
    u32 FSFL;
    u32 FSS0;
    u32 FSS1;
    u32 FSS2;
    u32 FSS3;
    u32 FSS4;
    u32 FSS5;
    u32 FSS6;
    u32 FSS7;
    u32 FSS8;
    u32 FSS9;
    u32 FSSA;
} __packed;


static ssize_t func_fan_curve_current_value_get(struct kobject *kobj,struct kobj_attribute *kattr, char *buf)
{
	struct legion_wmi_fm_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));
    u8 buffer[88 * 2] = { 0 };
    struct WMIFanTableRead * fantable =(struct WMIFanTableRead *)buffer;

    int ret = 0;

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

	ret = legion_wmi_dev_evaluate_buffer(priv->wdev,0,WMI_METHOD_ID_FAN_GET_TABLE,NULL,0,buffer,sizeof(buffer));
	if(ret)
		return ret;

	return sysfs_emit(buf, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu\n",
			fantable->FSS0,
			fantable->FSS1,
			fantable->FSS2,
			fantable->FSS3,
			fantable->FSS4,
			fantable->FSS5,
			fantable->FSS6,
			fantable->FSS7,
			fantable->FSS8,
			fantable->FSS9);
}

static ssize_t func_fan_curve_current_value_set(struct kobject *kobj, struct kobj_attribute *kattr,
		const char *buf, size_t count)
{
	struct legion_wmi_fm_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));
	u8 points[20]  		= {0};
	u8 buffer[0x20] 	= {0};
	int ret  			= 0;
	enum thermal_mode 	mode;

	ret = legion_wmi_fm_notifier_call(&mode,LEGION_WMI_GZ_GET_THERMAL_MODE);
	if (ret) {
		return ret;
	}

	if (mode != LEGION_WMI_GZ_THERMAL_MODE_CUSTOM) {
		return -EBUSY;
	}

	if(sscanf(buf,"%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu",&points[0],
																      &points[1],
																      &points[2],
																      &points[3],
																      &points[4],
																	  &points[5],
																	  &points[6],
																	  &points[7],
																	  &points[8],
																	  &points[9]) != MAX_FANCURVE_SIZE)
	{
		return -EINVAL;
	}

	for (int i = 0;i  < MAX_FANCURVE_SIZE;i++)
		if(points[i] > 10)
			return -EINVAL;

	buffer[0x00] = 1;
	buffer[0x01] = 0;
	buffer[0x02] = 0;
	buffer[0x06] = points[0];
	buffer[0x08] = points[1];
	buffer[0x0A] = points[2];
	buffer[0x0C] = points[3];
	buffer[0x0E] = points[4];
	buffer[0x10] = points[5];
	buffer[0x12] = points[6];
	buffer[0x14] = points[7];
	buffer[0x16] = points[8];
	buffer[0x18] = points[9];

	ret = legion_wmi_dev_evaluate_buffer(priv->wdev,0,WMI_METHOD_ID_FAN_SET_TABLE,buffer,sizeof(buffer),NULL,0);
	if(ret)
		return ret;

	return count;
}


#define __LEGION_WMI_DEFAULT_TABLES(_attribute,_func,_fan_id,_sensor_id,_table)\
static ssize_t func_##_func##_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) {\
\
	struct legion_wmi_fm_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));\
	int ret = 0;\
	int len = 0;\
	struct legion_wmi_ftable ftable;\
\
	enum thermal_mode modes[THERMAL_MODES_SIZE] = {LEGION_WMI_GZ_THERMAL_MODE_END};\
\
	/* Critical: Prevent NULL pointer dereference and system freeze */\
	if (!priv) {\
		return -ENODEV;\
	}\
\
	ret = legion_wmi_fm_notifier_call(&modes,LEGION_WMI_GZ_GET_SUPPORTED_THERMAL_MODES);\
	if (ret) {\
		return ret;\
	}\
\
	for (int i = 0;(i < THERMAL_MODES_SIZE) && (modes[i] != LEGION_WMI_GZ_THERMAL_MODE_END);i++)\
	{\
		u16 *table 		= NULL;\
		int  table_len 	= 0;\
\
		len += sysfs_emit_at(buf,len,"%d=",modes[i]);\
\
		ret = legion_wmi_ftable_get_data(priv->ft_list,modes[i],_fan_id,_sensor_id, &ftable);\
		if (ret) {\
			return ret;\
		}\
\
		if(_table == FAN)\
		{\
			table 		= ftable.fanTableData;\
			table_len 	= sizeof(ftable.fanTableData)/sizeof(ftable.fanTableData[0]);\
		} else if(_table == SENSOR)\
		{\
			table 		= ftable.sensorTableData;\
			table_len 	= sizeof(ftable.sensorTableData)/sizeof(ftable.sensorTableData[0]);\
		} else \
			return -EINVAL;\
\
		for(int i = 0 ; i < table_len;i++) {\
			len +=sysfs_emit_at(buf,len,"%d",table[i]);\
			if((i + 1) != table_len)\
				len += sysfs_emit_at(buf,len,"|");\
		}\
\
		if(modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)\
			len += sysfs_emit_at(buf,len,",");\
	}\
\
	len += sysfs_emit_at(buf,len,"\n");\
	return len;\
}\
static struct kobj_attribute kobj_attr_##_func##_value  	= __LEGION_WMI_ATTR_RO(_attribute,_func);


static struct kobj_attribute kobj_attr_fan_curve_current_value  		= __LEGION_WMI_ATTR_RW(current_value,fan_curve_current_value);


/*
 * Default Table values
 */

__LEGION_WMI_DEFAULT_TABLES(cpu_fan_default,cpu_fan_default_v1,fan_table_id_v1[CPU],fan_sensor_id_v1[CPU],FAN);
__LEGION_WMI_DEFAULT_TABLES(cpu_sensor_default,cpu_sensor_default_v1,fan_table_id_v1[CPU],fan_sensor_id_v1[CPU],SENSOR);
__LEGION_WMI_DEFAULT_TABLES(cpusen_fan_default,cpusen_fan_default_v1,fan_table_id_v1[CPUSensor],fan_sensor_id_v1[CPUSensor],FAN);
__LEGION_WMI_DEFAULT_TABLES(cpusen_sensor_default,cpusen_sensor_default_v1,fan_table_id_v1[CPUSensor],fan_sensor_id_v1[CPUSensor],SENSOR);
__LEGION_WMI_DEFAULT_TABLES(gpu_fan_default,gpu_fan_default_v1,fan_table_id_v1[GPU],fan_sensor_id_v1[GPU],FAN);
__LEGION_WMI_DEFAULT_TABLES(gpu_sensor_default,gpu_sensor_default_v1,fan_table_id_v1[GPU],fan_sensor_id_v1[GPU],SENSOR);



__LEGION_WMI_DEFAULT_TABLES(cpu_fan_default,cpu_fan_default_v2,fan_table_id_v2[CPU],fan_sensor_id_v2[CPU],FAN);
__LEGION_WMI_DEFAULT_TABLES(cpu_sensor_default,cpu_sensor_default_v2,fan_table_id_v2[SYS],fan_sensor_id_v2[SYS],SENSOR);
__LEGION_WMI_DEFAULT_TABLES(gpu_fan_default,gpu_fan_default_v2,fan_table_id_v2[GPU],fan_sensor_id_v2[GPU],FAN);
__LEGION_WMI_DEFAULT_TABLES(gpu_sensor_default,gpu_sensor_default_v2,fan_table_id_v2[GPU],fan_sensor_id_v2[GPU],SENSOR);
__LEGION_WMI_DEFAULT_TABLES(sys_fan_default,sys_fan_default_v2,fan_table_id_v2[SYS],fan_sensor_id_v2[SYS],FAN);
__LEGION_WMI_DEFAULT_TABLES(sys_sensor_default,sys_sensor_default_v2,fan_table_id_v2[CPU],fan_sensor_id_v2[CPU],SENSOR);




__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,"FAN curve related settings",fan_curve_display_name)


static struct attribute *legion_sysfs_fm_fan_curve_attributes_v1[] = {
		&kobj_attr_fan_curve_current_value.attr,
		&kobj_attr_cpu_fan_default_v1_value.attr,
		&kobj_attr_cpu_sensor_default_v1_value.attr,
		&kobj_attr_cpusen_fan_default_v1_value.attr,
		&kobj_attr_cpusen_sensor_default_v1_value.attr,
		&kobj_attr_gpu_fan_default_v1_value.attr,
		&kobj_attr_gpu_sensor_default_v1_value.attr,
		&kobj_attr_fan_curve_display_name.attr,
		NULL
};

static const struct attribute_group legion_sysfs_fm_group_v1[] = {
		{
			.name = "fan_curve",
			.attrs = legion_sysfs_fm_fan_curve_attributes_v1
		},
		{}
};

static struct attribute *legion_sysfs_fm_fan_curve_attributes_v2[] = {
		&kobj_attr_fan_curve_current_value.attr,
		&kobj_attr_cpu_fan_default_v2_value.attr,
		&kobj_attr_cpu_sensor_default_v2_value.attr,
		&kobj_attr_gpu_fan_default_v2_value.attr,
		&kobj_attr_gpu_sensor_default_v2_value.attr,
		&kobj_attr_sys_fan_default_v2_value.attr,
		&kobj_attr_sys_sensor_default_v2_value.attr,
		&kobj_attr_fan_curve_display_name.attr,
		NULL
};

static const struct attribute_group legion_sysfs_fm_group_v2[] = {
		{
			.name = "fan_curve",
			.attrs = legion_sysfs_fm_fan_curve_attributes_v2
		},
		{}
};


static int legion_wmi_fm_fw_attr_add(struct legion_wmi_fm_priv *priv)
{
	int err 			= 0;
	unsigned int i   	= 0;

	/*
	 * Read SmartFan version
	 */
	err = legion_wmi_fm_notifier_call(&priv->smart_fan_version,LEGION_WMI_GZ_GET_SMARTFAN_VERSION);
	if (err) {
		return err;
	}

	priv->ida_id = ida_alloc(&legion_wmi_fm_ida, GFP_KERNEL);
	if (priv->ida_id < 0)
		return priv->ida_id;


	priv->fw_attr_dev = device_create(&legion_firmware_attributes_class, NULL,
					  MKDEV(0, 0), priv, "%s-%u",
					  LEGION_WMI_FM_FW_ATTR_BASE_PATH,
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


	if(priv->smart_fan_version == 8)
	{
		for (i = 0; i < ARRAY_SIZE(legion_sysfs_fm_group_v2) - 1; i++) {
			err = sysfs_create_group(&priv->fw_attr_kset->kobj,
					&legion_sysfs_fm_group_v2[i]);
			if (err)
				goto err_remove_groups;
		}

		for(int i = 0;i < FAN_MAX; ++i)
		{
			if(priv->fan_max_speeds[i].fan_id == fan_table_id_v2[CPU] && priv->fan_max_speeds[i].sensor_id == fan_sensor_id_v2[CPU])
			{
				priv->fan_cpu_max_speed = priv->fan_max_speeds[i].max_speed;
			}
			else if(priv->fan_max_speeds[i].fan_id == fan_table_id_v2[GPU] && priv->fan_max_speeds[i].sensor_id == fan_sensor_id_v2[GPU])
			{
				priv->fan_gpu_max_speed = priv->fan_max_speeds[i].max_speed;
			}
			else if(priv->fan_max_speeds[i].fan_id == fan_table_id_v2[SYS] && priv->fan_max_speeds[i].sensor_id == fan_sensor_id_v2[SYS])
			{
				priv->fan_sys_max_speed = priv->fan_max_speeds[i].max_speed;
			}
		}
	}
	else
	{
		for (i = 0; i < ARRAY_SIZE(legion_sysfs_fm_group_v1) - 1; i++) {
			err = sysfs_create_group(&priv->fw_attr_kset->kobj,
					&legion_sysfs_fm_group_v1[i]);
			if (err)
				goto err_remove_groups;
		}

		for(int i = 0;i < FAN_MAX; ++i)
		{
			if(priv->fan_max_speeds[i].fan_id == fan_table_id_v1[CPU] && priv->fan_max_speeds[i].sensor_id == fan_sensor_id_v1[CPU])
			{
				priv->fan_cpu_max_speed = priv->fan_max_speeds[i].max_speed;
			}
			else if(priv->fan_max_speeds[i].fan_id == fan_table_id_v1[GPU] && priv->fan_max_speeds[i].sensor_id == fan_sensor_id_v1[GPU])
			{
				priv->fan_gpu_max_speed = priv->fan_max_speeds[i].max_speed;
			}
		}
	}

	return 0;
err_remove_groups:
	if(priv->smart_fan_version == 8)
		while (i--)
			sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_fm_group_v2[i]);
	else
		while (i--)
			sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_fm_group_v1[i]);
	kset_unregister(priv->fw_attr_kset);
err_unregister_fw_attr_dev:
	device_unregister(priv->fw_attr_dev);
err_free_ida:
	ida_free(&legion_wmi_fm_ida, priv->ida_id);
	return err;
}

static void legion_wmi_fm_fw_attr_remove(struct legion_wmi_fm_priv *priv)
{
	/* Remove sysfs group first */
	if(priv->smart_fan_version == 8)
		for (unsigned int i = 0; i < ARRAY_SIZE(legion_sysfs_fm_group_v2) - 1; i++)
			sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_fm_group_v2[i]);
	else
		for (unsigned int i = 0; i < ARRAY_SIZE(legion_sysfs_fm_group_v1) - 1; i++)
			sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_fm_group_v1[i]);

	/* Unregister kset before device since kset is child of device's kobj */
	kset_unregister(priv->fw_attr_kset);

	/* Device unregister will clean up remaining kobject hierarchy */
	device_unregister(priv->fw_attr_dev);

	ida_free(&legion_wmi_fm_ida, priv->ida_id);
}

int  legion_wmi_fm_sysfs_init(struct legion_wmi_fm_priv *priv) {
	return legion_wmi_fm_fw_attr_add(priv);
}


void legion_wmi_fm_sysfs_exit(struct legion_wmi_fm_priv *priv) {
	legion_wmi_fm_fw_attr_remove(priv);
}
