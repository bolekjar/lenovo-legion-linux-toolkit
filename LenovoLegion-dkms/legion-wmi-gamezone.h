// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_GAMEZONE_H_
#define LEGION_WMI_GAMEZONE_H_

#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/notifier.h>
#include <linux/platform_profile.h>
#include <linux/version.h>


enum gamezone_events_type {
	LEGION_WMI_GZ_GET_THERMAL_MODE 				= 1,
	LEGION_WMI_GZ_GET_SUPPORTED_THERMAL_MODES 	= 2,
	LEGION_WMI_GZ_GET_SMARTFAN_VERSION  		= 3
};

enum LEGION_GAMEZONE_METHOD_ID {

//  Other related
	GetIRTemp 						= 1,

//  N/A
	GetThermalTableID 				= 2,
	SetThermalTableID 				= 3,

// N/A
	IsSupportGpuOC 					= 4,
	GetGpuGpsState 					= 5,
	SetGpuGpsState					= 6,

//	Fan related
	GetFanCount						= 7,
	GetFan1Speed					= 8,
	GetFan2Speed        			= 9,
	GetFanMaxSpeed					= 10,

//  Other related
	GetVersion						= 11,

//  N/A
	IsSupportFanCooling 			= 12,
	SetFanCooling					= 13,

//  Others GPU related features
	IsSupportCpuOC					= 14,

// N/A
	IsBIOSSupportOC					= 15,
	SetBIOSOC						= 16,

//  Other related
	GetTriggerTemperatureValue 		= 17,

/// Others CPU related features
	GetCPUTemp						= 18,

//  Others GPU related features
	GetGPUTemp						= 19,

// Fan cooling capabilty
	GetFanCoolingStatus				= 20,

// EC disable/enable windows key capability
	IsSupportDisableWinKey			= 21,
	SetWinKeyStatus					= 22,
	GetWinKeyStatus					= 23,

// EC disable/enable touchpad capability
	IsSupportDisableTP				= 24,
	SetTPStatus						= 25,
	GetTPStatus						= 26,

//  Others GPU related features
	GetGPUPow						= 27,
	GetGPUOCPow						= 28,
	GetGPUOCType					= 29,

//  Other related
	GetKeyboardfeaturelist			= 30,
	GetMemoryOCInfo					= 31,

// N/A
	IsSupportWaterCooling			= 32,
	SetWaterCoolingStatus			= 33,
	GetWaterCoolingStatus			= 34,

// N/A
	IsSupportLightingFeature		= 35,
	SetKeyboardLight				= 36,
	GetKeyboardLight				= 37,

//  Macrokey related
	GetMacrokeyScancode				= 38,
	GetMacrokeyCount				= 39,

// G-Sync feature
	IsSupportGSync					= 40,
	GetGSyncStatus					= 41,
	SetGSyncStatus					= 42,

// Smart Fan feature
	IsSupportSmartFan				= 43,
	SetSmartFanMode					= 44,
	GetSmartFanMode					= 45,

// Other related
	GetPowerChargeMode				= 47,
	GetProductInfo					= 48,

// Over Drive feature capability
	IsSupportOD						= 49,
	GetODStatus						= 50,
	SetODStatus						= 51,

//  Other related
	SetLightControlOwner			= 52,
	SetDDSControlOwner				= 53,

// N/A
	IsRestoreOCValue				= 54,

// Other related
	GetThermalMode					= 55,

// BIOS has overclock capability
	GetBIOSOCMode					= 56,

//  Other related
	SetIntelligentSubMode			= 57,
	GetIntelligentSubMode			= 58,
	GetHardwareInfoSupportVersion	= 59,

/// Others CPU related features
	GetCpuFrequency					= 60,

//  Other related
	GetLearningProfileCount			= 61,
	IsACFitForOC					= 62,

// IGPU mode
	IsSupportIGPUMode				= 63,
	GetIGPUModeStatus				= 64,
	SetIGPUModeStatus				= 65,

//  Others GPU related features
	NotifyDGPUStatus				= 66,

//  Other related
	IsChangedYLog					= 67,

//  Others GPU related features
	GetDGPUHWId						= 68
};


enum thermal_mode {
	LEGION_WMI_GZ_THERMAL_MODE_END          = 	   0x00,
	LEGION_WMI_GZ_THERMAL_MODE_QUIET 		=	   0x01,
	LEGION_WMI_GZ_THERMAL_MODE_BALANCED 	=	   0x02,
	LEGION_WMI_GZ_THERMAL_MODE_PERFORMANCE 	= 	   0x03,
	LEGION_WMI_GZ_THERMAL_MODE_EXTREME 		=	   0xE0, /* Ver 6+ */
	LEGION_WMI_GZ_THERMAL_MODE_CUSTOM 		=	   0xFF
};

#define THERMAL_MODES_SIZE 6

enum power_adapter_status
{
	LEGION_WMI_GZ_AC_CONNECTED              = 0x00,
	LEGION_WMI_GZ_AC_CONNECTED_LOW_WATTAGE  = 0x01,
	LEGION_WMI_GZ_AC_DISCONNECTED           = 0x02
};

#define DEFAULT_THERMAL_MODE LEGION_WMI_GZ_THERMAL_MODE_BALANCED

struct game_zone_preloaded_method_values
{
	unsigned int IsSupportGpuOC;
	unsigned int IsSupportCpuOC;
	unsigned int IsSupportDisableWinKey;
	unsigned int IsSupportDisableTP;
	unsigned int IsSupportGSync;
	unsigned int IsSupportSmartFan;
	unsigned int IsSupportOD;
	unsigned int IsSupportIGPUMode;


	unsigned int GetFanCount;
	unsigned int GetFanMaxSpeed;
};

struct lenovo_wmi_gz_priv {
	spinlock_t gz_mode_lock; /* current_mode lock */

	enum thermal_mode current_mode_on_ac;
	enum thermal_mode current_mode_on_battery;

	enum power_adapter_status current_adapter_status;
	bool  extreme_supported;

	struct notifier_block event_nb;
	struct notifier_block other_nb;
	struct notifier_block fm_nb;

	struct wmi_device *wdev;
	struct device *ppdev;

	//SysFs
	struct device *fw_attr_dev;
	struct kset   *fw_attr_kset;
	int ida_id;

	struct game_zone_preloaded_method_values preloaded_values; /* Cached value to avoid WMI calls in sysfs */


#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 14, 0)
    struct platform_profile_handler 	platform_profile_handler;
#endif

};

struct device;

int legion_wmi_gz_get(struct wmi_device *wdev,enum LEGION_GAMEZONE_METHOD_ID method_id,u32 *value);
int legion_wmi_gz_set(struct wmi_device *wdev,enum LEGION_GAMEZONE_METHOD_ID method_id,u32 value);
int legion_wmi_gz_get_string(struct wmi_device *wdev,enum LEGION_GAMEZONE_METHOD_ID method_id,char *retval,size_t max_retval_size);


int  legion_wmi_gamezone_driver_init(struct device *parent);
void legion_wmi_gamezone_driver_exit(void);

#endif /* LEGION_WMI_GAMEZONE_H_ */
