// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-firmware-attributes-class.h"

#include <linux/module.h>

const struct class legion_firmware_attributes_class = {
	.name = "legion-firmware-attributes",
};


static  int fw_attributes_class_init(void)
{
	return class_register(&legion_firmware_attributes_class);
}

static void fw_attributes_class_exit(void)
{
	class_unregister(&legion_firmware_attributes_class);
}


int legion_firmware_attributes_class_init(void) {
	return fw_attributes_class_init();
}

void legion_firmware_attributes_class_exit(void) {
	fw_attributes_class_exit();
}
