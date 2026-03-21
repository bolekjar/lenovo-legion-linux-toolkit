// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_RAPL_H_
#define LEGION_RAPL_H_

#include <linux/mutex.h>

struct legion_rapl_private {
	struct mutex lock;
};

int  legion_rapl_sysfs_init(const struct device *parent);
void legion_rapl_sysfs_exit(const struct device *parent);

/* Check if RAPL is enabled */
ssize_t  legion_rapl_sysfs_is_enabled(struct legion_rapl_private* rapl_private,  bool* is_enabled);
ssize_t  legion_rapl_sysfs_set_enabled(struct legion_rapl_private* rapl_private, bool enable);

/* Power limit read/write functions */
ssize_t  legion_pl1_power_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* pl1_uw);
ssize_t  legion_pl2_power_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* pl2_uw);
ssize_t  legion_pl4_power_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* pl4_uw);

/* Time window read functions (in microseconds) */
ssize_t  legion_pl1_time_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* time_us);
ssize_t  legion_pl2_time_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* time_us);

/* Set power limits (constraint 0 = PL1, constraint 1 = PL2, constraint 2 = PL4) */
ssize_t  legion_pl1_power_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int pl1_uw);
ssize_t  legion_pl2_power_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int pl2_uw);
ssize_t  legion_pl4_power_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int pl4_uw);

/* Set time windows */
ssize_t  legion_pl1_time_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int time_us);
ssize_t  legion_pl2_time_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int time_us);

/* Combined set functions */
ssize_t  legion_set_power_sysfs(struct legion_rapl_private* rapl_private,unsigned int pl1_uw, unsigned int pl2_uw);
ssize_t  legion_set_power_and_time_sysfs(struct legion_rapl_private* rapl_private,unsigned int pl1_uw, unsigned int pl1_time_us,unsigned int pl2_uw);

#endif /* LEGION_RAPL_H_ */
