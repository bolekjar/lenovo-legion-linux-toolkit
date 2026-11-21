/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_PLATFORM_H_
#define LEGION_WMI_PLATFORM_H_

#include <linux/types.h>

struct device;

bool legion_wmi_platform_detect(struct device *dev, int *err);
bool legion_wmi_platform_legacy(int *err);
bool legion_wmi_platform_bios_supported(struct device *dev);

#endif /* LEGION_WMI_PLATFORM_H_ */
