// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-machine-information.h"

#include <linux/dmi.h>


void read_dmi_machine_information(struct dmi_machine_information* machine_info) {

	machine_info->bios_date = dmi_get_system_info(DMI_BIOS_DATE);
	machine_info->bios_version = dmi_get_system_info(DMI_BIOS_VERSION);
	machine_info->bios_vendor = dmi_get_system_info(DMI_BIOS_VENDOR);
	machine_info->bios_release = dmi_get_system_info(DMI_BIOS_RELEASE);

	machine_info->product_name = dmi_get_system_info(DMI_PRODUCT_NAME);
	machine_info->product_family= dmi_get_system_info(DMI_PRODUCT_FAMILY);
	machine_info->product_serial = dmi_get_system_info(DMI_PRODUCT_SERIAL);
	machine_info->product_sku = dmi_get_system_info(DMI_PRODUCT_SKU);
	machine_info->product_uuid = dmi_get_system_info(DMI_PRODUCT_UUID);
	machine_info->product_version = dmi_get_system_info(DMI_PRODUCT_VERSION);

	machine_info->board_name = dmi_get_system_info(DMI_BOARD_NAME);
	machine_info->board_vendor = dmi_get_system_info(DMI_BOARD_VENDOR);
	machine_info->board_version = dmi_get_system_info(DMI_BOARD_VERSION);
	machine_info->board_serial = dmi_get_system_info(DMI_BOARD_SERIAL);
	machine_info->board_asset_tag = dmi_get_system_info(DMI_BOARD_ASSET_TAG);

	machine_info->chassis_asset_tag = dmi_get_system_info(DMI_CHASSIS_ASSET_TAG);
	machine_info->chassis_serial= dmi_get_system_info(DMI_CHASSIS_SERIAL);
	machine_info->chassis_type= dmi_get_system_info(DMI_CHASSIS_TYPE);
	machine_info->chassis_vendor= dmi_get_system_info(DMI_CHASSIS_VENDOR);
	machine_info->chassis_version= dmi_get_system_info(DMI_CHASSIS_VERSION);

	machine_info->sys_vendor = dmi_get_system_info(DMI_SYS_VENDOR);
	machine_info->ec_firmware_release = dmi_get_system_info(DMI_EC_FIRMWARE_RELEASE);
}

void init_dmi_machine_information(struct dmi_machine_information* machine_info){
	machine_info->bios_date = "";
	machine_info->bios_version = "";
	machine_info->bios_vendor = "";
	machine_info->bios_release = "";

	machine_info->product_name = "";
	machine_info->product_family= "";
	machine_info->product_serial = "";
	machine_info->product_sku = "";
	machine_info->product_uuid = "";
	machine_info->product_version = "";

	machine_info->board_name = "";
	machine_info->board_vendor = "";
	machine_info->board_version = "";
	machine_info->board_serial = "";
	machine_info->board_asset_tag = "";

	machine_info->chassis_asset_tag = "";
	machine_info->chassis_serial= "";
	machine_info->chassis_type= "";
	machine_info->chassis_vendor= "";
	machine_info->chassis_version= "";

	machine_info->sys_vendor = "";
	machine_info->ec_firmware_release = "";
}

void print_dmi_machine_information(struct device* dev, const struct dmi_machine_information* machine_info) {
	dev_info(dev, "\tDMI Machine Information start\n");
	dev_info(dev, "\t\t bios_vendor:%s\n",machine_info->bios_vendor);
	dev_info(dev, "\t\t bios_version:%s\n",machine_info->bios_version);
	dev_info(dev, "\t\t bios_date:%s\n",machine_info->bios_date);
	dev_info(dev, "\t\t bios_release:%s\n",machine_info->bios_release);

	dev_info(dev, "\t\t product_name:%s\n",machine_info->product_name);
	dev_info(dev, "\t\t product_family:%s\n",machine_info->product_family);
	dev_info(dev, "\t\t product_version:%s\n",machine_info->product_version);
	dev_info(dev, "\t\t product_serial:%s\n",machine_info->product_serial);
	dev_info(dev, "\t\t product_sku:%s\n",machine_info->product_sku);
	dev_info(dev, "\t\t product_uuid:%s\n",machine_info->product_uuid);

	dev_info(dev, "\t\t board_name:%s\n",machine_info->board_name);
	dev_info(dev, "\t\t board_vendor:%s\n",machine_info->board_vendor);
	dev_info(dev, "\t\t board_version:%s\n",machine_info->board_version);
	dev_info(dev, "\t\t board_serial:%s\n",machine_info->board_serial);
	dev_info(dev, "\t\t board_asset_tag:%s\n",machine_info->board_asset_tag);

	dev_info(dev, "\t\t chassis_vendor:%s\n",machine_info->chassis_vendor);
	dev_info(dev, "\t\t chassis_version:%s\n",machine_info->chassis_version);
	dev_info(dev, "\t\t chassis_type:%s\n",machine_info->chassis_type);
	dev_info(dev, "\t\t chassis_serial:%s\n",machine_info->chassis_serial);
	dev_info(dev, "\t\t chassis_asset_tag:%s\n",machine_info->chassis_asset_tag);

	dev_info(dev, "\t\t sys_vendor:%s\n",machine_info->sys_vendor);
	dev_info(dev, "\t\t ec_firmware_release:%s\n",machine_info->ec_firmware_release);
	dev_info(dev, "\tDMI Machine Information end\n");
}

void read_machine_information(struct machine_information* machine_info) {
	read_dmi_machine_information(&machine_info->dmi_info);
}

void init_machine_information(struct machine_information* machine_info){
	init_dmi_machine_information(&machine_info->dmi_info);
}

void print_machine_information(struct device* dev, const struct machine_information* machine_info){
	print_dmi_machine_information(dev,&machine_info->dmi_info);
}


