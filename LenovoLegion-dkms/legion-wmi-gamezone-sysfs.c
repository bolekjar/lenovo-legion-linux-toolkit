// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-common.h"
#include "legion-wmi-gamezone.h"
#include "legion-wmi-gamezone-sysfs.h"
#include "legion-wmi-helpers.h"
#include "legion-firmware-attributes-class.h"

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/component.h>
#include <linux/container_of.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/gfp_types.h>
#include <linux/idr.h>
#include <linux/wmi.h>
#include <linux/platform_device.h>

static DEFINE_IDA(legion_wmi_gz_ida);

#define LEGION_WMI_GZ_FW_ATTR_BASE_PATH "legion-wmi-gamezone"




static int legion_wmi_validate_input(u32 value,u32 *possible_inputs,size_t possible_inputs_len) {
	for(size_t i = 0;i < possible_inputs_len; i++)
	{
		if(possible_inputs[i] == value)
			return 0;
	}
	return -EINVAL;
}

#define __LEGION_WMI_KOBJ_ATTR_RO_CACHED(_name,_attr,_func) \
static ssize_t func_##_func##_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) \
{																												 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																												 \
	/* Return cached value instead of calling WMI (prevents deadlock/freeze) */									 \
	return sysfs_emit(buf, "%u\n", priv->preloaded_values._attr);												 \
}																												 \
static struct kobj_attribute kobj_attr_##_func  = __LEGION_WMI_ATTR_RO(_name,_func);


#define __LEGION_WMI_KOBJ_ATTR_RO(_name,_attr,_func) \
static ssize_t func_##_func##_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) \
{																												 \
	u32 value;																									 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
	if(legion_wmi_gz_get(priv->wdev,_attr,&value))		 											    	     \
		return -ENODEV;																							 \
	return sysfs_emit(buf, "%u\n", value);																         \
}																												 \
static struct kobj_attribute kobj_attr_##_func  = __LEGION_WMI_ATTR_RO(_name,_func);

#define __LEGION_WMI_KOBJ_ATTR_WO(_name,_attr,_func,_possible_inputs) \
static ssize_t func_##_func##_store(struct kobject *kobj, struct kobj_attribute *kattr,            				 \
		const char *buf, size_t count)   																		 \
{																												 \
	int ret = 0,value = 0;																						 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																												 \
	ret = kstrtouint(buf, 10, &value);																			 \
	if (ret)																									 \
		return ret;\
	ret = legion_wmi_validate_input(value,_possible_inputs,sizeof(_possible_inputs)/sizeof(_possible_inputs[0]));\
	if (ret)																									 \
		return ret;\
	ret = legion_wmi_gz_set(priv->wdev,_attr,value); 														     \
	if (ret)																									 \
		return ret;																								 \
																												 \
	return count;																								 \
}																												 \
static struct kobj_attribute kobj_attr_##_func  = __LEGION_WMI_ATTR_WO(_name,_func);

#define __LEGION_WMI_KOBJ_ATTR_RW(_name,_attrget,_attrset,_func,_possible_inputs)     \
static ssize_t func_##_func##_get(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) 	 		     \
{																												 \
	u32 value;																									 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
    if(legion_wmi_gz_get(priv->wdev,_attrget,&value))		 											    	 \
	    return -ENODEV;																							 \
	return sysfs_emit(buf, "%u\n", value);																         \
}																												 \
static ssize_t func_##_func##_set(struct kobject *kobj, struct kobj_attribute *kattr,            				 \
		const char *buf, size_t count)   																		 \
{																												 \
	int ret = 0,value = 0;																						 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																											     \
	ret = kstrtouint(buf, 10, &value);																			 \
	if (ret)																									 \
		return ret;\
	ret = legion_wmi_validate_input(value,_possible_inputs,sizeof(_possible_inputs)/sizeof(_possible_inputs[0]));\
	if (ret)																									 \
		return ret;\
	\
	ret = legion_wmi_gz_set(priv->wdev,_attrset,value); 														 \
	if (ret)																									 \
		return ret;																								 \
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 \
	return count;																								 \
}																												 \
static struct kobj_attribute kobj_attr_##_func  =                                                  \
		__LEGION_WMI_ATTR_RW(_name,_func);



#define __LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED(_name,_attr,_attrget,_attrset,_dispname,_possible_inputs) \
static ssize_t func_##_name##_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) \
{																												 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																												 \
	/* Return cached value instead of calling WMI (prevents deadlock/freeze) */									 \
	return sysfs_emit(buf, "%u\n", priv->preloaded_values._attr);						    					 \
}                                          																		 \
static ssize_t func_##_name##_display_name_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) 	 \
{																												 \
	return sysfs_emit(buf, "%s\n", _dispname);																     \
}																												 \
static ssize_t func_##_name##_get(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) 	 		     \
{																												 \
	u32 value;																									 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
    if(legion_wmi_gz_get(priv->wdev,_attrget,&value))		 											    	 \
	    return -ENODEV;																							 \
	return sysfs_emit(buf, "%u\n", value);																         \
}																												 \
static ssize_t func_##_name##_set(struct kobject *kobj, struct kobj_attribute *kattr,            				 \
		const char *buf, size_t count)   																		 \
{																												 \
	int ret = 0,value = 0;																						 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																											     \
	ret = kstrtouint(buf, 10, &value);																			 \
	if (ret)																									 \
		return ret;																								 \
		                                                                                                         \
    ret = legion_wmi_validate_input(value,_possible_inputs,sizeof(_possible_inputs)/sizeof(_possible_inputs[0]));\
	if (ret)																									 \
		return ret;                                                                                              \
                                                                                                                 \
	ret = legion_wmi_gz_set(priv->wdev,_attrset,value); 														 \
	if (ret)																									 \
		return ret;																								 \
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 \
	return count;																								 \
}																												 \
static struct kobj_attribute kobj_attr_##_name  = __LEGION_WMI_ATTR_RO(supported,_name);		            	 \
static struct kobj_attribute kobj_attr_display_name_##_name  =                                                   \
		__LEGION_WMI_ATTR_RO(display_name,_name##_display_name);	     		 								 \
static struct kobj_attribute kobj_attr_current_value_##_name  =                                                  \
		__LEGION_WMI_ATTR_RW(current_value,_name);	     				 						 			     \
																												 \
static struct attribute *legion_sysfs_##_name##_attributes[] = {                                                 \
	&kobj_attr_##_name.attr,																		             \
	&kobj_attr_display_name_##_name.attr,																		 \
	&kobj_attr_current_value_##_name.attr,																	     \
	NULL                                                                                                         \
};


#define __LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED_SMARTFAN(_name,_attr,_attrget,_attrset,_dispname,_possible_inputs) \
static ssize_t func_##_name##_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) \
{																												 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																												 \
	/* Return cached value instead of calling WMI (prevents deadlock/freeze) */									 \
	return sysfs_emit(buf, "%u\n", priv->preloaded_values._attr);						    					 \
}																												 \
static ssize_t func_##_name##_extreme_supported##_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf)\
{																												 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																												 \
	/* Return cached value instead of calling WMI (prevents deadlock/freeze) */									 \
	return sysfs_emit(buf, "%u\n", priv->extreme_supported);						    					 \
}        																										 \
static ssize_t func_##_name##_display_name_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) 	 \
{																												 \
	return sysfs_emit(buf, "%s\n", _dispname);																     \
}																												 \
static ssize_t func_##_name##_get(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) 	 		     \
{																												 \
	u32 value;																									 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
    if(legion_wmi_gz_get(priv->wdev,_attrget,&value))		 											    	 \
	    return -ENODEV;																							 \
	return sysfs_emit(buf, "%u\n", value);																         \
}																												 \
static ssize_t func_##_name##_set(struct kobject *kobj, struct kobj_attribute *kattr,            				 \
		const char *buf, size_t count)   																		 \
{																												 \
	int ret = 0,value = 0;																						 \
	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));								 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																											     \
	ret = kstrtouint(buf, 10, &value);																			 \
	if (ret)																									 \
		return ret;																								 \
		                                                                                                         \
    ret = legion_wmi_validate_input(value,_possible_inputs,sizeof(_possible_inputs)/sizeof(_possible_inputs[0]));\
	if (ret)																									 \
		return ret;                                                                                              \
                                                                                                                 \
	ret = legion_wmi_gz_set(priv->wdev,_attrset,value); 														 \
	if (ret)																									 \
		return ret;																								 \
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 \
	return count;																								 \
}																												 \
static struct kobj_attribute kobj_attr_##_name  = __LEGION_WMI_ATTR_RO(supported,_name);		            	 \
static struct kobj_attribute kobj_attr_display_name_##_name  =                                                   \
		__LEGION_WMI_ATTR_RO(display_name,_name##_display_name);	     		 								 \
static struct kobj_attribute kobj_attr_current_value_##_name  =                                                  \
		__LEGION_WMI_ATTR_RW(current_value,_name);	     				 						 			     \
static struct kobj_attribute kobj_attr_extreme_supported_value_##_name  =                                        \
		__LEGION_WMI_ATTR_RO(extreme_supported,_name##_extreme_supported);	     				 						 			 \
																												 \
static struct attribute *legion_sysfs_##_name##_attributes[] = {                                                 \
	&kobj_attr_##_name.attr,																		             \
	&kobj_attr_display_name_##_name.attr,																		 \
	&kobj_attr_current_value_##_name.attr,																	     \
	&kobj_attr_extreme_supported_value_##_name.attr,															 \
	NULL                                                                                                         \
};

/*
 * Features: isSupported, Get, Set
 */
static u32 SmartFanPossibleValues[] = {
		LEGION_WMI_GZ_THERMAL_MODE_END,
		LEGION_WMI_GZ_THERMAL_MODE_QUIET,
		LEGION_WMI_GZ_THERMAL_MODE_BALANCED,
		LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE,
		LEGION_WMI_GZ_THERMAL_MODE_EXTREME,
		LEGION_WMI_GZ_THERMAL_MODE_CUSTOM
};

/*
 * IGPUModeStates
 */
static u32 IGPUModeStates[] = {
		0, //Default
		1, //IGPUOnly
		2  //Auto
};

/*
 * Other
 */
static u32 StateONOFF[] = {
		0, //Off
		1  //On
};

__LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED_SMARTFAN(smart_fan,IsSupportSmartFan,GetSmartFanMode,SetSmartFanMode,"Smart Fan feature",SmartFanPossibleValues);
__LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED(igpu_mode,IsSupportIGPUMode,GetIGPUModeStatus,SetIGPUModeStatus,"IGPU mode",IGPUModeStates);
__LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED(disable_win_key,IsSupportDisableWinKey,GetWinKeyStatus,SetWinKeyStatus,"EC disable/enable windows key capability",StateONOFF);
__LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED(disable_tp,IsSupportDisableTP,GetTPStatus,SetTPStatus,"EC disable/enable touchpad capability",StateONOFF);
__LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED(gsync,IsSupportGSync,GetGSyncStatus,SetGSyncStatus,"G-Sync feature",StateONOFF);
__LEGION_WMI_RO_FEATURE_SET_GET_SUPPORTED(over_drive,IsSupportOD,GetODStatus,SetODStatus,"Over Drive feature capability",StateONOFF);



/*
 * Features: Get
 */

////////////////// Fan related //////////////////////////////////////////
__LEGION_WMI_KOBJ_ATTR_RO_CACHED(count,GetFanCount,fan_count);
__LEGION_WMI_KOBJ_ATTR_RO(fan1_speed,GetFan1Speed,fan1_speed);
__LEGION_WMI_KOBJ_ATTR_RO(fan2_speed,GetFan2Speed,fan2_speed);
__LEGION_WMI_KOBJ_ATTR_RO(max_speed,GetFanMaxSpeed,fan_max_speed);
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,"Others Fan related features",display_fan)

static struct attribute *legion_sysfs_fans_attributes[] = {
	&kobj_attr_fan_count.attr,
	&kobj_attr_fan1_speed.attr,
	&kobj_attr_fan2_speed.attr,
	&kobj_attr_fan_max_speed.attr,
	&kobj_attr_display_fan.attr,
	NULL
};
////////////////// Fan related //////////////////////////////////////////




////////////////// GPU related //////////////////////////////////////////

static u32 StateGPUONOFF[] = {
		0, //Off
		1  //On
};

/*
 * GetDGPUHWId
 */
static ssize_t func_dgpu_hwid_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf) \
{
	char value[128] = {0};
	int ret =0;

	struct lenovo_wmi_gz_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}
	ret = legion_wmi_gz_get_string(priv->wdev,GetDGPUHWId,value,128);
	if(ret)
		return -ENODEV;

	return sysfs_emit(buf, "%s\n", value);
}

static struct kobj_attribute kobj_attr_dgpu_hwid_name  = __LEGION_WMI_ATTR_RO(dgpu_hwid,dgpu_hwid);

__LEGION_WMI_KOBJ_ATTR_RO(temp,GetGPUTemp,gpu_temp);
__LEGION_WMI_KOBJ_ATTR_RO(pow,GetGPUPow,gpu_pow);
__LEGION_WMI_KOBJ_ATTR_RO(oc_pow,GetGPUOCPow,gpu_oc_pow);
__LEGION_WMI_KOBJ_ATTR_RO(oc_type,GetGPUOCType,gpu_oc_type);
__LEGION_WMI_KOBJ_ATTR_WO(notify_dgpu_satus,NotifyDGPUStatus,gpu_notify_dgpu_satus,StateGPUONOFF);
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,"Others GPU related features",display_gpu);

static struct attribute *legion_sysfs_gpu_attributes[] = {
	&kobj_attr_gpu_temp.attr,
	&kobj_attr_gpu_pow.attr,
	&kobj_attr_gpu_oc_pow.attr,
	&kobj_attr_gpu_oc_type.attr,
	&kobj_attr_gpu_notify_dgpu_satus.attr,
	&kobj_attr_dgpu_hwid_name.attr,
	&kobj_attr_display_gpu.attr,
	NULL
};
////////////////// GPU related //////////////////////////////////////////

////////////////// CPU related //////////////////////////////////////////

__LEGION_WMI_KOBJ_ATTR_RO(temp,GetCPUTemp,cpu_temp);
__LEGION_WMI_KOBJ_ATTR_RO(freq,GetCpuFrequency,cpu_freq);
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,"Others CPU related features",display_cpu);

static struct attribute *legion_sysfs_cpu_attributes[] = {
	&kobj_attr_cpu_temp.attr,
	&kobj_attr_cpu_freq.attr,
	&kobj_attr_display_cpu.attr,
	NULL
};

////////////////// CPU related //////////////////////////////////////////


////////////////// Macrokey related //////////////////////////////////////
__LEGION_WMI_KOBJ_ATTR_RO(scancode,GetMacrokeyScancode,macro_key_scancode);
__LEGION_WMI_KOBJ_ATTR_RO(count,GetMacrokeyCount,macro_key_count);
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,"Macrokey related features",display_macro_key);

static struct attribute *legion_sysfs_macro_key_attributes[] = {
	&kobj_attr_macro_key_scancode.attr,
	&kobj_attr_macro_key_count.attr,
	&kobj_attr_display_macro_key.attr,
	NULL
};
////////////////// Macrokey related //////////////////////////////////////


////////////////// Other related   //////////////////////////////////////
static u32 StateOtherONOFF[] = {
		0, //Off
		1  //On
};

__LEGION_WMI_KOBJ_ATTR_RO(get_ir_temp,GetPowerChargeMode,get_ir_temp);
__LEGION_WMI_KOBJ_ATTR_RO(get_power_charge_mode,GetPowerChargeMode,get_power_charge_mode);
__LEGION_WMI_KOBJ_ATTR_RO(get_product_info,GetProductInfo,get_product_info);
__LEGION_WMI_KOBJ_ATTR_RO(get_version,GetVersion,get_version);
__LEGION_WMI_KOBJ_ATTR_RO(get_trigger_temperature_value,GetTriggerTemperatureValue,get_trigger_temperature_value);
__LEGION_WMI_KOBJ_ATTR_RO(get_keyboard_feature_list,GetKeyboardfeaturelist,get_keyboard_feature_list);
__LEGION_WMI_KOBJ_ATTR_RO(get_memory_oc_info,GetMemoryOCInfo,get_memory_oc_info);

__LEGION_WMI_KOBJ_ATTR_WO(set_light_control_owner,SetLightControlOwner,set_light_control_owner,StateOtherONOFF);
__LEGION_WMI_KOBJ_ATTR_WO(set_dds_control_owner,SetDDSControlOwner,set_dds_control_owner,StateOtherONOFF);

__LEGION_WMI_KOBJ_ATTR_RW(get_intelligent_sub_mode,GetIntelligentSubMode,SetIntelligentSubMode,get_intelligent_sub_mode,StateOtherONOFF);
__LEGION_WMI_KOBJ_ATTR_RO(get_hardware_info_support_version,GetHardwareInfoSupportVersion,get_hardware_info_support_version);
__LEGION_WMI_KOBJ_ATTR_RO(get_learning_profile_count,GetLearningProfileCount,get_learning_profile_count);
__LEGION_WMI_KOBJ_ATTR_RO(is_ac_fit_for_oc,IsACFitForOC,is_ac_fit_for_oc);
__LEGION_WMI_KOBJ_ATTR_RO(get_thermal_mode,GetThermalMode,get_thermal_mode);
__LEGION_WMI_KOBJ_ATTR_RO(is_changed_ylog,IsChangedYLog,is_changed_ylog);
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_other,"No related features",display_other);


static struct attribute *legion_sysfs_other_attributes[] = {
	&kobj_attr_get_ir_temp.attr,
	&kobj_attr_get_power_charge_mode.attr,
	&kobj_attr_get_product_info.attr,
	&kobj_attr_get_version.attr,
	&kobj_attr_get_trigger_temperature_value.attr,
	&kobj_attr_get_keyboard_feature_list.attr,
	&kobj_attr_get_memory_oc_info.attr,
	&kobj_attr_get_intelligent_sub_mode.attr,
	&kobj_attr_set_light_control_owner.attr,
	&kobj_attr_set_dds_control_owner.attr,
	&kobj_attr_get_hardware_info_support_version.attr,
	&kobj_attr_get_learning_profile_count.attr,
	&kobj_attr_is_ac_fit_for_oc.attr,
	&kobj_attr_get_thermal_mode.attr,
	&kobj_attr_is_changed_ylog.attr,
	&kobj_attr_display_other.attr,
	NULL
};

////////////////// Other related   //////////////////////////////////////


static const struct attribute_group legion_sysfs_gz_group[] = {
{
	.name = "smart_fan",
    .attrs = legion_sysfs_smart_fan_attributes
},
{
	.name = "igpu_mode",
	.attrs = legion_sysfs_igpu_mode_attributes
},
{
	.name = "disable_win_key",
	.attrs = legion_sysfs_disable_win_key_attributes
},
{
	.name = "disable_tp",
	.attrs = legion_sysfs_disable_tp_attributes
},
{
	.name = "gsync",
	.attrs = legion_sysfs_gsync_attributes
},
{
	.name = "over_drive",
	.attrs = legion_sysfs_over_drive_attributes
},
{
	.name = "fans",
	.attrs = legion_sysfs_fans_attributes
},
{
	.name = "gpu",
	.attrs = legion_sysfs_gpu_attributes
},
{
	.name = "cpu",
	.attrs = legion_sysfs_cpu_attributes
},
{
	.name = "macro_key",
	.attrs = legion_sysfs_macro_key_attributes
},
{
	.name = "other",
	.attrs = legion_sysfs_other_attributes
},
{}
};
/**
 * lwmi_om_fw_attr_add() - Register all firmware_attributes_class members
 * @priv: The Other Mode driver data.
 *
 * Return: Either 0, or an error code.
 */
static int legion_wmi_gz_fw_attr_add(struct lenovo_wmi_gz_priv *priv)
{
	int err = 0;
	unsigned int i   = 0;
	priv->ida_id = ida_alloc(&legion_wmi_gz_ida, GFP_KERNEL);
	if (priv->ida_id < 0)
		return priv->ida_id;

	priv->fw_attr_dev = device_create(&legion_firmware_attributes_class, NULL,
					  MKDEV(0, 0), priv, "%s-%u",
					  LEGION_WMI_GZ_FW_ATTR_BASE_PATH,
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

	for (i = 0; i < ARRAY_SIZE(legion_sysfs_gz_group) - 1; i++) {
		err = sysfs_create_group(&priv->fw_attr_kset->kobj,
				&legion_sysfs_gz_group[i]);
		if (err)
			goto err_remove_groups;
	}

	return 0;
err_remove_groups:
	while (i--)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_gz_group[i]);
	kset_unregister(priv->fw_attr_kset);
err_unregister_fw_attr_dev:
	device_unregister(priv->fw_attr_dev);
err_free_ida:
	ida_free(&legion_wmi_gz_ida, priv->ida_id);
	return err;
}

/**
 * legion_wmi_gz_fw_attr_remove() - Unregister all capability data attribute groups
 * @priv: the lenovo-wmi-other driver data.
 */
static void legion_wmi_gz_fw_attr_remove(struct lenovo_wmi_gz_priv *priv)
{
	/* Remove sysfs group first */
	for (unsigned int i = 0; i < ARRAY_SIZE(legion_sysfs_gz_group) - 1; i++)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_gz_group[i]);

	/* Unregister kset before device since kset is child of device's kobj */
	kset_unregister(priv->fw_attr_kset);
	
	/* Device unregister will clean up remaining kobject hierarchy */
	device_unregister(priv->fw_attr_dev);

	ida_free(&legion_wmi_gz_ida, priv->ida_id);
}

int  legion_wmi_gamezone_sysfs_init(struct lenovo_wmi_gz_priv *priv) {
	return legion_wmi_gz_fw_attr_add(priv);
}

void legion_wmi_gamezone_sysfs_exit(struct lenovo_wmi_gz_priv *priv) {
    legion_wmi_gz_fw_attr_remove(priv);
}

