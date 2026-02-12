// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include <linux/io.h>
#include <linux/pci.h>

#ifndef LEGION_RAPL_MMIO_H_
#define LEGION_RAPL_MMIO_H_

#define POWER_LIMIT_MAX_VALUE_IN_MW 999000

struct legion_rapl_mmio_private {
	void __iomem *rapl_mmio_base;
	struct pci_dev *thermal_dev;
	resource_size_t bar_start;
	resource_size_t bar_len;
	struct mutex lock;
};

int  legion_rapl_mmio_init(struct device *parent);
void legion_rapl_mmio_exit(struct device *parent);

/* Power limit read/write functions */
int  legion_pl1_power_read(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int* pl1);
int  legion_pl2_power_read(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int* pl2);
int  legion_pl4_power_read(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int* pl4);

/* Time window read functions (in microseconds) */
int  legion_pl1_time_read(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int* time_us);
int  legion_pl2_time_read(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int* time_us);

/* Lock status functions */
int  legion_is_locked_power_read(struct legion_rapl_mmio_private* rapl_mmio_private, bool* isLocked);
int  legion_pl4_lock_read(struct legion_rapl_mmio_private* rapl_mmio_private, bool* isLocked);

/* Set and lock functions */
int  legion_set_and_lock(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl1_mw, unsigned int pl2_mw);
int  legion_pl4_set_and_lock(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl4_mw);
int  legion_set_power_and_time_lock(struct legion_rapl_mmio_private* rapl_mmio_private,
                                     unsigned int pl1_mw, unsigned int pl1_time_us,
                                     unsigned int pl2_mw, unsigned int pl2_time_us);

/* Set without lock functions (can be changed later if not locked) */
int  legion_set_power(struct legion_rapl_mmio_private* rapl_mmio_private,
                      unsigned int pl1_mw, unsigned int pl2_mw);
/* Sets PL1/PL2 power and PL1 time window only (not PL2 time window) */
int  legion_set_power_and_time(struct legion_rapl_mmio_private* rapl_mmio_private,
                                unsigned int pl1_mw, unsigned int pl1_time_us,
                                unsigned int pl2_mw);
int  legion_pl4_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl4_mw);

/* Individual set functions for fine-grained control (without lock) */
int  legion_pl1_power_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl1_mw);
int  legion_pl2_power_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl2_mw);
int  legion_pl1_time_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int time_us);
int  legion_pl2_time_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int time_us);


#endif /* LEGION_RAPL_MMIO_H_ */
