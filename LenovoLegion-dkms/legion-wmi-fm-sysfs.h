// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_FM_SYSFS_H_
#define LEGION_WMI_FM_SYSFS_H_


struct legion_wmi_fm_priv;

int  legion_wmi_fm_sysfs_init(struct legion_wmi_fm_priv *priv);
void legion_wmi_fm_sysfs_exit(struct legion_wmi_fm_priv *priv);


#endif /* LEGION_WMI_FM_SYSFS_H_ */
