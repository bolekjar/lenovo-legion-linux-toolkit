/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_COMMON_H
#define LEGION_COMMON_H

#include "legion-machine-information.h"

#include <linux/device.h>

/*
 * Data of the platform device (VPC2004)
 */
struct legion_data {
	struct machine_information machine_info; /* DMI, compatibility check */
	struct device *hwmon_dev;
	u8 hwmon_temperatures; /* supported temperature channels (bits) */
	u8 hwmon_fans; /* supported fan channels (bits) */
};

enum legion_events_type {

	/*
	 * Reserve 0 - 999 for WMI EVENTS directly from HW
	 */
	LENOVO_WMI_EVENT_UTILITY = 0,
	LEGION_WMI_EVENT_THERMAL_MODE = 1,
	LEGION_WMI_EVENT_POWER_CHARGE_MODE = 2,
	LEGION_WMI_EVENT_SMART_FAN_MODE = 3,
	LEGION_WMI_EVENT_KEYLOCK_STATUS = 4,
	LEGION_WMI_EVENT_UNKNOWN = 5,

	/*
	 * Reserve 1000 - 1999 for WMI Other Events
	 */
	LENOVO_WMI_OTHER_CPU_STP = 1000,
	LENOVO_WMI_OTHER_CPU_LTP = 1001,
	LENOVO_WMI_OTHER_CPU_PP = 1002,
	LENOVO_WMI_OTHER_CPU_TMP = 1003,
	LENOVO_WMI_OTHER_APU_PPT = 1004,
	LENOVO_WMI_OTHER_CPU_CLP = 1005,
	LENOVO_WMI_OTHER_CPU_PL1_TAU = 1006,
	LENOVO_WMI_OTHER_GPU_PB = 1007,
	LENOVO_WMI_OTHER_GPU_TGP = 1008,
	LENOVO_WMI_OTHER_GPU_TMP = 1009,
	LENOVO_WMI_OTHER_GPU_TAC = 1010,
	LENOVO_WMI_OTHER_GC_DB = 1011,
	LENOVO_WMI_OTHER_IGPU_M = 1012,
	LENOVO_WMI_OTHER_IB_AC = 1013,
	LENOVO_WMI_OTHER_IB_UPD = 1014,
	LENOVO_WMI_OTHER_FF_S = 1015,
	LENOVO_WMI_OTHER_OD = 1016,
	LENOVO_WMI_OTHER_GM_FNQ = 1017,
	LENOVO_WMI_OTHER_NG_DDS = 1018,
	LENOVO_WMI_OTHER_FL_ST = 1019,
	LENOVO_WMI_OTHER_AMD_STT = 1020,
	LENOVO_WMI_OTHER_SS_M = 1021,

	/*
	 * Reserve 2000 - 2999 for WMI FAN Events
	 */
	LENOVO_WMI_FAN_CURVE = 2000

	/*
	 * 2001 - 2005: Intel MSR events of the separate module legion_intel_msr (legion-intel-msr-sysfs.h)
	 */
};

int legion_data_init(struct legion_data *data);
void legion_data_exit(struct legion_data *data);

#endif // LEGION_COMMON_H
