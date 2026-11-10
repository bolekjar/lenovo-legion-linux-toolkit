// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Lenovo Legion notebooks: WMI game zone (thermal mode, platform_profile), other mode (power limits and features),
 * capability data, fan curve, events (uevents), hwmon (under the other mode device).
 *
 * The module has no device of its own: module_init checks the model by DMI and registers the WMI drivers, the
 * component drivers (capability data, discrete data, fan table) before their masters (other mode, fan method).
 * The module is loaded by the aliases of the WMI GUIDs. The ACPI device VPC2004 belongs to ideapad_laptop, which
 * has to stay blacklisted: it binds the WMI GUID 8FC0DE0C (Fn keys) used by the events driver as well.
 */
#include "legion-kernel-version.h"
#include "legion-firmware-attributes-class.h"
#include "legion-wmi-capdata00.h"
#include "legion-wmi-capdata01.h"
#include "legion-wmi-ddata.h"
#include "legion-wmi-events.h"
#include "legion-wmi-fm.h"
#include "legion-wmi-ftable.h"
#include "legion-wmi-gamezone.h"
#include "legion-wmi-other.h"

#include <linux/dmi.h>
#include <linux/init.h>
#include <linux/module.h>

#define LEGION_MODEL(_family)                                       \
	{                                                           \
		.matches = {                                        \
			DMI_EXACT_MATCH(DMI_BIOS_VENDOR, "LENOVO"), \
			DMI_MATCH(DMI_PRODUCT_FAMILY, _family),     \
		},                                                  \
	}

/*
 * Supported models: part of the product family (e.g. "Legion Pro 7 16IAX10H")
 */
static const struct dmi_system_id legion_models[] __initconst = {
	/* Worldwide variants */
	LEGION_MODEL("17ACH"),
	LEGION_MODEL("17ARH"),
	LEGION_MODEL("17ITH"),
	LEGION_MODEL("17IMH"),
	LEGION_MODEL("16ACH"),
	LEGION_MODEL("16AHP"),
	LEGION_MODEL("16APH"),
	LEGION_MODEL("16ARH"),
	LEGION_MODEL("16ARP"),
	LEGION_MODEL("16ARX"),
	LEGION_MODEL("16IAH"),
	LEGION_MODEL("16IAX"),
	LEGION_MODEL("16IRH"),
	LEGION_MODEL("16IRX"),
	LEGION_MODEL("16ITH"),
	LEGION_MODEL("15ACH"),
	LEGION_MODEL("15AHP"),
	LEGION_MODEL("15APH"),
	LEGION_MODEL("15ARH"),
	LEGION_MODEL("15ARP"),
	LEGION_MODEL("15IAH"),
	LEGION_MODEL("15IAX"),
	LEGION_MODEL("15IHU"),
	LEGION_MODEL("15IMH"),
	LEGION_MODEL("15IRH"),
	LEGION_MODEL("15ITH"),
	LEGION_MODEL("14APH"),
	LEGION_MODEL("14IRP"),
	/* Chinese variants */
	LEGION_MODEL("G5000"),
	LEGION_MODEL("R9000"),
	LEGION_MODEL("R7000"),
	LEGION_MODEL("Y9000"),
	LEGION_MODEL("Y7000"),
	/* Limited compatibility */
	LEGION_MODEL("17IR"),
	LEGION_MODEL("15IR"),
	LEGION_MODEL("15IC"),
	LEGION_MODEL("15IK"),
	{}
};

struct legion_driver {
	const char *name;
	int (*init)(void);
	void (*exit)(void);
};

/*
 * Components before their masters, so the masters bind at once
 */
static const struct legion_driver legion_drivers[] = {
	{ "capability data 00", legion_wmi_cd00_driver_init,
	  legion_wmi_cd00_driver_exit },
	{ "capability data 01", legion_wmi_cd01_driver_init,
	  legion_wmi_cd01_driver_exit },
	{ "discrete data", legion_wmi_dd_driver_init, legion_wmi_dd_driver_exit },
	{ "fan table", legion_wmi_ftable_driver_init,
	  legion_wmi_ftable_driver_exit },
	{ "events", legion_wmi_events_driver_init, legion_wmi_events_driver_exit },
	{ "game zone", legion_wmi_gamezone_driver_init,
	  legion_wmi_gamezone_driver_exit },
	{ "other mode", legion_wmi_other_driver_init,
	  legion_wmi_other_driver_exit },
	{ "fan method", legion_wmi_fm_driver_init, legion_wmi_fm_driver_exit },
};

static void legion_drivers_exit(size_t count)
{
	while (count--)
		legion_drivers[count].exit();
}

static int __init legion_init(void)
{
	size_t i;
	int err;

	if (!dmi_first_match(legion_models))
		return -ENODEV;

	err = legion_firmware_attributes_class_init();
	if (err)
		return err;

	for (i = 0; i < ARRAY_SIZE(legion_drivers); i++) {
		err = legion_drivers[i].init();
		if (err) {
			pr_err("lenovo_legion: WMI %s driver not registered: %d\n",
			       legion_drivers[i].name, err);
			goto err_drivers;
		}
	}

	return 0;

err_drivers:
	legion_drivers_exit(i);
	legion_firmware_attributes_class_exit();
	return err;
}

static void __exit legion_exit(void)
{
	legion_drivers_exit(ARRAY_SIZE(legion_drivers));
	legion_firmware_attributes_class_exit();
}

module_init(legion_init);
module_exit(legion_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaroslav Bolek");
MODULE_DESCRIPTION(
	"Lenovo Legion notebooks: WMI game zone, other mode, fan curve, hwmon");
MODULE_VERSION("0.9.10");
