// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_CAPDATA0_H_
#define LEGION_WMI_CAPDATA0_H_

#include <linux/types.h>

struct device;
struct legion_wmi_cd00_list;

struct legion_wmi_capdata00 {
	u32 id;
	u32 supported;
	u32 default_value;
};

int legion_wmi_cd00_get_data(struct legion_wmi_cd00_list *list, u32 attribute_id, struct legion_wmi_capdata00 *output);
int legion_wmi_cd00_match(struct device *dev, void *data);


int  legion_wmi_cd00_driver_init(struct device *parent);
void legion_wmi_cd00_driver_exit(void);

#endif /* LEGION_WMI_CAPDATA0_H_ */
