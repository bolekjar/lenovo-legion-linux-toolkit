/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_COMMON_H
#define LEGION_COMMON_H

/*
 * Name of the kset of the attributes under each class device (/sys/class/<class>/<device>/attributes/<attribute>),
 * the same as the firmware-attributes class of the kernel
 */
#define LEGION_FW_ATTR_KSET_NAME "attributes"

/*
 * Event types of the kernel uevents (EVENT_TYPE) and of the notifier chains of the driver
 */
enum legion_events_type {

	/*
	 * Reserve 0 - 999 for WMI EVENTS directly from HW
	 */
	LENOVO_WMI_EVENT_UTILITY = 0,
	/*
	 * EVENT_VALUE: effective thermal mode, 255 for the custom mode also on the older firmware; sent also after a
	 * resume (the current mode)
	 */
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
	LENOVO_WMI_FAN_CURVE = 2000,
	LENOVO_WMI_FAN_FULL_SPEED = 2001,

	/*
	 * 2001 - 2005: voltage offsets of the separate module legion_intel_msr (EVENT_VALUE: offset in uV)
	 */
	LEGION_INTEL_MSR_PLANE_CPU = 2001,
	LEGION_INTEL_MSR_PLANE_CACHE = 2002,
	LEGION_INTEL_MSR_PLANE_GPU = 2003,
	LEGION_INTEL_MSR_PLANE_UNCORE = 2004,
	LEGION_INTEL_MSR_PLANE_ANALOGIO = 2005,

	/*
	 * Reserve 3000 - 3999 for the ACPI VPC2004 features (legion-acpi-vpc.c, order of its features)
	 */
	LEGION_ACPI_VPC_CONSERVATION_MODE = 3000,
	LEGION_ACPI_VPC_USB_CHARGING = 3001,
	LEGION_ACPI_VPC_FN_LOCK = 3002,
	LEGION_ACPI_VPC_CAMERA_POWER = 3003,
	LEGION_ACPI_VPC_RAPID_CHARGE = 3004,

	/*
	 * Reserve 4000 - 4999 for the limits of the older firmware (legion-wmi-cpu-gpu.c)
	 */
	LEGION_WMI_CPU_LTP = 4000,
	LEGION_WMI_CPU_STP = 4001,
	LEGION_WMI_CPU_PP = 4002,
	LEGION_WMI_CPU_CLP = 4003,
	LEGION_WMI_CPU_TMP = 4004,
	LEGION_WMI_CPU_APU_SPPT = 4005,
	LEGION_WMI_GPU_PB = 4010,
	LEGION_WMI_GPU_TGP = 4011,
	LEGION_WMI_GPU_TMP = 4012
};

#endif // LEGION_COMMON_H
