/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_CPU_GPU_SYSFS_H_
#define LEGION_WMI_CPU_GPU_SYSFS_H_

struct legion_wmi_cpu_gpu_priv;

int legion_wmi_cpu_gpu_sysfs_init(struct legion_wmi_cpu_gpu_priv *priv);
void legion_wmi_cpu_gpu_sysfs_exit(struct legion_wmi_cpu_gpu_priv *priv);

#endif /* LEGION_WMI_CPU_GPU_SYSFS_H_ */
