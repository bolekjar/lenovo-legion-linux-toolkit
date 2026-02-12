/*
 * legion-dkms.h
 *
 *  Created on: Jan 19, 2026
 *      Author: bojar
 */

#ifndef LEGION_DKMS_H_
#define LEGION_DKMS_H_


#include "legion-wmi-gamezone.h"

enum other_events_type {
	LEGION_WMI_OTHER_SET_THERMAL_MODE_RAPL_AND_MMIO		= 1,
	LEGION_WMI_OTHER_SET_THERMAL_MODE_RAPL_MMIO_ONLY	= 2
};

struct other_events_data {
	enum thermal_mode mode;

    struct legion_rapl_mmio_private*      	   rapl_mmio_private;
    struct legion_rapl_private*				   rapl_private;
};

struct device;
struct notifier_block;
int devm_lenovo_dkms_register_notifier(struct device *dev,struct notifier_block *nb);

#endif /* LEGION_DKMS_H_ */
