/* SPDX-License-Identifier: GPL-2.0 */
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

/* Maximal number of the discrete values of a feature (16IAX10H: 13 values of the PL1 Tau) */
#define LEGION_WMI_DD_MAX_VALUES 32

/*
 * Instance of LENOVO_DISCRETE_DATA (binary MOF: IDs, Value), one discrete value of a feature in a
 * power mode; the features without a range of the capability data 01 (bit 10). The firmware fills
 * the data block up with instances of the ID 0 (ASL WQA7), they are not kept. An instance is a buffer
 * of the ID and the value (16IAX10H) or a package of the two integers (AMD Legion of the generation 8).
 */
struct legion_wmi_ddata {
	u32 id;
	u32 value;
};

int legion_wmi_dd_get_data(struct legion_wmi_dd_list *list, u32 attribute_id,
			   struct legion_wmi_ddata *output, size_t output_len);
int legion_wmi_dd_match(struct device *dev, void *data);

int legion_wmi_dd_driver_init(void);
void legion_wmi_dd_driver_exit(void);

#endif /* LEGION_WMI_DDATA_H_ */
