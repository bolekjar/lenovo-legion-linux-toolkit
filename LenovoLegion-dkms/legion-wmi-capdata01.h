// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_CAPDATA01_H_
#define LEGION_WMI_CAPDATA01_H_

#include <linux/types.h>

struct device;
struct legion_wmi_cd01_list;

struct legion_wmi_capdata01 {
	u32 id;
	u32 supported;
	u32 default_value;
	u32 step;
	u32 min_value;
	u32 max_value;
};

int legion_wmi_cd01_get_data(struct legion_wmi_cd01_list *list, u32 attribute_id, struct legion_wmi_capdata01 *output);
int legion_wmi_cd01_match(struct device *dev, void *data);


int  legion_wmi_cd01_driver_init(struct device *parent);
void legion_wmi_cd01_driver_exit(void);

#endif /* LEGION_WMI_CAPDATA01_H_ */
