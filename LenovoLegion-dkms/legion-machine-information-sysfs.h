// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_MACHINE_INFORMATION_SYSFS_H_
#define LEGION_MACHINE_INFORMATION_SYSFS_H_

struct machine_information_sysfs_private {
	int 	ida_id;
	struct 	device *dev;
};

struct legion_data;
int  machine_information_sysfs_init(struct legion_data* data);
void machine_information_sysfs_exit(struct legion_data* data);


#endif /* LEGION_MACHINE_INFORMATION_SYSFS_H_ */
