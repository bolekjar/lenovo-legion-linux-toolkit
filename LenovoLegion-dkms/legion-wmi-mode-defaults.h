/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_MODE_DEFAULTS_H_
#define LEGION_WMI_MODE_DEFAULTS_H_

#include <linux/types.h>

/*
 * Default values of a power mode of the older firmware (LENOVO_DEFAULT_VALUE_IN_DIFFERENT_MODE_DATA)
 */
enum legion_wmi_mode_default {
	LEGION_WMI_MODE_DEFAULT_NONE = 0,
	LEGION_WMI_MODE_DEFAULT_CPU_LTP,
	LEGION_WMI_MODE_DEFAULT_CPU_STP,
	LEGION_WMI_MODE_DEFAULT_CPU_CLP,
	LEGION_WMI_MODE_DEFAULT_CPU_TMP,
	LEGION_WMI_MODE_DEFAULT_GPU_PPAB,
	LEGION_WMI_MODE_DEFAULT_GPU_CTGP,
	LEGION_WMI_MODE_DEFAULT_GPU_TMP,
};

int legion_wmi_mode_default_get(u32 mode, enum legion_wmi_mode_default field,
				u32 *value);

int legion_wmi_mode_defaults_driver_init(void);
void legion_wmi_mode_defaults_driver_exit(void);

#endif /* LEGION_WMI_MODE_DEFAULTS_H_ */
