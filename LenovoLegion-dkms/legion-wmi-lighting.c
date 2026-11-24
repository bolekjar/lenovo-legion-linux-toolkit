// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Lights of the notebook controlled by the firmware: WMI data block
 * LENOVO_LIGHTING_DATA (binary MOF, ASL WQA4), one instance for each light
 * (keyboard, Y logo, ports, ...). Lenovo Legion Toolkit switches the Y logo
 * (ID 3) and the ports (ID 5) by LENOVO_LIGHTING_METHOD
 * (Get / Set_Lighting_Current_Status). A light of the control interface 0 is
 * controlled by USB (RGB keyboard, Lenovo Spectrum), not by the firmware.
 *
 * For now the instances are only read and logged (dynamic debug), the lights of
 * the firmware of the notebooks are not known yet.
 */
#include "legion-wmi-lighting.h"

#include <linux/acpi.h>
#include <linux/build_bug.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wmi.h>

#define LEGION_WMI_LIGHTING_DATA_GUID "4DD5BD84-15A9-47E2-AD65-CC61A5C62FD0"

/* Light not present on the notebook (ASL WQA4: Lighting_Id 0xFF) */
#define LEGION_WMI_LIGHTING_ID_NONE 0xff

/*
 * Instance of the data block (binary MOF and ASL LTDA)
 */
struct legion_wmi_lighting_data {
	u8 id; /* Lighting_Id */
	u8 control_interface; /* Control_Interface */
	u8 type; /* Lighting_Type */
	u8 state_type_num; /* State_Type_Num */
	u8 default_state; /* Default_State */
	u8 brightness_level; /* Brightness_Level */
	u8 default_brightness_level; /* Default_Brightness_Level */
} __packed;
static_assert(sizeof(struct legion_wmi_lighting_data) == 7);

/*
 * Read and log the instances of the data block
 */
static int legion_wmi_lighting_data_read(struct wmi_device *wdev)
{
	const int count = wmidev_instance_count(wdev);
	struct legion_wmi_lighting_data data;

	if (count <= 0)
		return -ENODEV;

	for (int idx = 0; idx < count; idx++) {
		union acpi_object *obj __free(kfree) =
			wmidev_block_query(wdev, idx);

		if (!obj || obj->type != ACPI_TYPE_BUFFER ||
		    obj->buffer.length < sizeof(data)) {
			dev_dbg(&wdev->dev, "light %d: no data\n", idx);
			continue;
		}

		memcpy(&data, obj->buffer.pointer, sizeof(data));

		if (data.id == LEGION_WMI_LIGHTING_ID_NONE) {
			dev_dbg(&wdev->dev, "light %d: not present\n", idx);
			continue;
		}

		dev_dbg(&wdev->dev,
			"light %d: ID %u, control interface %u, type %u, states %u, default state %u, brightness levels %u, default brightness %u\n",
			idx, data.id, data.control_interface, data.type,
			data.state_type_num, data.default_state,
			data.brightness_level, data.default_brightness_level);
	}

	return 0;
}

static int legion_wmi_lighting_data_probe(struct wmi_device *wdev,
					  const void *context)
{
	return legion_wmi_lighting_data_read(wdev);
}

static const struct wmi_device_id legion_wmi_lighting_data_id_table[] = {
	{ LEGION_WMI_LIGHTING_DATA_GUID, NULL },
	{}
};

static struct wmi_driver legion_wmi_lighting_data_driver = {
	.driver = {
		.name = "legion_wmi_lighting_data",
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.id_table = legion_wmi_lighting_data_id_table,
	.probe = legion_wmi_lighting_data_probe,
	.no_singleton = true,
};

int legion_wmi_lighting_data_driver_init(void)
{
	return wmi_driver_register(&legion_wmi_lighting_data_driver);
}

void legion_wmi_lighting_data_driver_exit(void)
{
	wmi_driver_unregister(&legion_wmi_lighting_data_driver);
}

MODULE_DEVICE_TABLE(wmi, legion_wmi_lighting_data_id_table);
