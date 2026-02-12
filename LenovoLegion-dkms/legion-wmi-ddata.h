// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_DDATA_H_
#define LEGION_WMI_DDATA_H_

#include <linux/types.h>

struct device;
struct legion_wmi_dd_list;

struct legion_wmi_ddata {
	u32  id;
	u32  value;
};

int legion_wmi_dd_get_data(struct legion_wmi_dd_list *list, u32 attribute_id, struct legion_wmi_ddata *output,size_t output_len);
int legion_wmi_dd_match(struct device *dev, void *data);


int  legion_wmi_dd_driver_init(struct device *parent);
void legion_wmi_dd_driver_exit(void);

#endif /* LEGION_WMI_DDATA_H_ */
