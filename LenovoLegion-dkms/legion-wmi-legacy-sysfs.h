/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_LEGACY_SYSFS_H_
#define LEGION_WMI_LEGACY_SYSFS_H_

struct legion_wmi_legacy_priv;

int legion_wmi_legacy_sysfs_init(struct legion_wmi_legacy_priv *priv);
void legion_wmi_legacy_sysfs_exit(struct legion_wmi_legacy_priv *priv);

#endif /* LEGION_WMI_LEGACY_SYSFS_H_ */
