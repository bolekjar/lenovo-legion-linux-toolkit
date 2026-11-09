// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-kernel-version.h"
#include "legion-common.h"
#include "legion-compatibility.h"
#include "legion-wmi-events.h"
#include "legion-wmi-gamezone.h"
#include "legion-wmi-capdata00.h"
#include "legion-wmi-capdata01.h"
#include "legion-wmi-other.h"
#include "legion-wmi-ddata.h"
#include "legion-firmware-attributes-class.h"
#include "legion-hwmon.h"
#include "legion-wmi-ftable.h"
#include "legion-wmi-fm.h"

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/acpi.h>
#include <linux/dmi.h>
#include <linux/component.h>

static int legion_pm_resume(struct device *dev)
{
	dev_dbg(dev, "PM  Resuming Lenovo Legion platform driver\n");

	return 0;
}

static SIMPLE_DEV_PM_OPS(legion_pm, NULL, legion_pm_resume);

static int legion_probe(struct platform_device *pdev)
{
	int err = 0;
	struct legion_data *data = NULL;

	dev_info(&pdev->dev, "Legion platform driver probing:\n");

	err = legion_firmware_attributes_class_init();
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create firmware attributes: %d\n", err);
		return err;
	}
	dev_info(&pdev->dev, "\tLegion firmware attributes was initialized\n");

	data = devm_kzalloc(&pdev->dev, sizeof(struct legion_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto err_enomem;
	}

	err = legion_data_init(data);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create legion data structure: %d\n", err);
		goto err_legion_init;
	}
	dev_info(&pdev->dev, "\tLegion data structure was initialized\n");

	platform_set_drvdata(pdev, data);

	read_dmi_machine_information(&data->machine_info.dmi_info);
	if (!is_compatible(&data->machine_info.dmi_info)) {
		dev_info(&pdev->dev, "\tIncompatible model !\n");
		/* Probe must fail: with 0 the driver stays bound and legion_remove() cleans up freed data */
		err = -ENODEV;
		goto err_model_mismatach;
	}

	print_machine_information(&pdev->dev, &data->machine_info);

	/*
	 * WMI gamezone
	 */
	err = legion_wmi_gamezone_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create WMI game zone driver: %d\n", err);
		goto err_wmi_gamezone;
	}
	dev_info(&pdev->dev, "\tWMI game zone driver was initialized\n");

	/*
	 * WMI events
	 */
	err = legion_wmi_events_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create WMI events driver: %d\n", err);
		goto err_wmi_events;
	}
	dev_info(&pdev->dev, "\tWMI events driver was initialized\n");

	/*
	 * WMI cd00
	 */
	err = legion_wmi_cd00_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create WMI capability data 00 driver: %d\n",
			err);
		goto err_wmi_cd00;
	}
	dev_info(&pdev->dev,
		 "\tWMI capability data 00 driver was initialized\n");

	/*
	 * WMI cd01
	 */
	err = legion_wmi_cd01_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create WMI capability data 01 driver: %d\n",
			err);
		goto err_wmi_cd01;
	}
	dev_info(&pdev->dev,
		 "\tWMI capability data 01 driver was initialized\n");

	/*
	 * WMI ddata
	 */
	err = legion_wmi_dd_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create WMI discrete data driver: %d\n",
			err);
		goto err_wmi_dd;
	}
	dev_info(&pdev->dev, "\tWMI discrete data driver was initialized\n");

	/*
	 * WMI other
	 */
	err = legion_wmi_other_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev, "\tFailed to create WMI other driver: %d\n",
			err);
		goto err_wmi_other;
	}
	dev_info(&pdev->dev, "\tWMI other driver was initialized\n");

	/*
	 * Fan Table
	 */
	err = legion_wmi_ftable_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create WMI Fan table driver: %d\n", err);
		goto err_ftable;
	}
	dev_info(&pdev->dev, "\tWMI Fan table driver was initialized\n");

	/*
	 * Fan Method
	 */
	err = legion_wmi_fm_driver_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev,
			"\tFailed to create WMI Fan method driver: %d\n", err);
		goto err_fm;
	}
	dev_info(&pdev->dev, "\tWMI Fan method driver was initialized\n");

	/*
	 * HwMon
	 */
	err = legion_hwmon_init(&pdev->dev);
	if (err) {
		dev_err(&pdev->dev, "\tFailed to create HwMon driver: %d\n",
			err);
		goto err_hwmon;
	}
	dev_info(&pdev->dev, "\tHwMon driver was initialized\n");

	/*
	 * Intel MSR is the separate module legion_intel_msr (not tied to the notebook).
	 * RAPL power limits of the thermal modes are written by the daemon (powercap intel-rapl, intel-rapl-mmio).
	 */

	dev_info(&pdev->dev, "Legion platform driver was loaded\n");
	return 0;
err_hwmon:
	legion_wmi_fm_driver_exit();
err_fm:
	legion_wmi_ftable_driver_exit();
err_ftable:
	legion_wmi_other_driver_exit();
err_wmi_other:
	legion_wmi_dd_driver_exit();
err_wmi_dd:
	legion_wmi_cd01_driver_exit();
err_wmi_cd01:
	legion_wmi_cd00_driver_exit();
err_wmi_cd00:
	legion_wmi_events_driver_exit();
err_wmi_events:
	legion_wmi_gamezone_driver_exit();
err_wmi_gamezone:
err_model_mismatach:
	legion_data_exit(data);
err_legion_init:
err_enomem:
	legion_firmware_attributes_class_exit();
	dev_err(&pdev->dev, "Legion platform not loaded for this device\n");
	return err;
}

static void legion_remove(struct platform_device *pdev)
{
	struct legion_data *data = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "Lenovo Legion platform driver removing:\n");

	legion_hwmon_exit(&pdev->dev);
	dev_info(&pdev->dev, "\tHwMon driver was unregistered\n");

	legion_wmi_fm_driver_exit();
	dev_info(&pdev->dev, "\tWMI Fan method was unregistered\n");

	legion_wmi_ftable_driver_exit();
	dev_info(&pdev->dev, "\tWMI Fan table was unregistered\n");

	legion_wmi_other_driver_exit();
	dev_info(&pdev->dev, "\tWMI other driver was unregistered\n");

	legion_wmi_dd_driver_exit();
	dev_info(&pdev->dev, "\tWMI discrete data was unregistered\n");

	legion_wmi_cd01_driver_exit();
	dev_info(&pdev->dev,
		 "\tWMI capability data 01 driver was unregistered\n");

	legion_wmi_cd00_driver_exit();
	dev_info(&pdev->dev,
		 "\tWMI capability data 00 driver was unregistered\n");

	legion_wmi_gamezone_driver_exit();
	dev_info(&pdev->dev, "\tWMI game zone driver was unregistered\n");

	legion_wmi_events_driver_exit();
	dev_info(&pdev->dev, "\tWMI events driver was unregistered\n");

	legion_data_exit(data);
	dev_info(&pdev->dev, "\tLegion data structure was unregistered\n");

	legion_firmware_attributes_class_exit();
	dev_info(&pdev->dev, "\tLegion firmware attributes was unregistered\n");

	dev_info(&pdev->dev, "Legion platform driver was removed\n");
}

static int legion_resume(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "Resuming Lenovo Legion platform driver\n");
	return 0;
}

static void legion_shutdown(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "Shutting down Lenovo Legion platform driver\n");
}

static int legion_suspend(struct platform_device *pdev, pm_message_t state)
{
	dev_dbg(&pdev->dev, "Suspending Lenovo Legion platform driver\n");
	return 0;
}

/*
 * VPC2004 Virtual power connector
 */
static const struct acpi_device_id legion_device_ids[] = {
	{ "VPC2004", 0 },
	{ "", 0 },
};

static struct platform_driver legion_driver = {
	.probe = legion_probe,
	.remove = legion_remove,
	.shutdown = legion_shutdown,
	.suspend = legion_suspend,
	.resume = legion_resume,
	.driver = { .name = "lenovo-legion",
		    .pm = &legion_pm,
		    .acpi_match_table = ACPI_PTR(legion_device_ids),
		    .owner = THIS_MODULE }
};

static int __init legion_init(void)
{
	const int err = platform_driver_register(&legion_driver);

	if (err)
		return err;
	return 0;
}

static void __exit legion_exit(void)
{
	platform_driver_unregister(&legion_driver);
}

module_init(legion_init);
module_exit(legion_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaroslav Bolek");
MODULE_DESCRIPTION("Lenovo Legion Pro 7 16IRX9H DKMS driver");
MODULE_VERSION("0.9.10");
MODULE_DEVICE_TABLE(acpi, legion_device_ids);
