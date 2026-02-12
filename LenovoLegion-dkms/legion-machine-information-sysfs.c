// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-common.h"

#include <linux/device.h>


#define LEGION_MACHINE_INF_BASE_PATH "machine-information"

static DEFINE_IDA(legion_machine_inf_sysfs_ida);

const struct class legion_dmi_class = {
	.name = "legion-dmi",
};

#define __LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(_name) 												 \
static ssize_t _name##_show(struct device *dev,struct device_attribute *attr, char *buf) 				 \
{																												 \
	struct legion_data *priv = dev_get_drvdata(dev);															 \
																												 \
	/* Critical: Prevent NULL pointer dereference and system freeze */   										 \
	if (!priv) {																								 \
		return -ENODEV;																							 \
	}																											 \
																												 \
	/* Return cached value instead of calling WMI (prevents deadlock/freeze) */									 \
	return sysfs_emit(buf, "%s\n", priv->machine_info.dmi_info._name);												 	 \
}																												 \
static DEVICE_ATTR_RO(_name);


__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(bios_vendor);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(bios_version);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(bios_date);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(bios_release);

__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(product_name);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(product_version);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(product_serial);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(product_uuid);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(product_sku);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(product_family);

__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(board_vendor);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(board_name);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(board_version);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(board_serial);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(board_asset_tag);



__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(chassis_vendor);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(chassis_type);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(chassis_version);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(chassis_serial);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(chassis_asset_tag);

__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(sys_vendor);
__LEGION_MACHINE_INF_SYSFS_DEV_ATTR_RO_CACHED(ec_firmware_release);


static struct attribute *legion_machine_inf_sysfs_attributes[]  = {
	    &dev_attr_bios_vendor.attr,
		&dev_attr_bios_version.attr,
		&dev_attr_bios_date.attr,
		&dev_attr_bios_release.attr,

		&dev_attr_product_name.attr,
		&dev_attr_product_version.attr,
		&dev_attr_product_serial.attr,
		&dev_attr_product_uuid.attr,
		&dev_attr_product_sku.attr,
		&dev_attr_product_family.attr,

		&dev_attr_board_vendor.attr,
		&dev_attr_board_name.attr,
		&dev_attr_board_version.attr,
		&dev_attr_board_serial.attr,
		&dev_attr_board_asset_tag.attr,

		&dev_attr_chassis_vendor.attr,
		&dev_attr_chassis_type.attr,
		&dev_attr_chassis_version .attr,
		&dev_attr_chassis_serial.attr,
		&dev_attr_chassis_asset_tag.attr,

		&dev_attr_sys_vendor.attr,
		&dev_attr_ec_firmware_release.attr,
		NULL
};

static const struct attribute_group legion_machine_inf_sysfs_attributes_group = {
    .attrs = legion_machine_inf_sysfs_attributes,
};

int  machine_information_sysfs_init(struct legion_data* data)
{
	int ret = 0;

    ret = class_register(&legion_dmi_class);
    if (ret) {
        return ret;
    }

	ret = data->machine_info_sysfs_private.ida_id = ida_alloc(&legion_machine_inf_sysfs_ida, GFP_KERNEL);
	if (data->machine_info_sysfs_private.ida_id < 0)
		goto err_unregister_class;

	data->machine_info_sysfs_private.dev = device_create(&legion_dmi_class, NULL,
					  MKDEV(0, 0), data, "%s-%u",
					  LEGION_MACHINE_INF_BASE_PATH,
					  data->machine_info_sysfs_private.ida_id);
	if (IS_ERR(data->machine_info_sysfs_private.dev)) {
		ret = PTR_ERR(data->machine_info_sysfs_private.dev);
		goto err_free_ida;
	}


	ret = device_add_group(data->machine_info_sysfs_private.dev,&legion_machine_inf_sysfs_attributes_group);
    if (ret) {
    	goto err_unregister_dev;
    }


	return 0;

err_unregister_dev:
	device_unregister(data->machine_info_sysfs_private.dev);
err_free_ida:
	ida_free(&legion_machine_inf_sysfs_ida, data->machine_info_sysfs_private.ida_id);
err_unregister_class:
	class_unregister(&legion_dmi_class);
	return ret;
}

void machine_information_sysfs_exit(struct legion_data* data) {
	if (!data)
		return;

	/* Remove sysfs group first */
	device_remove_group(data->machine_info_sysfs_private.dev,&legion_machine_inf_sysfs_attributes_group);


	/* Device unregister will clean up remaining kobject hierarchy */
	device_unregister(data->machine_info_sysfs_private.dev);

	ida_free(&legion_machine_inf_sysfs_ida, data->machine_info_sysfs_private.ida_id);

	class_unregister(&legion_dmi_class);
}
