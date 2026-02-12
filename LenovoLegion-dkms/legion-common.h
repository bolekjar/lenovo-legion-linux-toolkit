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
#include <linux/mutex.h>
#include <linux/platform_profile.h>
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


int legion_data_init(struct legion_data* data);
void legion_data_exit(struct legion_data* data);


#endif // LEGION_COMMON_H
