/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_FTABLE_H_
#define LEGION_WMI_FTABLE_H_

#include "legion-wmi-fm.h"

#include <linux/types.h>

struct device;
struct legion_wmi_ftable_list;

/*
 * Start of a row of LENOVO_FAN_TABLE_DATA (binary MOF, packed), the rest of
 * the row (the limits of the fan and the sensor table, from
 * StartOnlyUpwardAdjustNumber on) is not used. The fan and the sensor table
 * have the points of the fan curve (FanTable_Len, SensorTable_Len).
 */
struct legion_wmi_ftable {
	u16 mode;
	u16 fanId;
	u32 lengthFanTable;
	u16 fanTableData[MAX_FANCURVE_SIZE];
	u32 sensorId;
	u32 lengthSensorTable;
	u16 sensorTableData[MAX_FANCURVE_SIZE];
} __packed;

int legion_wmi_ftable_get_data(struct legion_wmi_ftable_list *list, u16 mode,
			       u16 fanId, u16 sensorId,
			       struct legion_wmi_ftable *output);
int legion_wmi_ftable_match(struct device *dev, void *data);

int legion_wmi_ftable_driver_init(void);
void legion_wmi_ftable_driver_exit(void);

#endif /* LEGION_WMI_FTABLE_H_ */
