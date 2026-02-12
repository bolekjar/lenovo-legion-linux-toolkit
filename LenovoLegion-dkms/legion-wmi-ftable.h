// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_FTABLE_H_
#define LEGION_WMI_FTABLE_H_

#include <linux/types.h>

struct device;
struct legion_wmi_ftable_list;

struct legion_wmi_ftable {
	u16 mode;
	u16 fanId;
	u32 lengthFanTable;
	u16 fanTableData[10];
	u16 sensorId;
	u32 lengthSensorTable;
	u16 sensorTableData[10];
};

int legion_wmi_ftable_get_data(struct legion_wmi_ftable_list *list,u16 mode, u16 fanId, u16 sensorId, struct legion_wmi_ftable *output);
int legion_wmi_ftable_match(struct device *dev, void *data);


int  legion_wmi_ftable_driver_init(struct device *parent);
void legion_wmi_ftable_driver_exit(void);

#endif /* LEGION_WMI_FTABLE_H_ */
