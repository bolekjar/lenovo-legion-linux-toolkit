// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-common.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)

int legion_data_init(struct legion_data* data) {
	 mutex_init(&data->mutex);
	 init_machine_information(&data->machine_info);

	 data->ec_fw_attr_dev 	= NULL;
	 data->ec_fw_attr_kset 	= NULL;


	 data->hwmon_dev    	= NULL;


	 return 0;
}

void legion_data_exit(struct legion_data* data) {
	mutex_destroy(&data->mutex);
}

#else

int legion_data_init(struct legion_data* data) {
	 mutex_init(&data->mutex);
	 init_machine_information(&data->machine_info);

	 data->ec_fw_attr_dev 	= NULL;
	 data->ec_fw_attr_kset 	= NULL;

	 data->hwmon_dev    = NULL;

	 return 0;
}

void legion_data_exit(struct legion_data* data) {
	mutex_destroy(&data->mutex);
}

#endif
