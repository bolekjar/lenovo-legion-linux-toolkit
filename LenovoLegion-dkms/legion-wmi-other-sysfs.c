// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-common.h"
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
#include <linux/export.h>
#include <linux/gfp_types.h>
#include <linux/idr.h>
#include <linux/wmi.h>

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


static ssize_t legion_wmi_other_cap01_attr_show_no_steps_val(struct legion_wmi_cd01_list *cd01_list,enum attribute_property prop,const u32 attribute_id_raw,enum thermal_mode modes[THERMAL_MODES_SIZE],char *buf,ssize_t* buf_len) {
	struct legion_wmi_capdata01 capdata;
	ssize_t 					len = 0;

	modes[THERMAL_MODES_SIZE - 1] = LEGION_WMI_GZ_THERMAL_MODE_END;
	for(int i = 0;(i < THERMAL_MODES_SIZE) && (modes[i] != LEGION_WMI_GZ_THERMAL_MODE_END);i++){
		int ret 	= 0;
		int value 	= 0;

		len += sysfs_emit_at(buf,len,"%d=",modes[i]);

		ret = legion_wmi_cd01_get_data(cd01_list,attribute_id_raw | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, modes[i]), &capdata);
		if (ret) {
			if(modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)
				len += sysfs_emit_at(buf,len,",");
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

		len += sysfs_emit_at(buf,len,"%d",value);

    	if(modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)
			len += sysfs_emit_at(buf,len,",");

	}
	(*buf_len) += len;

	return 0;
}

static ssize_t legion_wmi_other_cap00_show(struct legion_wmi_cd00_list *cd00_list,enum attribute_property prop,const u32 attribute_id_raw,char *buf,ssize_t* buf_len) {
	struct legion_wmi_capdata00 capdata;
	ssize_t 					len = 0;
	int ret 	= 0;
	int value 	= 0;

	ret = legion_wmi_cd00_get_data(cd00_list,attribute_id_raw, &capdata);
	if (ret) {
		if(ret == -EINVAL)
		{
			len += sysfs_emit_at(buf,len,"%d",value);
			(*buf_len) += len;
			return 0;
		}
		else
		{
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

	len += sysfs_emit_at(buf,len,"%d",value);
	(*buf_len) += len;
	return 0;
}

static ssize_t legion_wmi_other_cap01_attr_show_steps_val(struct legion_wmi_dd_list *dd_list,const u32 attribute_id_raw,enum thermal_mode modes[THERMAL_MODES_SIZE],char *buf,ssize_t* buf_len) {
	struct legion_wmi_ddata     lddata[48] = {{0,0}};
	ssize_t 					len 	   = 0;

	modes[THERMAL_MODES_SIZE - 1] = LEGION_WMI_GZ_THERMAL_MODE_END;
	for(int i = 0;(i < THERMAL_MODES_SIZE) && (modes[i] != LEGION_WMI_GZ_THERMAL_MODE_END);i++){
		int ret = 0;

		len += sysfs_emit_at(buf,len,"%d=",modes[i]);

		ret = legion_wmi_dd_get_data(dd_list,attribute_id_raw | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, modes[i]), lddata,sizeof(lddata)/sizeof(lddata[0]) - 1);
		if (ret) {
			if(modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)
				len += sysfs_emit_at(buf,len,",");
			continue;
		}

		for(int i = 0;i < (sizeof(lddata)/sizeof(lddata[0])) && lddata[i].id != 0;++i)
		{
			len += sysfs_emit_at(buf,len,"%d",lddata[i].value);

			if(lddata[i + 1].id != 0)
				len += sysfs_emit_at(buf,len,"|");
		}

    	if(modes[i + 1] != LEGION_WMI_GZ_THERMAL_MODE_END)
			len += sysfs_emit_at(buf,len,",");
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
static ssize_t legion_wmi_other_cap01_attr_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf, enum attribute_property prop,u32 attribute_id)
{
	struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));
	ssize_t len = 	0;
	int 	ret =	0;
	enum thermal_mode modes[THERMAL_MODES_SIZE] = {LEGION_WMI_GZ_THERMAL_MODE_END};

	ret = legion_wmi_other_notifier_call(&modes,LEGION_WMI_GZ_GET_SUPPORTED_THERMAL_MODES);
	if (ret)
		return ret;

	if(prop == STEPS_VAL)
	{
		ret = legion_wmi_other_cap01_attr_show_steps_val(priv->dd_list,attribute_id  & (~LEGION_WMI_MODE_ID_MASK),modes,buf,&len);
		if(ret)
			return ret;
	}
	else
	{
		ret = legion_wmi_other_cap01_attr_show_no_steps_val(priv->cd01_list,prop,attribute_id  & (~LEGION_WMI_MODE_ID_MASK),modes,buf,&len);
		if(ret)
			return ret;
	}

	len += sysfs_emit_at(buf,len,"\n");
	return len;
}


static ssize_t legion_wmi_other_cap00_attr_show(struct kobject *kobj,struct kobj_attribute *kattr, char *buf, enum attribute_property prop,u32 attribute_id)
{
	struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));
	ssize_t len = 	0;
	int 	ret =	0;
	enum thermal_mode modes[THERMAL_MODES_SIZE] = {LEGION_WMI_GZ_THERMAL_MODE_END};

	ret = legion_wmi_other_notifier_call(&modes,LEGION_WMI_GZ_GET_SUPPORTED_THERMAL_MODES);
	if (ret)
		return ret;


	ret = legion_wmi_other_cap00_show(priv->cd00_list,prop,attribute_id,buf,&len);
	if(ret)
		return ret;


	len += sysfs_emit_at(buf,len,"\n");
	return len;
}



/**
 * legion_wmi_attr_current_value_store() - Set the current value of the given attribute
 * @kobj: Pointer to the driver object.
 * @kattr: Pointer to the attribute calling this function.
 * @buf: The buffer to read from, this is parsed to `int` type.
 * @count: Required by sysfs attribute macros, pass in from the callee attr.
 * @tunable_attr: The attribute to be stored.
 *
 * Sets the value of the given attribute when operating under the "custom"
 * smartfan profile. The current smartfan profile is retrieved from the
 * lenovo-wmi-gamezone driver and error is returned if the result is not
 * "custom". This function is intended to be generic so it can be called from
 * any integer attribute's "_store" function. The integer to be sent to the WMI
 * method is range checked and an error code is returned if out of range.
 *
 * If the value is valid and WMI is success, then the sysfs attribute is
 * notified.
 *
 * Return: Either count, or an error code.
 */
static ssize_t legion_wmi_attr_current_value_store(struct kobject *kobj,
					struct kobj_attribute *kattr,
					const char *buf, size_t count,u32 attribute_id)
{
	struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));
	u32 attribute_id_raw = attribute_id  & (~LEGION_WMI_MODE_ID_MASK);

	struct wmi_method_args_32 args;
	struct legion_wmi_capdata01 capdata;
	enum thermal_mode mode;

	u32 value;


	int ret;

	ret = legion_wmi_other_notifier_call(&mode,LEGION_WMI_GZ_GET_THERMAL_MODE);
	if (ret)
		return ret;

	if (mode != LEGION_WMI_GZ_THERMAL_MODE_CUSTOM)
		return -EBUSY;


	ret = legion_wmi_cd01_get_data(priv->cd01_list,  attribute_id_raw | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, LEGION_WMI_GZ_THERMAL_MODE_CUSTOM), &capdata);
	if (ret)
		return ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (capdata.supported == 0)
	{
		return -ENODEV;
	}

	if(capdata.max_value == 0 && capdata.min_value == 0 && capdata.step == 0)
	{
		struct legion_wmi_ddata     lddata[48] = {{0,0}};
		bool contains_value					   = false;

		ret = legion_wmi_dd_get_data(priv->dd_list,attribute_id_raw | FIELD_PREP(LEGION_WMI_MODE_ID_MASK, LEGION_WMI_GZ_THERMAL_MODE_CUSTOM), lddata,sizeof(lddata)/sizeof(lddata[0]) - 1);
		if (ret)
			return ret;

		for(int i = 0;i < (sizeof(lddata)/sizeof(lddata[0])) && lddata[i].id != 0;++i)
		{
			if(lddata[i].value == value) {
				contains_value = true;
				break;
			}
		}
	}
	else
	{
		if (value < capdata.min_value || value > capdata.max_value)
			return -EINVAL;
	}

	args.arg0 = attribute_id_raw;
	args.arg1 = value;

	ret = legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_SET,
				    (unsigned char *)&args, sizeof(args), NULL);
	if (ret)
		return ret;

	return count;
};


static ssize_t legion_wmi_attr_current_value_store_min_max(struct kobject *kobj,
					struct kobj_attribute *kattr,
					const char *buf, size_t count,u32 attribute_id,int min,int max)
{
	struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));
	u32 attribute_id_raw = attribute_id  & (~LEGION_WMI_MODE_ID_MASK);

	struct wmi_method_args_32 args;
	struct legion_wmi_capdata00 capdata = {0,0,0};
	u32 value;

	int ret = legion_wmi_cd00_get_data(priv->cd00_list,  attribute_id_raw, &capdata);
	if (ret && ret != -EINVAL) {
		return ret;
	}

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (capdata.supported == 0)
	{
		return -ENODEV;
	}

	if (value < min || value > max)
		return -EINVAL;

	args.arg0 = attribute_id_raw;
	args.arg1 = value;

	ret = legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_SET,
				    (unsigned char *)&args, sizeof(args), NULL);
	if (ret)
		return ret;

	return count;
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
				       struct kobj_attribute *kattr, char *buf,u32 attribute_id)
{
	struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));
	u32 attribute_id_raw = attribute_id  & (~LEGION_WMI_MODE_ID_MASK);
	struct wmi_method_args_32 args;
	int retval;
	int ret;

	args.arg0 = attribute_id_raw;

	ret = legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_GET,
				    (unsigned char *)&args, sizeof(args),
				    &retval);
	if (ret) {
		return ret;
	}

	return sysfs_emit(buf, "%d\n", retval);
}

/* Attribute property read only */
#define __LEGION_WMI_RO_CAP01(_attrname,_func, _prop_type, _attr_id)                		\
	static ssize_t func_##_func##_show(                             					    \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) 						\
	{                                                                       				\
		return legion_wmi_other_cap01_attr_show(kobj, kattr, buf,_prop_type,_attr_id);      \
	}                                                                       				\
	static struct kobj_attribute kobj_attr_##_func##_prop =               					\
	__LEGION_WMI_ATTR_RO(_attrname, _func)

/* Attribute property read only */
#define __LEGION_WMI_RO_CAP00(_attrname,_func, _prop_type, _attr_id)                		\
	static ssize_t func_##_func##_show(                             					    \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) 						\
	{                                                                       				\
		return legion_wmi_other_cap00_attr_show(kobj, kattr, buf,_prop_type,_attr_id);      \
	}                                                                       				\
	static struct kobj_attribute kobj_attr_##_func##_prop =               					\
	__LEGION_WMI_ATTR_RO(_attrname, _func)

/* Attribute current value read/write */
#define __LEGION_WMI_CURRENT_VALUE(_attrname,_func,_attr_id)                         	\
	static ssize_t func_##_func##_set(                        							\
		struct kobject *kobj, struct kobj_attribute *kattr,            					\
		const char *buf, size_t count)                                 					\
	{                                                                      				\
		return legion_wmi_attr_current_value_store(kobj, kattr, buf, count,      		\
				_attr_id);                   											\
	}                                                                      				\
	static ssize_t func_##_func##_get(                         						    \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) 					\
	{                                                                      				\
		return legion_wmi_attr_current_value_show(kobj, kattr, buf,_attr_id );  		\
	}                                                                      				\
	static struct kobj_attribute kobj_attr_##_func##_prop =        					    \
	__LEGION_WMI_ATTR_RW(_attrname, _func)

/* Attribute current value read/write */
#define __LEGION_WMI_CURRENT_MIN_MAX(_attrname,_func,_attr_id,_min,_max)   				\
	static ssize_t func_##_func##_set(                        							\
		struct kobject *kobj, struct kobj_attribute *kattr,            					\
		const char *buf, size_t count)                                 					\
	{                                                                      				\
		return legion_wmi_attr_current_value_store_min_max(kobj, kattr, buf, count,		\
				_attr_id,_min,_max);                   									\
	}                                                                      				\
	static ssize_t func_##_func##_get(                         						    \
		struct kobject *kobj, struct kobj_attribute *kattr, char *buf) 					\
	{                                                                      				\
		return legion_wmi_attr_current_value_show(kobj, kattr, buf,_attr_id );  		\
	}                                                                      				\
	static struct kobj_attribute kobj_attr_##_func##_prop =        					    \
	__LEGION_WMI_ATTR_RW(_attrname, _func)


#define __LEGION_WMI_OTHER_CAP01(_attrname,_attr_id,_name) \
__LEGION_WMI_RO_CAP01(default_value,_attrname##_default_value,DEFAULT_VAL,_attr_id);\
__LEGION_WMI_RO_CAP01(max_value,_attrname##_max_value,MAX_VAL,_attr_id);\
__LEGION_WMI_RO_CAP01(min_value,_attrname##_min_value,MIN_VAL,_attr_id);\
__LEGION_WMI_RO_CAP01(scalar_increment,_attrname##_scalar_increment,STEP_VAL,_attr_id);\
__LEGION_WMI_RO_CAP01(steps,_attrname##_steps,STEPS_VAL,_attr_id);\
__LEGION_WMI_RO_CAP01(supported,_attrname##_supported,SUPPORTED,_attr_id);\
__LEGION_WMI_CURRENT_VALUE(current_value,_attrname##_current_value,_attr_id);\
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(type,"integer",_attrname##_type);\
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,_name,_attrname##_display_name);\
\
static struct attribute *legion_sysfs_##_attrname##_attributes[] = {\
	&kobj_attr_##_attrname##_default_value_prop.attr,\
	&kobj_attr_##_attrname##_max_value_prop.attr,\
	&kobj_attr_##_attrname##_min_value_prop.attr,\
	&kobj_attr_##_attrname##_scalar_increment_prop.attr,\
	&kobj_attr_##_attrname##_steps_prop.attr,\
	&kobj_attr_##_attrname##_supported_prop.attr,\
	&kobj_attr_##_attrname##_current_value_prop.attr,\
	&kobj_attr_##_attrname##_type.attr,\
	&kobj_attr_##_attrname##_display_name.attr,\
	NULL\
};

#define __LEGION_WMI_OTHER_CAP0(_attrname,_attr_id,_name,_value_type,_min,_max) \
__LEGION_WMI_RO_CAP00(default_value,_attrname##_default_value,DEFAULT_VAL,_attr_id);\
__LEGION_WMI_RO_CAP00(supported,_attrname##_supported,SUPPORTED,_attr_id);\
__LEGION_WMI_CURRENT_MIN_MAX(current_value,_attrname##_current_value,_attr_id,_min,_max);\
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(type,_value_type,_attrname##_type);\
__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,_name,_attrname##_display_name);\
\
static struct attribute *legion_sysfs_##_attrname##_attributes[] = {\
	&kobj_attr_##_attrname##_default_value_prop.attr,\
	&kobj_attr_##_attrname##_supported_prop.attr,\
	&kobj_attr_##_attrname##_current_value_prop.attr,\
	&kobj_attr_##_attrname##_type.attr,\
	&kobj_attr_##_attrname##_display_name.attr,\
	NULL\
};




#define __LEGION_WMI_OTHER_RW_VALUE(_attrname,_func,_attr_id,_possible_states)\
static ssize_t func_##_func##_set(\
	struct kobject *kobj, struct kobj_attribute *kattr,\
	const char *buf, size_t count)\
{\
	int ret,value;\
\
	ret = kstrtouint(buf, 10, &value);\
	if (ret)\
		return ret;\
\
	for (size_t i = 0; i < sizeof(_possible_states)/sizeof(_possible_states[0]);++i)\
	{\
		if(value == _possible_states[i])\
		{\
			struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));\
			struct wmi_method_args_32 args;\
\
			args.arg0 = _attr_id;\
			args.arg1 = value;\
\
			ret = legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_SET,\
						    (unsigned char *)&args, sizeof(args), NULL);\
			if (ret)\
				return ret;\
\
			return count;\
		}\
\
	}\
	return -EINVAL;\
}\
static ssize_t func_##_func##_get(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)\
{\
	struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));\
\
	struct wmi_method_args_32 args;\
	int ret;\
	int retval;\
\
	args.arg0 = _attr_id;\
\
	ret = legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_GET,\
				    (unsigned char *)&args, sizeof(args),\
				    &retval);\
	if (ret) {\
		return ret;\
	}\
\
	return sysfs_emit(buf, "%d\n", retval);\
}\
static struct kobj_attribute kobj_attr_##_func##_prop =        					    \
__LEGION_WMI_ATTR_RW(_attrname, _func)


#define __LEGION_WMI_OTHER_RO_VALUE(_attrname,_func,_attr_id)\
static ssize_t func_##_func##_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)\
{\
	struct legion_wmi_other_priv *priv = dev_get_drvdata(kobj_to_dev(kobj->parent));\
\
	struct wmi_method_args_32 args;\
	int ret;\
	int retval;\
\
	args.arg0 = _attr_id;\
\
	ret = legion_wmi_dev_evaluate_int(priv->wdev, 0x0, LEGION_WMI_OTHER_FEATURE_VALUE_GET,\
				    (unsigned char *)&args, sizeof(args),\
				    &retval);\
	if (ret) {\
		return ret;\
	}\
\
	return sysfs_emit(buf, "%d\n", retval);\
}\
static struct kobj_attribute kobj_attr_##_func##_prop =        					    \
__LEGION_WMI_ATTR_RO(_attrname, _func)


/*
 * CPU CAP01
 */
__LEGION_WMI_OTHER_CAP01(cpu_stp_limit,CPUShortTermPowerLimit,"Set the CPU short term power limit");
__LEGION_WMI_OTHER_CAP01(cpu_ltp_limit,CPULongTermPowerLimit,"Set the CPU long term power limit");
__LEGION_WMI_OTHER_CAP01(cpu_pp_limit,CPUPeakPowerLimit,"Set the CPU peak power limit");
__LEGION_WMI_OTHER_CAP01(cpu_tmp_limit,CPUTemperatureLimit ,"Set the CPU temperature limit");
__LEGION_WMI_OTHER_CAP01(apus_pptp_limit,APUsPPTPowerLimit ,"Set the APUs ppt power limit");
__LEGION_WMI_OTHER_CAP01(cpu_clp_limit,CPUCrossLoadingPowerLimit ,"Set the CPU cross loading power limit");
__LEGION_WMI_OTHER_CAP01(cpu_pl1_tau,CPUPL1Tau,"Set the CPU PL1 Tau limit");


/*
 * GPU CAP01
 */
__LEGION_WMI_OTHER_CAP01(gpu_power_boost,GPUPowerBoost,"Set the GPU power boost");
__LEGION_WMI_OTHER_CAP01(gpu_configurable_tgp,GPUConfigurableTGP,"Set the GPU configurable TGP");
__LEGION_WMI_OTHER_CAP01(gpu_temperature_limit,GPUTemperatureLimit,"Set the GPU temperature limit");
__LEGION_WMI_OTHER_CAP01(gpu_total_onac,GPUTotalProcessingPowerTargetOnAcOffsetFromBaseline ,"Set the GPU total processing power target on AC offset from base line limit");
__LEGION_WMI_OTHER_CAP01(gpu_to_cpu_dynamic_boost,GPUToCPUDynamicBoost ,"Set the GPU To CPU dynamic boost");

/*
 * CAP00
 */
__LEGION_WMI_OTHER_CAP0(igpu_mode,IGPUMode,"IGPU mode","integer",0,2)
__LEGION_WMI_OTHER_CAP0(instant_boot_ac,InstantBootAc,"Instant Boot","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(instant_boot_usb_power_delivery,InstantBootUsbPowerDelivery,"Instant boot USB power delivery","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(fan_full_speed,FanFullSpeed,"Fan Full Speed","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(over_drive,OverDrive,"Over Drive","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(god_mode_fnq_switchable,GodModeFnQSwitchable,"God mode FN+Q switchable","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(nvidia_gpu_dd_switching,NvidiaGPUDynamicDisplaySwitching,"NVIDIDA GPU dynamic switching","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(flip_to_start,FlipToStart,"Flip to start","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(skin_temperature_tracking,AMDSkinTemperatureTracking,"AMD Skin temperature tracking","boolean",0,1)
__LEGION_WMI_OTHER_CAP0(smart_shiftmode,AMDSmartShiftMode,"AMD Smart shift mode","boolean",0,1)

/*
 * Other Other Get
 */
__LEGION_WMI_OTHER_RO_VALUE(gpu_status,gpu_status,GPUStatus);
__LEGION_WMI_OTHER_RO_VALUE(gpu_did_vid,gpu_did_vid,GPUDidVid);
__LEGION_WMI_OTHER_RO_VALUE(igpu_mode_change_status,igpu_mode_change_status,IGPUModeChangeStatus);
__LEGION_WMI_OTHER_RO_VALUE(zone_support_version,zone_support_version,LegionZoneSupportVersion);
__LEGION_WMI_OTHER_RO_VALUE(supported_power_modes,supported_power_modes,SupportedPowerModes);

__LEGION_WMI_KOBJ_ATTR_RO_STATIC_STRING(display_name,"Other related settings",gpu_other_display_name);

static struct attribute *legion_sysfs_others_attributes[] = {
	&kobj_attr_gpu_status_prop.attr,
	&kobj_attr_gpu_did_vid_prop.attr,
	&kobj_attr_igpu_mode_change_status_prop.attr,
	&kobj_attr_zone_support_version_prop.attr,
	&kobj_attr_supported_power_modes_prop.attr,
	&kobj_attr_gpu_other_display_name.attr,
	NULL
};


static const struct attribute_group legion_sysfs_other_group[] = {
		/*
		 * CPU CAP01
		 */
		{
			.name = "cpu_stp_limit",
			.attrs = legion_sysfs_cpu_stp_limit_attributes
		},
		{
			.name = "cpu_ltp_limit",
			.attrs = legion_sysfs_cpu_ltp_limit_attributes
		},
		{
			.name = "cpu_pp_limit",
			.attrs = legion_sysfs_cpu_pp_limit_attributes
		},
		{
			.name = "cpu_clp_limit",
			.attrs = legion_sysfs_cpu_clp_limit_attributes
		},
		{
			.name = "cpu_tmp_limit",
			.attrs = legion_sysfs_cpu_tmp_limit_attributes
		},
		{
			.name = "apus_pptp_limit",
			.attrs = legion_sysfs_apus_pptp_limit_attributes
		},
		{
			.name = "cpu_pl1_tau",
			.attrs = legion_sysfs_cpu_pl1_tau_attributes
		},



		/*
		 * GPU CAP01
		 */
		{
			.name = "gpu_power_boost",
			.attrs = legion_sysfs_gpu_power_boost_attributes
		},
		{
			.name = "gpu_configurable_tgp",
			.attrs = legion_sysfs_gpu_configurable_tgp_attributes
		},
		{
			.name = "gpu_temperature_limit",
			.attrs = legion_sysfs_gpu_temperature_limit_attributes
		},
		{
			.name = "gpu_total_onac",
			.attrs = legion_sysfs_gpu_total_onac_attributes
		},
		{
			.name = "gpu_to_cpu_dynamic_boost",
			.attrs = legion_sysfs_gpu_to_cpu_dynamic_boost_attributes
		},



		/*
		 * GPU CAP00
		 */
		{
			.name = "igpu_mode",
			.attrs = legion_sysfs_igpu_mode_attributes
		},
		{
			.name = "instant_boot_ac",
			.attrs = legion_sysfs_instant_boot_ac_attributes
		},
		{
			.name = "instant_boot_usb_power_delivery",
			.attrs = legion_sysfs_instant_boot_usb_power_delivery_attributes
		},
		{
			.name = "fan_full_speed",
			.attrs = legion_sysfs_fan_full_speed_attributes
		},
		{
			.name = "over_drive",
			.attrs = legion_sysfs_over_drive_attributes
		},
		{
			.name = "god_mode_fnq_switchable",
			.attrs = legion_sysfs_god_mode_fnq_switchable_attributes
		},
		{
			.name = "nvidia_gpu_dd_switching",
			.attrs = legion_sysfs_nvidia_gpu_dd_switching_attributes
		},
		{
			.name = "flip_to_start",
			.attrs = legion_sysfs_flip_to_start_attributes
		},
		{
			.name = "skin_temperature_tracking",
			.attrs = legion_sysfs_skin_temperature_tracking_attributes
		},
		{
			.name = "smart_shiftmode",
			.attrs = legion_sysfs_smart_shiftmode_attributes
		},


		{
			.name = "others",
			.attrs = legion_sysfs_others_attributes
		},
		{}
};

/**
 * lwmi_om_fw_attr_add() - Register all firmware_attributes_class members
 * @priv: The Other Mode driver data.
 *
 * Return: Either 0, or an error code.
 */
static int legion_wmi_other_fw_attr_add(struct legion_wmi_other_priv *priv)
{
	int err = 0;
	unsigned int i   = 0;
	priv->ida_id = ida_alloc(&legion_wmi_other_ida, GFP_KERNEL);
	if (priv->ida_id < 0)
		return priv->ida_id;

	priv->fw_attr_dev = device_create(&legion_firmware_attributes_class, NULL,
					  MKDEV(0, 0), priv, "%s-%u",
					  LEGION_WMI_OTHER_FW_ATTR_BASE_PATH,
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

	for (i = 0; i < ARRAY_SIZE(legion_sysfs_other_group) - 1; i++) {
		err = sysfs_create_group(&priv->fw_attr_kset->kobj,
				&legion_sysfs_other_group[i]);
		if (err)
			goto err_remove_groups;
	}

	return 0;
err_remove_groups:
	while (i--)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_other_group[i]);
	kset_unregister(priv->fw_attr_kset);
err_unregister_fw_attr_dev:
	device_unregister(priv->fw_attr_dev);
err_free_ida:
	ida_free(&legion_wmi_other_ida, priv->ida_id);
	return err;
}

/**
 * legion_wmi_gz_fw_attr_remove() - Unregister all capability data attribute groups
 * @priv: the lenovo-wmi-other driver data.
 */
static void legion_wmi_other_fw_attr_remove(struct legion_wmi_other_priv *priv)
{
	/* Remove sysfs group first */
	for (unsigned int i = 0; i < ARRAY_SIZE(legion_sysfs_other_group) - 1; i++)
		sysfs_remove_group(&priv->fw_attr_kset->kobj,&legion_sysfs_other_group[i]);

	/* Unregister kset before device since kset is child of device's kobj */
	kset_unregister(priv->fw_attr_kset);

	/* Device unregister will clean up remaining kobject hierarchy */
	device_unregister(priv->fw_attr_dev);

	ida_free(&legion_wmi_other_ida, priv->ida_id);
}

int  legion_wmi_other_sysfs_init(struct legion_wmi_other_priv *priv) {
	return legion_wmi_other_fw_attr_add(priv);
}

void legion_wmi_other_sysfs_exit(struct legion_wmi_other_priv *priv) {
    legion_wmi_other_fw_attr_remove(priv);
}
