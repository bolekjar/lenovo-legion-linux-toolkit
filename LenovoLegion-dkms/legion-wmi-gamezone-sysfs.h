// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_GAMEZONE_SYSFS_H_
#define LEGION_WMI_GAMEZONE_SYSFS_H_

struct lenovo_wmi_gz_priv;

int  legion_wmi_gamezone_sysfs_init(struct lenovo_wmi_gz_priv *priv);
void legion_wmi_gamezone_sysfs_exit(struct lenovo_wmi_gz_priv *priv);

#endif /* LEGION_WMI_GAMEZONE_SYSFS_H_ */
