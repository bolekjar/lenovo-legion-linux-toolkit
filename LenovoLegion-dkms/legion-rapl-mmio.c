// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-common.h"
#include "legion-rapl-mmio.h"

#include <linux/pci_ids.h>



/* RAPL MMIO register offset within the processor thermal BAR */
#define RAPL_LIMIT_OFFSET 0x59A0
#define RAPL_PL4_OFFSET   0x59B0  /* Peak Power Limit (PPL/PL4) */

/* RAPL register bit fields */
#define RAPL_PL1_MASK       0x7FFF        /* Bits 0-14: Power Limit 1 (LTP) */
#define RAPL_PL1_ENABLE     (1ULL << 15)  /* Bit 15: Enable PL1 */
#define RAPL_PL1_CLAMP      (1ULL << 16)  /* Bit 16: Clamp PL1 */
#define RAPL_PL1_TIME_MASK  0xFE0000      /* Bits 17-23: Time window PL1 */
#define RAPL_PL1_TIME_SHIFT 17

#define RAPL_PL2_MASK       0x7FFF00000000ULL  /* Bits 32-46: Power Limit 2 (STP) */
#define RAPL_PL2_SHIFT      32
#define RAPL_PL2_ENABLE     (1ULL << 47)  /* Bit 47: Enable PL2 */
#define RAPL_PL2_CLAMP      (1ULL << 48)  /* Bit 48: Clamp PL2 */
#define RAPL_PL2_TIME_MASK  0xFE000000000000ULL  /* Bits 49-55: Time window PL2 */
#define RAPL_PL2_TIME_SHIFT 49

/* PL4 (Peak Power Limit / PPL) register bit fields at offset 0x59B0 */
#define RAPL_PL4_MASK       0x7FFF        /* Bits 0-14: Power Limit 4 (PPL) */
#define RAPL_PL4_ENABLE     (1ULL << 15)  /* Bit 15: Enable PL4 */
#define RAPL_PL4_LOCK       (1ULL << 31)  /* Bit 31: Lock PL4 */



#define RAPL_LOCK_BIT       (1ULL << 63)  /* Bit 63: Lock */
#define RAPL_LOCK_BIT_SHIFT 63

/* Power unit: typically 1/8 Watt = 0.125W per unit */
#define POWER_UNIT_DIVISOR  8


/* Intel Thermal Device IDs (RAPL MMIO compatible) */
#define PCI_DEVICE_ID_INTEL_TGL_THERMAL   0x9A03  /* Tiger Lake */
#define PCI_DEVICE_ID_INTEL_ADL_THERMAL   0x461D  /* Alder Lake */
#define PCI_DEVICE_ID_INTEL_RPL_THERMAL   0xA71D  /* Raptor Lake */
#define PCI_DEVICE_ID_INTEL_MTLP_THERMAL  0x7D03  /* Meteor Lake-P */
#define PCI_DEVICE_ID_INTEL_LNLM_THERMAL  0x641D  /* Lunar Lake-M */
#define PCI_DEVICE_ID_INTEL_ARL_S_THERMAL 0xAD03  /* Arrow Lake-S */


/* Whitelist of supported thermal device IDs */
static const u16 supported_thermal_devices[] = {
    PCI_DEVICE_ID_INTEL_TGL_THERMAL,
    PCI_DEVICE_ID_INTEL_ADL_THERMAL,
    PCI_DEVICE_ID_INTEL_RPL_THERMAL,
    PCI_DEVICE_ID_INTEL_MTLP_THERMAL,
    PCI_DEVICE_ID_INTEL_LNLM_THERMAL,
    PCI_DEVICE_ID_INTEL_ARL_S_THERMAL,
    0x0000
};


/* Check if device ID is in whitelist */
static bool is_device_supported(u16 device_id) {

	for (int i = 0; supported_thermal_devices[i] != 0x0000; i++) {
        if (supported_thermal_devices[i] == device_id)
            return true;
    }

    return false;
}

/* Find thermal device by scanning PCI bus */
static struct pci_dev *find_thermal_device_by_id(void) {
    struct pci_dev *pdev = NULL;

    /* Try standard location first (fast path) */
    pdev = pci_get_domain_bus_and_slot(0, 0, PCI_DEVFN(4, 0));
    if (pdev) {

    	/* Verify it's an Intel device with supported ID */
        if (pdev->vendor == PCI_VENDOR_ID_INTEL &&
            is_device_supported(pdev->device)) {
        	return pdev;
        }
    }

    /* Iterate through all PCI devices */
        /* Try standard location first (fast path) */

    while ((pdev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, pdev)) != NULL) {

        /* Check if device ID is in whitelist */
    	if(is_device_supported(pdev->device)){
    		return pdev;
    	}
    }

    return NULL;  /* Not found */
}


/* Helper: Convert power units to Watts */
static unsigned int units_to_watts(unsigned int units) {
    return (units * 1000) / POWER_UNIT_DIVISOR;  /* Returns milliwatts */
}

/* Helper: Convert Watts to power units */
static unsigned int watts_to_units(unsigned int milliwatts) {
    return (milliwatts * POWER_UNIT_DIVISOR) / 1000;
}

/* Helper: Decode time window from hardware format
 * Formula: time = 2^Y * (1 + F/4) * time_unit
 * Bits [6:5] = F (0-3), Bits [4:0] = Y (0-31)
 * time_unit = 1/1024 seconds (0.0009765625 s)
 * Returns: time in microseconds
 * Based on Linux kernel intel_rapl_common.c:rapl_compute_time_window_core()
 */
static unsigned int decode_time_window(unsigned int encoded) {
    unsigned int f, y;
    unsigned long long time_us;
    
    f = (encoded & 0x60) >> 5;  /* Bits 5-6: fraction */
    y = encoded & 0x1F;          /* Bits 0-4: exponent */
    
    /* time = 2^Y * (1 + F/4) * (1/1024) seconds */
    /* time_us = 2^Y * (4 + F) / 4 * 1000000 / 1024 */
    time_us = (1ULL << y) * (4 + f) * 1000000ULL / 4;
    time_us = time_us >> 10;  /* Divide by 1024 */
    
    return (unsigned int)time_us;
}

/* Helper: Encode time window to hardware format
 * Formula: time = 2^Y * (1 + F/4) * time_unit, where time_unit = 1/1024 s
 * Input: time in microseconds
 * Returns: encoded value (7 bits): bits [6:5]=F, bits [4:0]=Y
 * Based on Linux kernel intel_rapl_common.c:rapl_compute_time_window_core()
 */
static unsigned int encode_time_window(unsigned int time_us) {
    unsigned long long time_units;
    unsigned int y, f;
    unsigned long long y_val;
    
    /* Convert to units of 1/1024 seconds */
    time_units = ((unsigned long long)time_us << 10) / 1000000ULL;
    
    if (time_units == 0)
        return 0;
    
    /* Find Y such that 2^Y <= time_units */
    y = 0;
    if (time_units > 1) {
        for (y = 0; y < 31; y++) {
            if ((1ULL << (y + 1)) > time_units)
                break;
        }
    }
    
    /* Clamp Y to 5 bits (max 31) */
    if (y > 31)
        return 0x7F;
    
    y_val = 1ULL << y;
    
    /* Calculate F: time_units = 2^Y * (1 + F/4) */
    /* F = 4 * (time_units / 2^Y - 1) = 4 * (time_units - 2^Y) / 2^Y */
    if (time_units >= y_val) {
        f = (unsigned int)((4 * (time_units - y_val)) / y_val);
        if (f > 3)
            f = 3;  /* Clamp F to 2 bits */
    } else {
        f = 0;
    }
    
    /* Encode: bits [6:5] = F, bits [4:0] = Y */
    return (f << 5) | (y & 0x1F);
}

/* Find and map the processor thermal device WITHOUT claiming exclusive access */
int legion_rapl_mmio_init(struct device *parent) {
	 struct legion_data* data 							= dev_get_drvdata(parent);
	 struct legion_rapl_mmio_private* rapl_mmio_private = &data->rapl_mmio_private;;

	 int bar = 0;

     mutex_init(&rapl_mmio_private->lock);

     /* Find processor thermal device */
     rapl_mmio_private->thermal_dev = find_thermal_device_by_id();
     if (!rapl_mmio_private->thermal_dev) {
         return -ENODEV;
     }

     /* Verify it's an Intel device */
     if (rapl_mmio_private->thermal_dev->vendor != PCI_VENDOR_ID_INTEL) {
         pci_dev_put(rapl_mmio_private->thermal_dev);
         rapl_mmio_private->thermal_dev = NULL;
         return -ENODEV;
     }

     /* Check if device ID is supported */
     if (!is_device_supported(rapl_mmio_private->thermal_dev->device)) {
         pci_dev_put(rapl_mmio_private->thermal_dev);
         rapl_mmio_private->thermal_dev = NULL;
         return -ENODEV;
     }

     /* Get BAR 0 information WITHOUT enabling or claiming it */
     rapl_mmio_private->bar_start = pci_resource_start(rapl_mmio_private->thermal_dev, bar);
     rapl_mmio_private->bar_len = pci_resource_len(rapl_mmio_private->thermal_dev, bar);

     if (!rapl_mmio_private->bar_start || !rapl_mmio_private->bar_len) {
         pci_dev_put(rapl_mmio_private->thermal_dev);
         rapl_mmio_private->thermal_dev = NULL;
         return -ENODEV;
     }

     /* Map the BAR directly using ioremap - NO exclusive claim */
     rapl_mmio_private->rapl_mmio_base = ioremap(rapl_mmio_private->bar_start, rapl_mmio_private->bar_len);
     if (!rapl_mmio_private->rapl_mmio_base) {
         pci_dev_put(rapl_mmio_private->thermal_dev);
         rapl_mmio_private->thermal_dev = NULL;
         return -ENOMEM;
     }

     return 0;
}

void legion_rapl_mmio_exit(struct device *parent) {
	struct legion_data* data 							= dev_get_drvdata(parent);
	struct legion_rapl_mmio_private* rapl_mmio_private = &data->rapl_mmio_private;;

    if (rapl_mmio_private->rapl_mmio_base)
        iounmap(rapl_mmio_private->rapl_mmio_base);

    if (rapl_mmio_private->thermal_dev)
        pci_dev_put(rapl_mmio_private->thermal_dev);

    mutex_destroy(&rapl_mmio_private->lock);
}

/* Read PL1 (LTP - Long Term Power) in milliwatts */
int legion_pl1_power_read(struct legion_rapl_mmio_private* rapl_mmio_private,unsigned int* pl1) {
    uint64_t rapl_limit;
    unsigned int pl1_units, pl1_mw;

    if (!rapl_mmio_private->rapl_mmio_base)
    	return -ENODEV;

    guard(mutex)(&rapl_mmio_private->lock);

    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);
    pl1_units = rapl_limit & RAPL_PL1_MASK;
    pl1_mw = units_to_watts(pl1_units);

    *pl1 = pl1_mw;

    return 0;
}

/* Read PL2 (STP - Short Term Power) in milliwatts */
int legion_pl2_power_read(struct legion_rapl_mmio_private* rapl_mmio_private,unsigned int* pl2) {
    uint64_t rapl_limit;
    unsigned int pl2_units, pl2_mw;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -ENODEV;


    guard(mutex)(&rapl_mmio_private->lock);

    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);
    pl2_units = (rapl_limit & RAPL_PL2_MASK) >> RAPL_PL2_SHIFT;
    pl2_mw = units_to_watts(pl2_units);

    *pl2 = pl2_mw;

    return 0;
}

/* Read PL1 time window in microseconds */
int legion_pl1_time_read(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int* time_us) {
    uint64_t rapl_limit;
    unsigned int time_encoded;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -ENODEV;

    guard(mutex)(&rapl_mmio_private->lock);

    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);
    time_encoded = (rapl_limit & RAPL_PL1_TIME_MASK) >> RAPL_PL1_TIME_SHIFT;
    *time_us = decode_time_window(time_encoded);
    
    pr_info("legion_pl1_time_read: register=0x%llx, encoded=0x%x, decoded=%u us\n",
            rapl_limit, time_encoded, *time_us);

    return 0;
}

/* Read PL2 time window in microseconds */
int legion_pl2_time_read(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int* time_us) {
    uint64_t rapl_limit;
    unsigned int time_encoded;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -ENODEV;

    guard(mutex)(&rapl_mmio_private->lock);

    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);
    time_encoded = (rapl_limit & RAPL_PL2_TIME_MASK) >> RAPL_PL2_TIME_SHIFT;
    *time_us = decode_time_window(time_encoded);

    return 0;
}

/* Show PL4 lock status */
int legion_pl4_lock_read(struct legion_rapl_mmio_private* rapl_mmio_private,bool* isLocked) {
	uint64_t rapl_pl4;

    if (!rapl_mmio_private->rapl_mmio_base)
    	return -ENODEV;

    guard(mutex)(&rapl_mmio_private->lock);

    rapl_pl4 = readq(rapl_mmio_private->rapl_mmio_base + RAPL_PL4_OFFSET);

    *isLocked = (rapl_pl4 & RAPL_PL4_LOCK) ? 1 : 0;

    return 0;
}

/* Show PL4 (PPL - Peak Power Limit) in milliwatts */
int legion_pl4_power_read(struct legion_rapl_mmio_private* rapl_mmio_private,unsigned int* pl4) {
    uint64_t rapl_pl4;
    unsigned int pl4_units, pl4_mw;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -ENODEV;

    guard(mutex)(&rapl_mmio_private->lock);

    rapl_pl4 = readq(rapl_mmio_private->rapl_mmio_base + RAPL_PL4_OFFSET);
    pl4_units = rapl_pl4 & RAPL_PL4_MASK;
    pl4_mw = units_to_watts(pl4_units);

    *pl4 = pl4_mw;

    return 0;
}


/* Read locked value */
int legion_is_locked_power_read(struct legion_rapl_mmio_private* rapl_mmio_private,bool* isLocked) {
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
    	return -ENODEV;

    guard(mutex)(&rapl_mmio_private->lock);

    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    *isLocked = (rapl_limit & RAPL_LOCK_BIT) ? 1 : 0;

    return 0;
}


int legion_pl4_set_and_lock(struct legion_rapl_mmio_private* rapl_mmio_private,unsigned int pl4_mw) {
    unsigned int units;
    uint64_t rapl_pl4;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -ENODEV;

    /* Validate range */
    if (pl4_mw > POWER_LIMIT_MAX_VALUE_IN_MW) {
        return -EINVAL;
    }

    guard(mutex)(&rapl_mmio_private->lock);

    units = watts_to_units(pl4_mw);

    /* Atomic read-modify-write with enable and lock */
    rapl_pl4 = readq(rapl_mmio_private->rapl_mmio_base + RAPL_PL4_OFFSET);

    /* Clear power field */
    rapl_pl4 &= ~RAPL_PL4_MASK;

    /* Set new value */
    rapl_pl4 |= (units & RAPL_PL4_MASK);

    /* Enable PL4 */
    rapl_pl4 |= RAPL_PL4_ENABLE;

    /* Set lock bit */
    rapl_pl4 |= RAPL_PL4_LOCK;

    /* Write once - all changes applied atomically */
    writeq(rapl_pl4, rapl_mmio_private->rapl_mmio_base + RAPL_PL4_OFFSET);

    return 0;
}


int legion_set_and_lock(struct legion_rapl_mmio_private* rapl_mmio_private,unsigned int pl1_mw,unsigned int pl2_mw) {
    unsigned int pl1_units, pl2_units;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    /* Validate ranges */
    if (pl1_mw > POWER_LIMIT_MAX_VALUE_IN_MW) {
        return -EINVAL;
    }
    if (pl2_mw > POWER_LIMIT_MAX_VALUE_IN_MW) {
        return -EINVAL;
    }

    guard(mutex)(&rapl_mmio_private->lock);

    pl1_units = watts_to_units(pl1_mw);
    pl2_units = watts_to_units(pl2_mw);

    /* Atomic read-modify-write with lock */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear both PL1 and PL2 fields */
    rapl_limit &= ~(RAPL_PL1_MASK | RAPL_PL2_MASK);

    /* Set new values */
    rapl_limit |= (pl1_units & RAPL_PL1_MASK);
    rapl_limit |= ((uint64_t)pl2_units << RAPL_PL2_SHIFT) & RAPL_PL2_MASK;

    /* Enable both limits */
    rapl_limit |= (RAPL_PL1_ENABLE | RAPL_PL2_ENABLE);

    /* Set lock bit */
    rapl_limit |= RAPL_LOCK_BIT;

    /* Write once - all changes applied atomically */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

/* Set PL1/PL2 power and time windows, then lock */
int legion_set_power_and_time_lock(struct legion_rapl_mmio_private* rapl_mmio_private,
                                    unsigned int pl1_mw, unsigned int pl1_time_us,
                                    unsigned int pl2_mw, unsigned int pl2_time_us) {
    unsigned int pl1_units, pl2_units;
    unsigned int pl1_time_enc, pl2_time_enc;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    /* Validate ranges */
    if (pl1_mw > POWER_LIMIT_MAX_VALUE_IN_MW || pl2_mw > POWER_LIMIT_MAX_VALUE_IN_MW)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    pl1_units = watts_to_units(pl1_mw);
    pl2_units = watts_to_units(pl2_mw);
    pl1_time_enc = encode_time_window(pl1_time_us);
    pl2_time_enc = encode_time_window(pl2_time_us);

    /* Atomic read-modify-write with lock */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear PL1, PL2, and time window fields */
    rapl_limit &= ~(RAPL_PL1_MASK | RAPL_PL2_MASK | RAPL_PL1_TIME_MASK | RAPL_PL2_TIME_MASK);

    /* Set new power values */
    rapl_limit |= (pl1_units & RAPL_PL1_MASK);
    rapl_limit |= ((uint64_t)pl2_units << RAPL_PL2_SHIFT) & RAPL_PL2_MASK;

    /* Set time windows */
    rapl_limit |= ((uint64_t)pl1_time_enc << RAPL_PL1_TIME_SHIFT) & RAPL_PL1_TIME_MASK;
    rapl_limit |= ((uint64_t)pl2_time_enc << RAPL_PL2_TIME_SHIFT) & RAPL_PL2_TIME_MASK;

    /* Enable both limits */
    rapl_limit |= (RAPL_PL1_ENABLE | RAPL_PL2_ENABLE);

    /* Set lock bit */
    rapl_limit |= RAPL_LOCK_BIT;

    /* Write once - all changes applied atomically */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

/* Set PL1/PL2 power limits WITHOUT locking */
int legion_set_power(struct legion_rapl_mmio_private* rapl_mmio_private,
                     unsigned int pl1_mw, unsigned int pl2_mw) {
    unsigned int pl1_units, pl2_units;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    /* Validate ranges */
    if (pl1_mw > POWER_LIMIT_MAX_VALUE_IN_MW || pl2_mw > POWER_LIMIT_MAX_VALUE_IN_MW)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    pl1_units = watts_to_units(pl1_mw);
    pl2_units = watts_to_units(pl2_mw);

    /* Atomic read-modify-write */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear both PL1 and PL2 power fields */
    rapl_limit &= ~(RAPL_PL1_MASK | RAPL_PL2_MASK);

    /* Set new values */
    rapl_limit |= (pl1_units & RAPL_PL1_MASK);
    rapl_limit |= ((uint64_t)pl2_units << RAPL_PL2_SHIFT) & RAPL_PL2_MASK;

    /* Enable both limits */
    rapl_limit |= (RAPL_PL1_ENABLE | RAPL_PL2_ENABLE);

    /* Write without lock bit */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

/* Set PL1/PL2 power and PL1 time window WITHOUT locking */
int legion_set_power_and_time(struct legion_rapl_mmio_private* rapl_mmio_private,
                               unsigned int pl1_mw, unsigned int pl1_time_us,
                               unsigned int pl2_mw) {
    unsigned int pl1_units, pl2_units;
    unsigned int pl1_time_enc;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    /* Validate ranges */
    if (pl1_mw > POWER_LIMIT_MAX_VALUE_IN_MW || pl2_mw > POWER_LIMIT_MAX_VALUE_IN_MW)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    pl1_units = watts_to_units(pl1_mw);
    pl2_units = watts_to_units(pl2_mw);
    pl1_time_enc = encode_time_window(pl1_time_us);

    /* Atomic read-modify-write */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear PL1, PL2, and time window fields */
    rapl_limit &= ~(RAPL_PL1_MASK | RAPL_PL2_MASK | RAPL_PL1_TIME_MASK);

    /* Set new power values */
    rapl_limit |= (pl1_units & RAPL_PL1_MASK);
    rapl_limit |= ((uint64_t)pl2_units << RAPL_PL2_SHIFT) & RAPL_PL2_MASK;

    /* Set time windows */
    rapl_limit |= ((uint64_t)pl1_time_enc << RAPL_PL1_TIME_SHIFT) & RAPL_PL1_TIME_MASK;
    
    /* Enable both limits */
    rapl_limit |= (RAPL_PL1_ENABLE | RAPL_PL2_ENABLE);

    /* Write without lock bit */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

/* Set PL4 WITHOUT locking */
int legion_pl4_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl4_mw) {
    unsigned int units;
    uint64_t rapl_pl4;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -ENODEV;

    /* Validate range */
    if (pl4_mw > POWER_LIMIT_MAX_VALUE_IN_MW)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    units = watts_to_units(pl4_mw);

    /* Atomic read-modify-write */
    rapl_pl4 = readq(rapl_mmio_private->rapl_mmio_base + RAPL_PL4_OFFSET);

    /* Clear power field */
    rapl_pl4 &= ~RAPL_PL4_MASK;

    /* Set new value */
    rapl_pl4 |= (units & RAPL_PL4_MASK);

    /* Enable PL4 */
    rapl_pl4 |= RAPL_PL4_ENABLE;

    /* Write without lock bit */
    writeq(rapl_pl4, rapl_mmio_private->rapl_mmio_base + RAPL_PL4_OFFSET);

    return 0;
}

/* Set PL1 power only WITHOUT locking */
int legion_pl1_power_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl1_mw) {
    unsigned int pl1_units;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    /* Validate range */
    if (pl1_mw > POWER_LIMIT_MAX_VALUE_IN_MW)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    pl1_units = watts_to_units(pl1_mw);

    /* Atomic read-modify-write */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear PL1 power field */
    rapl_limit &= ~RAPL_PL1_MASK;

    /* Set new value */
    rapl_limit |= (pl1_units & RAPL_PL1_MASK);

    /* Enable PL1 */
    rapl_limit |= RAPL_PL1_ENABLE;

    /* Write without lock bit */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

/* Set PL2 power only WITHOUT locking */
int legion_pl2_power_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int pl2_mw) {
    unsigned int pl2_units;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    /* Validate range */
    if (pl2_mw > POWER_LIMIT_MAX_VALUE_IN_MW)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    pl2_units = watts_to_units(pl2_mw);

    /* Atomic read-modify-write */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear PL2 power field */
    rapl_limit &= ~RAPL_PL2_MASK;

    /* Set new value */
    rapl_limit |= ((uint64_t)pl2_units << RAPL_PL2_SHIFT) & RAPL_PL2_MASK;

    /* Enable PL2 */
    rapl_limit |= RAPL_PL2_ENABLE;

    /* Write without lock bit */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

/* Set PL1 time window only WITHOUT locking */
int legion_pl1_time_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int time_us) {
    unsigned int time_enc;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    time_enc = encode_time_window(time_us);

    /* Atomic read-modify-write */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear PL1 time window field */
    rapl_limit &= ~RAPL_PL1_TIME_MASK;

    /* Set new value */
    rapl_limit |= ((uint64_t)time_enc << RAPL_PL1_TIME_SHIFT) & RAPL_PL1_TIME_MASK;

    /* Write without lock bit */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

/* Set PL2 time window only WITHOUT locking */
int legion_pl2_time_set(struct legion_rapl_mmio_private* rapl_mmio_private, unsigned int time_us) {
    unsigned int time_enc;
    uint64_t rapl_limit;

    if (!rapl_mmio_private->rapl_mmio_base)
        return -EINVAL;

    guard(mutex)(&rapl_mmio_private->lock);

    time_enc = encode_time_window(time_us);

    /* Atomic read-modify-write */
    rapl_limit = readq(rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    /* Clear PL2 time window field */
    rapl_limit &= ~RAPL_PL2_TIME_MASK;

    /* Set new value */
    rapl_limit |= ((uint64_t)time_enc << RAPL_PL2_TIME_SHIFT) & RAPL_PL2_TIME_MASK;

    /* Write without lock bit */
    writeq(rapl_limit, rapl_mmio_private->rapl_mmio_base + RAPL_LIMIT_OFFSET);

    return 0;
}

