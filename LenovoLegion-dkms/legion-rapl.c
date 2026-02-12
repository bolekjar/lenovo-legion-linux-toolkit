// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-rapl.h"
#include "legion-common.h"

#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/mutex.h>

/* Sysfs base path for intel-rapl package 0 */
#define RAPL_SYSFS_BASE "/sys/class/powercap/intel-rapl/intel-rapl:0"

/* Helper: Read value from sysfs file */
static int read_sysfs_u64(const char *path, u64 *value)
{
	struct file *file;
	char buf[64];
	loff_t pos = 0;
	ssize_t ret;
	
	file = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(file))
		return PTR_ERR(file);
	
	ret = kernel_read(file, buf, sizeof(buf) - 1, &pos);
	filp_close(file, NULL);
	
	if (ret < 0)
		return ret;
	
	buf[ret] = '\0';
	ret = kstrtoull(buf, 10, value);
	
	return ret;
}

/* Helper: Write value to sysfs file */
static int write_sysfs_u64(const char *path, u64 value)
{
	struct file *file;
	char buf[64];
	loff_t pos = 0;
	ssize_t ret;
	size_t len;
	
	file = filp_open(path, O_WRONLY, 0);
	if (IS_ERR(file))
		return PTR_ERR(file);
	
	len = snprintf(buf, sizeof(buf), "%llu", value);
	ret = kernel_write(file, buf, len, &pos);
	filp_close(file, NULL);
	
	if (ret < 0)
		return ret;
	
	return (ret == len) ? 0 : -EIO;
}

/* Check if RAPL is enabled via sysfs */
int legion_rapl_sysfs_is_enabled(struct legion_rapl_private* rapl_private, bool* is_enabled)
{
	u64 enabled;
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = read_sysfs_u64(RAPL_SYSFS_BASE "/enabled", &enabled);
	mutex_unlock(&rapl_private->lock);
	
	if (ret)
		return ret;
	
	*is_enabled = (enabled != 0);
	return 0;
}

/* Enable or disable RAPL via sysfs */
int legion_rapl_sysfs_set_enabled(struct legion_rapl_private* rapl_private, bool enable)
{
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = write_sysfs_u64(RAPL_SYSFS_BASE "/enabled", enable ? 1 : 0);
	mutex_unlock(&rapl_private->lock);
	
	return ret;
}

/* Initialize RAPL sysfs interface */
int legion_rapl_sysfs_init(struct device *parent)
{
	struct legion_data* data = dev_get_drvdata(parent);

	mutex_init(&data->rapl_private.lock);
	
	/* Check if intel-rapl sysfs exists */
	struct file *file = filp_open(RAPL_SYSFS_BASE "/name", O_RDONLY, 0);
	if (IS_ERR(file)) {
		return PTR_ERR(file);
	}
	filp_close(file, NULL);


	return 0;
}

/* Cleanup RAPL sysfs interface */
void legion_rapl_sysfs_exit(struct device *parent)
{
	struct legion_data* data = dev_get_drvdata(parent);
	mutex_destroy(&data->rapl_private.lock);
}

/* Read PL1 power limit (in microwatts) */
int legion_pl1_power_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* pl1_uw)
{
	u64 power_uw;
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = read_sysfs_u64(RAPL_SYSFS_BASE "/constraint_0_power_limit_uw", &power_uw);
	mutex_unlock(&rapl_private->lock);
	
	if (ret)
		return ret;
	
	*pl1_uw = (unsigned int)power_uw;
	return 0;
}

/* Read PL2 power limit (in microwatts) */
int legion_pl2_power_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* pl2_uw)
{
	u64 power_uw;
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = read_sysfs_u64(RAPL_SYSFS_BASE "/constraint_1_power_limit_uw", &power_uw);
	mutex_unlock(&rapl_private->lock);
	
	if (ret)
		return ret;
	
	*pl2_uw = (unsigned int)power_uw;
	return 0;
}

/* Read PL4 power limit (in microwatts) */
int legion_pl4_power_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* pl4_uw)
{
	u64 power_uw;
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = read_sysfs_u64(RAPL_SYSFS_BASE "/constraint_2_power_limit_uw", &power_uw);
	mutex_unlock(&rapl_private->lock);
	
	if (ret)
		return ret;
	
	*pl4_uw = (unsigned int)power_uw;
	return 0;
}

/* Read PL1 time window (in microseconds) */
int legion_pl1_time_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* time_us)
{
	u64 time_window_us;
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = read_sysfs_u64(RAPL_SYSFS_BASE "/constraint_0_time_window_us", &time_window_us);
	mutex_unlock(&rapl_private->lock);
	
	if (ret)
		return ret;
	
	*time_us = (unsigned int)time_window_us;
	return 0;
}

/* Read PL2 time window (in microseconds) */
int legion_pl2_time_sysfs_read(struct legion_rapl_private* rapl_private, unsigned int* time_us)
{
	u64 time_window_us;
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = read_sysfs_u64(RAPL_SYSFS_BASE "/constraint_1_time_window_us", &time_window_us);
	mutex_unlock(&rapl_private->lock);
	
	if (ret)
		return ret;
	
	*time_us = (unsigned int)time_window_us;
	return 0;
}

/* Set PL1 power limit (in microwatts) */
int legion_pl1_power_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int pl1_uw)
{
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = write_sysfs_u64(RAPL_SYSFS_BASE "/constraint_0_power_limit_uw", pl1_uw);
	mutex_unlock(&rapl_private->lock);
	
	return ret;
}

/* Set PL2 power limit (in microwatts) */
int legion_pl2_power_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int pl2_uw)
{
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = write_sysfs_u64(RAPL_SYSFS_BASE "/constraint_1_power_limit_uw", pl2_uw);
	mutex_unlock(&rapl_private->lock);
	
	return ret;
}

/* Set PL4 power limit (in microwatts) */
int legion_pl4_power_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int pl4_uw)
{
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = write_sysfs_u64(RAPL_SYSFS_BASE "/constraint_2_power_limit_uw", pl4_uw);
	mutex_unlock(&rapl_private->lock);
	
	return ret;
}

/* Set PL1 time window (in microseconds) */
int legion_pl1_time_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int time_us)
{
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = write_sysfs_u64(RAPL_SYSFS_BASE "/constraint_0_time_window_us", time_us);
	mutex_unlock(&rapl_private->lock);
	
	return ret;
}

/* Set PL2 time window (in microseconds) */
int legion_pl2_time_sysfs_set(struct legion_rapl_private* rapl_private, unsigned int time_us)
{
	int ret;
	
	mutex_lock(&rapl_private->lock);
	ret = write_sysfs_u64(RAPL_SYSFS_BASE "/constraint_1_time_window_us", time_us);
	mutex_unlock(&rapl_private->lock);
	
	return ret;
}

/* Set PL1 and PL2 power limits together */
int legion_set_power_sysfs(struct legion_rapl_private* rapl_private,
                           unsigned int pl1_uw, unsigned int pl2_uw)
{
	int ret;
	
	ret = legion_pl1_power_sysfs_set(rapl_private, pl1_uw);
	if (ret)
		return ret;
	
	return legion_pl2_power_sysfs_set(rapl_private, pl2_uw);
}

/* Set PL1 and PL2 power limits and time windows together */
int legion_set_power_and_time_sysfs(struct legion_rapl_private* rapl_private,unsigned int pl1_uw, unsigned int pl1_time_us,unsigned int pl2_uw)
{
	int ret;
	
	ret = legion_pl1_power_sysfs_set(rapl_private, pl1_uw);
	if (ret)
		return ret;
	
	ret = legion_pl1_time_sysfs_set(rapl_private, pl1_time_us);
	if (ret)
		return ret;
	
	ret = legion_pl2_power_sysfs_set(rapl_private, pl2_uw);
	if (ret)
		return ret;
	
	return 0;
}
