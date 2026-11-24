/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_CAPDATA0_H_
#define LEGION_WMI_CAPDATA0_H_

#include <linux/bits.h>
#include <linux/types.h>

struct device;
struct legion_wmi_cd00_list;

/*
 * Bits of the capability of a feature (Capability of the capability data 00 / 01, the same as
 * LWMI_SUPP_* of the Linux kernel drivers/platform/x86/lenovo/wmi-capdata.h): e.g. 3 = read only
 * (temperatures), 7 = read and write (the custom mode of the capability data 01), 1 = supported
 * only (the other power modes)
 */
#define LEGION_WMI_SUPP_VALID BIT(0)
#define LEGION_WMI_SUPP_GET BIT(1)
#define LEGION_WMI_SUPP_SET BIT(2)

/* Instance of LENOVO_CAPABILITY_DATA_00 (binary MOF: IDs, Capability, DefaultValue) */
struct legion_wmi_capdata00 {
	u32 id;
	u32 supported;
	u32 default_value;
};

int legion_wmi_cd00_get_data(struct legion_wmi_cd00_list *list,
			     u32 attribute_id,
			     struct legion_wmi_capdata00 *output);
int legion_wmi_cd00_match(struct device *dev, void *data);

int legion_wmi_cd00_driver_init(void);
void legion_wmi_cd00_driver_exit(void);

#endif /* LEGION_WMI_CAPDATA0_H_ */
