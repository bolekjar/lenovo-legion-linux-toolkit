// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Generation of the Legion firmware, used by the drivers whose WMI interface differs between the older firmware
 * (e.g. Legion 5 15IAH7H) and the newer one (e.g. Legion Pro 7 16IAX10H).
 */
#include "legion-wmi-gamezone.h"
#include "legion-wmi-other.h"
#include "legion-wmi-platform.h"

#include <linux/device.h>
#include <linux/dmi.h>
#include <linux/kernel.h>
#include <linux/string.h>

/**
 * legion_wmi_platform_legacy() - Whether the firmware is the older platform
 * @dev: Device for the log messages.
 * @err: Set to -EPROBE_DEFER when the game zone does not answer yet, otherwise 0.
 *
 * Older firmware: SmartFan version 4 / 5 or LegionZone version 1 / 2, the same rule as
 * GetSupportsGodModeV1Internal of Lenovo Legion Toolkit (the old BIOS versions are checked by
 * legion_wmi_platform_bios_supported()). Used by the CPU / GPU method, the fan method and the other mode.
 *
 * Return: true on the older platform.
 */
bool legion_wmi_platform_legacy(struct device *dev, int *err)
{
	unsigned int smartfan = 0;
	unsigned int legionzone = 0;
	unsigned int *version = &smartfan;

	/* The game zone answers when it is bound, until then the probe is deferred */
	*err = legion_wmi_other_notifier_call(version,
					      LEGION_WMI_GZ_GET_SMARTFAN_VERSION);
	if (*err) {
		*err = -EPROBE_DEFER;
		return false;
	}

	if (smartfan == 4 || smartfan == 5)
		return true;

	/* Answered by the other mode driver, failure = not supported */
	version = &legionzone;
	if (legion_wmi_other_notifier_call(version,
					   LEGION_WMI_OM_GET_LEGIONZONE_VERSION))
		legionzone = 0;

	if (legionzone == 1 || legionzone == 2)
		return true;

	dev_dbg(dev, "SmartFan version %u, LegionZone version %u, not the older platform\n",
		smartfan, legionzone);
	return false;
}

/**
 * legion_wmi_platform_bios_supported() - Whether the BIOS supports the custom mode of the older platform
 * @dev: Device for the log messages.
 *
 * Old BIOS versions without the CPU / GPU limits and the fan full speed (Lenovo Legion Toolkit: no God mode
 * there), the sensors are not affected.
 *
 * Return: false for a BIOS older than the listed versions.
 */
bool legion_wmi_platform_bios_supported(struct device *dev)
{
	static const struct {
		const char *prefix;
		unsigned int version;
	} old_bios[] = {
		{ "G9CN", 24 }, { "GKCN", 46 }, { "H1CN", 39 },
		{ "HACN", 31 }, { "HHCN", 20 },
	};
	const char *bios = dmi_get_system_info(DMI_BIOS_VERSION);
	unsigned int bios_version;

	if (!bios || strlen(bios) < 6 ||
	    kstrtouint((char[]){ bios[4], bios[5], '\0' }, 10, &bios_version))
		return true;

	for (size_t i = 0; i < ARRAY_SIZE(old_bios); i++) {
		if (!strncmp(bios, old_bios[i].prefix, 4) &&
		    bios_version < old_bios[i].version) {
			dev_info(dev, "BIOS %s older than %s%u, the custom mode is not supported\n",
				 bios, old_bios[i].prefix, old_bios[i].version);
			return false;
		}
	}

	return true;
}
