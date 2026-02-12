// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_WMI_OTHER_SYSFS_H_
#define LEGION_WMI_OTHER_SYSFS_H_

struct legion_wmi_other_priv;

int  legion_wmi_other_sysfs_init(struct legion_wmi_other_priv *priv);
void legion_wmi_other_sysfs_exit(struct legion_wmi_other_priv *priv);

#endif /* LEGION_WMI_OTHER_SYSFS_H_ */
