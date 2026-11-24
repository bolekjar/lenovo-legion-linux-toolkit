/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_CAPDATA01_H_
#define LEGION_WMI_CAPDATA01_H_

#include <linux/types.h>

struct device;
struct legion_wmi_cd01_list;

/*
 * Instance of LENOVO_CAPABILITY_DATA_01 (binary MOF: IDs, Capability, DefaultValue, Step, MinValue,
 * MaxValue), the ID with the power mode (LEGION_WMI_MODE_ID_MASK); supported is the bit mask
 * LEGION_WMI_SUPP_* (legion-wmi-capdata00.h).
 *
 * Seen on Legion Pro 7 16IAX10H (ASL WQA5, one of 8 sets of tables by the model and the GPU):
 * - a feature without a range (min, max and step 0, e.g. 0x0107 PL1 Tau, 0x0201, 0x0202, 0x020B)
 *   has bit 10 set, its values are in the discrete data (legion-wmi-ddata.c),
 * - the overclocking features 0x0108 (CPU) and 0x0205 (GPU) are twice with the same IDs, the
 *   second one differs by bit 4 / 5; not used here, legion_wmi_cd01_get_data() returns the first.
 */
struct legion_wmi_capdata01 {
	u32 id;
	u32 supported;
	u32 default_value;
	u32 step;
	u32 min_value;
	u32 max_value;
};

int legion_wmi_cd01_get_data(struct legion_wmi_cd01_list *list,
			     u32 attribute_id,
			     struct legion_wmi_capdata01 *output);
int legion_wmi_cd01_match(struct device *dev, void *data);

int legion_wmi_cd01_driver_init(void);
void legion_wmi_cd01_driver_exit(void);

#endif /* LEGION_WMI_CAPDATA01_H_ */
