// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef _LEGION_WMI_H
#define _LEGION_WMI_H

#include "legion-wmi-gamezone.h"

#include <linux/types.h>

#define LEGION_WMI_OTHER_FEATURE_VALUE_GET 17
#define LEGION_WMI_OTHER_FEATURE_VALUE_SET 18

#define LEGION_WMI_MODE_ID_MASK 	 GENMASK(15, 8)

/*
 * DEV_ID=31,24|FEAT_ID=23,16|MODE_ID=15,8 |TYPE_ID=7, 0
 * 0x01 = CPU  |0x02 = LTP   |0xFF = CUSTOM|0x00 = N/A
 * 0x04 = FAN  |0x03 = CFS   |0x00 = RAW   |0x01 = CPU/02 = GPU
 */
enum CapabilityID
{

	//IGPU mode related settings
    IGPUMode 						 					= 0x00010000,

    //Other related settings
    FlipToStart 					 					= 0x00030000,
	NvidiaGPUDynamicDisplaySwitching 					= 0x00040000,

	//AMD related settings
    AMDSmartShiftMode 				 					= 0x00050001,
    AMDSkinTemperatureTracking 		 					= 0x00050002,

	//Other related settings
	SupportedPowerModes 								= 0x00070000,

	//Other related settings
    LegionZoneSupportVersion 							= 0x00090000,

	//Other related settings
    GodModeFnQSwitchable 								= 0x00100000,

	//Other related settings
    OverDrive 											= 0x001A0000,

	//Other related settings N/A
    AIChip 												= 0x000E0000,
	//IGPU mode related settings
	IGPUModeChangeStatus 								= 0x000F0000,



	//Set the CPU short term power limit
	CPUShortTermPowerLimit 								= 0x0101FF00,
	//Set the CPU long term power limit
	CPULongTermPowerLimit 								= 0x0102FF00,
	//Set the CPU peak power limit
    CPUPeakPowerLimit 									= 0x0103FF00,
	//Set the CPU temperature limit
	CPUTemperatureLimit 								= 0x0104FF00,
	//Set the APUs ppt power limit
	APUsPPTPowerLimit 									= 0x0105FF00,
	//Set the CPU cross loading power limit
	CPUCrossLoadingPowerLimit 							= 0x0106FF00,
	//Set the CPU PL1 Tau limit
	CPUPL1Tau 											= 0x0107FF00,



	//Set the GPU power boost
    GPUPowerBoost 										= 0x0201FF00,
	//Set the GPU configurable TGP
    GPUConfigurableTGP 									= 0x0202FF00,
    //Set the GPU temperature limit
	GPUTemperatureLimit 								= 0x0203FF00,
	//Set the GPU total processing power target on AC offset from base line limit
    GPUTotalProcessingPowerTargetOnAcOffsetFromBaseline = 0x0204FF00,
	//Set the GPU To CPU dynamic boost
    GPUToCPUDynamicBoost 								= 0x020BFF00,

	//GPU other related settings
    GPUStatus 											= 0x02070000,
    GPUDidVid 											= 0x02090000,

	//Instant boot related settings
	InstantBootAc 										= 0x03010001,
    InstantBootUsbPowerDelivery 						= 0x03010002,

	//Other related settings
	FanFullSpeed										= 0x04020000,

	//HwMon
	CpuCurrentFanSpeed 									= 0x04030001,
    GpuCurrentFanSpeed 									= 0x04030002,
	SysCurrentFanSpeed 									= 0x04030004,
	SysCurrentTemperature 								= 0x05010000,
    CpuCurrentTemperature 								= 0x05040000,
    GpuCurrentTemperature 								= 0x05050000
};


struct legion_wmi_other_priv {
	struct legion_wmi_cd00_list 	*cd00_list; /* only valid after capdata01 bind */
	struct legion_wmi_cd01_list 	*cd01_list; /* only valid after capdata01 bind */
    struct legion_wmi_dd_list 		*dd_list;   /* only valid after ddata bind */
	struct wmi_device 				*wdev;

	struct notifier_block 			hwmon_nb;    /* hwmon*/
	struct notifier_block 			dkms_nb;     /* dkms*/

	//SysFs
	struct device *fw_attr_dev;
	struct kset   *fw_attr_kset;
	int ida_id;

	/* Completion to signal when binding is done */
	struct completion bind_complete;
};

struct device;
struct notifier_block;

int legion_wmi_other_notifier_call(void *data,enum gamezone_events_type gamezone_events_type);

int devm_lenovo_wmi_other_register_notifier(struct device *dev,struct notifier_block *nb);

int legion_wmi_other_driver_init(struct device *dev);
void legion_wmi_other_driver_exit(void);

#endif
