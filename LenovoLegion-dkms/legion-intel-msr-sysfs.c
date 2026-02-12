// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-common.h"
#include "legion-intel-msr.h"

#include <linux/device.h>
#include <linux/export.h>
#include <linux/gfp_types.h>
#include <linux/idr.h>

#define LEGION_INTEL_MSR_BASE_PATH "intel-msr"

static DEFINE_IDA(legion_intel_msr_sysfs_ida);

const struct class legion_intel_msr_class = {
	.name = "legion-intel-msr"
};

/*
 * Sysfs attribute show/store functions
 */
static ssize_t cpu_offset_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    int mv_offset,ret;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = legion_intel_msr_offset_read_show(&priv->intel_msr_private,PLANE_CPU,&mv_offset);
    if(ret)
    	return ret;

    return sprintf(buf, "%d\n", mv_offset);
}

static ssize_t cpu_offset_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
{
    int ret,cpu_offset;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = kstrtoint(buf, 10, &cpu_offset);
    if (ret < 0)
        return ret;

    legion_intel_msr_apply_voltage_offset(&priv->intel_msr_private,PLANE_CPU, cpu_offset);

    return count;
}

static ssize_t cache_offset_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    int mv_offset,ret;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = legion_intel_msr_offset_read_show(&priv->intel_msr_private,PLANE_CACHE,&mv_offset);
    if(ret)
    	return ret;

    return sprintf(buf, "%d\n", mv_offset);
}

static ssize_t cache_offset_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
{
    int ret,cache_offset;
	struct legion_data *priv = dev_get_drvdata(dev);


	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}


    ret = kstrtoint(buf, 10, &cache_offset);
    if (ret < 0)
        return ret;

    legion_intel_msr_apply_voltage_offset(&priv->intel_msr_private,PLANE_CACHE, cache_offset);
    return count;
}

static ssize_t gpu_offset_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    int mv_offset,ret;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = legion_intel_msr_offset_read_show(&priv->intel_msr_private,PLANE_GPU,&mv_offset);
    if(ret)
    	return ret;

    return sprintf(buf, "%d\n", mv_offset);
}

static ssize_t gpu_offset_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
{
    int ret,gpu_offset;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}


	ret = kstrtoint(buf, 10, &gpu_offset);
    if (ret < 0)
        return ret;

    legion_intel_msr_apply_voltage_offset(&priv->intel_msr_private,PLANE_GPU, gpu_offset);
    return count;
}

static ssize_t uncore_offset_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    int mv_offset,ret;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = legion_intel_msr_offset_read_show(&priv->intel_msr_private,PLANE_UNCORE,&mv_offset);
    if(ret)
    	return ret;

    return sprintf(buf, "%d\n", mv_offset);
}

static ssize_t uncore_offset_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
{
    int ret,uncore_offset;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = kstrtoint(buf, 10, &uncore_offset);
    if (ret < 0)
        return ret;

    legion_intel_msr_apply_voltage_offset(&priv->intel_msr_private,PLANE_UNCORE, uncore_offset);
    return count;
}

static ssize_t analogio_offset_show(struct device *dev,struct device_attribute *attr, char *buf)
{
    int mv_offset,ret;
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = legion_intel_msr_offset_read_show(&priv->intel_msr_private,PLANE_ANALOGIO,&mv_offset);
    if(ret)
    	return ret;

    return sprintf(buf, "%d\n", mv_offset);
}

static ssize_t analogio_offset_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count)
{
	struct legion_data *priv = dev_get_drvdata(dev);
    int ret,analogio_offset;


	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    ret = kstrtoint(buf, 10, &analogio_offset);
    if (ret < 0)
        return ret;

    legion_intel_msr_apply_voltage_offset(&priv->intel_msr_private,PLANE_ANALOGIO, analogio_offset);
    return count;
}



static ssize_t cpu_max_undervolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_CPU].max_undervolt_uv);
}

static ssize_t cpu_max_overvolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_CPU].max_overvolt_uv);;
}




static ssize_t cache_max_undervolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_CACHE].max_undervolt_uv);;
}


/*
 * Show voltage offset limits for all planes
 */
static ssize_t cache_max_overvolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_CACHE].max_undervolt_uv);;
}



static ssize_t gpu_max_undervolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_GPU].max_undervolt_uv);;
}


/*
 * Show voltage offset limits for all planes
 */
static ssize_t gpu_max_overvolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_GPU].max_undervolt_uv);;
}




static ssize_t uncore_max_undervolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_UNCORE].max_undervolt_uv);;
}


/*
 * Show voltage offset limits for all planes
 */
static ssize_t uncore_max_overvolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_UNCORE].max_undervolt_uv);;
}

/*
 * Show which voltage planes support write operations
 */
static ssize_t cpu_offset_ctrl_supported_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_CPU].write_supported);
}

static ssize_t cache_offset_ctrl_supported_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_CACHE].write_supported);
}

static ssize_t gpu_offset_ctrl_supported_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_GPU].write_supported);
}

static ssize_t uncore_offset_ctrl_supported_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_UNCORE].write_supported);
}

static ssize_t analogio_offset_ctrl_supported_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_ANALOGIO].write_supported);
}

static ssize_t analogio_max_undervolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_ANALOGIO].max_undervolt_uv);;
}


/*
 * Show voltage offset limits for all planes
 */
static ssize_t analogio_max_overvolt_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct legion_data *priv = dev_get_drvdata(dev);

	/* Critical: Prevent NULL pointer dereference and system freeze */
	if (!priv) {
		return -ENODEV;
	}

    return sprintf(buf, "%d\n", priv->intel_msr_private.plane_limits[PLANE_ANALOGIO].max_undervolt_uv);;
}

static DEVICE_ATTR_RW(cpu_offset);
static DEVICE_ATTR_RW(cache_offset);
static DEVICE_ATTR_RW(gpu_offset);
static DEVICE_ATTR_RW(uncore_offset);
static DEVICE_ATTR_RW(analogio_offset);



static DEVICE_ATTR_RO(cpu_max_undervolt);
static DEVICE_ATTR_RO(cpu_max_overvolt);

static DEVICE_ATTR_RO(cache_max_undervolt);
static DEVICE_ATTR_RO(cache_max_overvolt);

static DEVICE_ATTR_RO(gpu_max_undervolt);
static DEVICE_ATTR_RO(gpu_max_overvolt);

static DEVICE_ATTR_RO(uncore_max_undervolt);
static DEVICE_ATTR_RO(uncore_max_overvolt);

static DEVICE_ATTR_RO(analogio_max_undervolt);
static DEVICE_ATTR_RO(analogio_max_overvolt);

static DEVICE_ATTR_RO(cpu_offset_ctrl_supported);
static DEVICE_ATTR_RO(cache_offset_ctrl_supported);
static DEVICE_ATTR_RO(gpu_offset_ctrl_supported);
static DEVICE_ATTR_RO(uncore_offset_ctrl_supported);
static DEVICE_ATTR_RO(analogio_offset_ctrl_supported);


static struct attribute *legion_intel_msr_sysfs_attributes[]  = {
	    &dev_attr_cpu_offset.attr,
	    &dev_attr_cache_offset.attr,
	    &dev_attr_gpu_offset.attr,
	    &dev_attr_uncore_offset.attr,
	    &dev_attr_analogio_offset.attr,

	    &dev_attr_cpu_max_undervolt.attr,
	    &dev_attr_cpu_max_overvolt.attr,
	    &dev_attr_cache_max_undervolt.attr,
	    &dev_attr_cache_max_overvolt.attr,
	    &dev_attr_gpu_max_undervolt.attr,
	    &dev_attr_gpu_max_overvolt.attr,
	    &dev_attr_uncore_max_undervolt.attr,
	    &dev_attr_uncore_max_overvolt.attr,
	    &dev_attr_analogio_max_undervolt.attr,
	    &dev_attr_analogio_max_overvolt.attr,

	    &dev_attr_cpu_offset_ctrl_supported.attr,
	    &dev_attr_cache_offset_ctrl_supported.attr,
	    &dev_attr_gpu_offset_ctrl_supported.attr,
	    &dev_attr_uncore_offset_ctrl_supported.attr,
	    &dev_attr_analogio_offset_ctrl_supported.attr,

		NULL
};

static const struct attribute_group legion_intel_msr_attributes_group = {
	    .attrs = legion_intel_msr_sysfs_attributes
};



int  legion_intel_msr_sysfs_init(struct device *parent) {

	int ret = 0;
	struct legion_data* data = dev_get_drvdata(parent);

	if (!data)
	    return -ENODEV;

	ret = legion_intel_msr_init(&data->intel_msr_private);
    if (ret)
        return ret;

    ret = class_register(&legion_intel_msr_class);
    if (ret) {
    	return ret;
    }

    ret = data->intel_msr_sysfs_private.ida_id = ida_alloc(&legion_intel_msr_sysfs_ida, GFP_KERNEL);
	if (data->intel_msr_sysfs_private.ida_id < 0)
		goto err_unregister_class;

	data->intel_msr_sysfs_private.dev = device_create(&legion_intel_msr_class, NULL,
					  MKDEV(0, 0), data, "%s-%u",
					  LEGION_INTEL_MSR_BASE_PATH,
					  data->intel_msr_sysfs_private.ida_id);
	if (IS_ERR(data->intel_msr_sysfs_private.dev)) {
		ret = PTR_ERR(data->intel_msr_sysfs_private.dev);
		goto err_free_ida;
	}


	ret = device_add_group(data->intel_msr_sysfs_private.dev,&legion_intel_msr_attributes_group);
    if (ret) {
    	goto err_unregister_dev;
    }

	return 0;

err_unregister_dev:
	device_unregister(data->intel_msr_sysfs_private.dev);
err_free_ida:
	ida_free(&legion_intel_msr_sysfs_ida, data->intel_msr_sysfs_private.ida_id);
err_unregister_class:
	class_unregister(&legion_intel_msr_class);
	return ret;
}


void legion_intel_msr_sysfs_exit(struct device *parent) {

	struct legion_data* data = dev_get_drvdata(parent);

	if (!data)
		return;

	/* Remove sysfs group first */
	device_remove_group(data->intel_msr_sysfs_private.dev,&legion_intel_msr_attributes_group);

	/* Device unregister will clean up remaining kobject hierarchy
	 * and wait for all outstanding sysfs operations to complete */
	device_unregister(data->intel_msr_sysfs_private.dev);

	/* Now safe to destroy mutex - all callbacks have completed */
	legion_intel_msr_exit(&data->intel_msr_private);

	ida_free(&legion_intel_msr_sysfs_ida, data->intel_msr_sysfs_private.ida_id);

	class_unregister(&legion_intel_msr_class);
}
