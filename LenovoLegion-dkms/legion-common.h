// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_COMMON_H
#define LEGION_COMMON_H

#include "legion-wmi-ddata.h"
#include "legion-wmi-ftable.h"
#include "legion-machine-information.h"
#include "legion-machine-information-sysfs.h"
#include "legion-intel-msr-sysfs.h"
#include "legion-intel-msr.h"
#include "legion-rapl.h"

#include <linux/device.h>
#include <linux/version.h>

#include "legion-rapl-mmio.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)

struct legion_data {
    struct mutex   						   		mutex;
    struct machine_information             		machine_info;
    struct machine_information_sysfs_private	machine_info_sysfs_private;

    /*
     * EC SysFs
     */
    int 										ec_ida_id;
	struct device 							   *ec_fw_attr_dev;
	struct kset   							   *ec_fw_attr_kset;


	/*
	 * Rapl MMIO
	 */
    struct legion_rapl_mmio_private      	   rapl_mmio_private;

    /*
     * Rapl
     */
    struct legion_rapl_private				   rapl_private;

    /*
	 * Intel MSR
	 */
    struct legion_intel_msr_sysfs_private      intel_msr_sysfs_private;
    struct legion_intel_msr_private            intel_msr_private;


    /*
     * Notifier block
     */
    struct notifier_block 					   nb;

    /*
     * HWMon device
     */
    struct device						   	   *hwmon_dev;

    /*
     * Component master bound flag
     */
    bool component_master_bound;
};

#else

struct legion_data {
    struct mutex   						   				mutex;
    struct machine_information             				machine_info;
    struct machine_information_sysfs_private			machine_info_sysfs_private;

    /*
     * EC SysFs
     */
    int 												ec_ida_id;
	struct device 							   		   *ec_fw_attr_dev;
	struct kset   							  		   *ec_fw_attr_kset;

    /*
     * Rapl
     */
    struct legion_rapl_private				  			rapl_private;

	/*
	 * Rapl MMIO
	 */
    struct legion_rapl_mmio_private      	   			rapl_mmio_private;

    /*
     * Notifier block
     */
    struct notifier_block 					   nb;

	/*
	 * Intel MSR
	 */
    struct legion_intel_msr_sysfs_private      intel_msr_sysfs_private;
    struct legion_intel_msr_private            intel_msr_private;

    /*
     * HWMon device
     */
    struct device						  			   *hwmon_dev;
};

#endif


enum legion_events_type {

	/*
	 * Reserve 0 - 999 for WMI EVENTS directly from HW
	 */
	LENOVO_WMI_EVENT_UTILITY            = 0,
	LEGION_WMI_EVENT_THERMAL_MODE 		= 1,
	LEGION_WMI_EVENT_POWER_CHARGE_MODE 	= 2,
	LEGION_WMI_EVENT_SMART_FAN_MODE     = 3,
	LEGION_WMI_EVENT_KEYLOCK_STATUS     = 4,
	LEGION_WMI_EVENT_UNKNOWN			= 5,

	/*
	 * Reserve 1000 - 1999 for WMI Other Events
	 */
	LENOVO_WMI_OTHER_CPU_STP			= 1000,
	LENOVO_WMI_OTHER_CPU_LTP			= 1001,
	LENOVO_WMI_OTHER_CPU_PP				= 1002,
	LENOVO_WMI_OTHER_CPU_TMP			= 1003,
	LENOVO_WMI_OTHER_APU_PPT			= 1004,
	LENOVO_WMI_OTHER_CPU_CLP			= 1005,
	LENOVO_WMI_OTHER_CPU_PL1_TAU		= 1006,
	LENOVO_WMI_OTHER_GPU_PB				= 1007,
	LENOVO_WMI_OTHER_GPU_TGP			= 1008,
	LENOVO_WMI_OTHER_GPU_TMP			= 1009,
	LENOVO_WMI_OTHER_GPU_TAC			= 1010,
	LENOVO_WMI_OTHER_GC_DB				= 1011,
	LENOVO_WMI_OTHER_IGPU_M				= 1012,
	LENOVO_WMI_OTHER_IB_AC				= 1013,
	LENOVO_WMI_OTHER_IB_UPD				= 1014,
	LENOVO_WMI_OTHER_FF_S				= 1015,
	LENOVO_WMI_OTHER_OD					= 1016,
	LENOVO_WMI_OTHER_GM_FNQ				= 1017,
	LENOVO_WMI_OTHER_NG_DDS				= 1018,
	LENOVO_WMI_OTHER_FL_ST				= 1019,
	LENOVO_WMI_OTHER_AMD_STT			= 1020,
	LENOVO_WMI_OTHER_SS_M				= 1021,

	/*
	 * Reserve 2000 - 2999 for WMI FAN Events
	 */
	LENOVO_WMI_FAN_CURVE				= 2000,

	/*
	 * Reserve 3000 - 3999 for INTEL MSR Events
	 */
	LENOVO_INTEL_MSR_PLANE_CPU			= 2001,
	LENOVO_INTEL_MSR_PLANE_CACHE		= 2002,
	LENOVO_INTEL_MSR_PLANE_GPU			= 2003,
	LENOVO_INTEL_MSR_PLANE_UNCORE		= 2004,
	LENOVO_INTEL_MSR_PLANE_ANALOGIO		= 2005
};

int legion_data_init(struct legion_data* data);
void legion_data_exit(struct legion_data* data);


#endif // LEGION_COMMON_H
