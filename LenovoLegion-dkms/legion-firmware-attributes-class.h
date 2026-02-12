// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_FIRMWARE_ATTRIBUTES_CLASS_H_
#define LEGION_FIRMWARE_ATTRIBUTES_CLASS_H_

#include <linux/device/class.h>

extern const struct class legion_firmware_attributes_class;

int legion_firmware_attributes_class_init(void);
void legion_firmware_attributes_class_exit(void);

#endif /* LEGION_FIRMWARE_ATTRIBUTES_CLASS_H_ */
