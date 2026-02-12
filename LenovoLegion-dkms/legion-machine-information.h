// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_MACHINE_INFORMATION_H_
#define LEGION_MACHINE_INFORMATION_H_

#include <linux/device.h>

struct dmi_machine_information {
	/*
	 * BIOS
	 */
	const char * bios_vendor;
	const char * bios_version;
	const char * bios_date;
	const char * bios_release;

	/*
	 * Product
	 */
	const char * product_name;
	const char * product_version;
	const char * product_serial;
	const char * product_uuid;
	const char * product_sku;
	const char * product_family;

	/*
	 * Board
	 */
	const char * board_vendor;
	const char * board_name;
	const char * board_version;
	const char * board_serial;
	const char * board_asset_tag;

	/*
	 * Chassis
	 */
	const char * chassis_vendor;
	const char * chassis_type;
	const char * chassis_version;
	const char * chassis_serial;
	const char * chassis_asset_tag;


	/*
	 * Other
	 */
	const char * sys_vendor;
	const char * ec_firmware_release;
};

struct machine_information
{
	struct dmi_machine_information dmi_info;
};

void init_dmi_machine_information(struct dmi_machine_information* machine_info);
void read_dmi_machine_information(struct dmi_machine_information* machine_info);
void print_dmi_machine_information(struct device* dev, const struct dmi_machine_information* machine_info);

void init_machine_information(struct machine_information* machine_info);
void read_machine_information(struct machine_information* machine_info);
void print_machine_information(struct device* dev, const struct machine_information* machine_info);


#endif /* LEGION_MACHINE_INFORMATION_H_ */
