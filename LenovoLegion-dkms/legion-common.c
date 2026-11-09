// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-common.h"

int legion_data_init(struct legion_data *data)
{
	init_machine_information(&data->machine_info);
	data->hwmon_dev = NULL;

	return 0;
}

void legion_data_exit(struct legion_data *data)
{
}
