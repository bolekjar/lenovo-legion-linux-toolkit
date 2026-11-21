/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_WMI_FM_H_
#define LEGION_WMI_FM_H_

#include "legion-wmi-gamezone.h"

#include <linux/notifier.h>

struct legion_hwmon;
struct wmi_device;
struct kset;

#define FAN_MAX 20

/*
 * Fan table of the custom mode: points, every point is a level
 * MIN_FANCURVE_LEVEL..MAX_FANCURVE_LEVEL (the index level - 1 of the fan speeds
 * of the fan table data), limited by the firmware
 * (legion_wmi_ftable_curve_limits())
 */
#define MAX_FANCURVE_SIZE 10
#define MIN_FANCURVE_LEVEL 1
#define MAX_FANCURVE_LEVEL 10

/*
 * Methods of LENOVO_FAN_METHOD (binary MOF of the firmware). The newer firmware (e.g. 16IAX10H) has only the fan
 * table (5, 6), the others are on the older firmware only (e.g. 15IAH7H); the newer firmware provides them by
 * the other mode (capability data, GetFeatureValue).
 */

// set fan to maximal speed; dust cleaning mode, only works in custom power mode
// Fan_Get_FullSpeed([out] boolean Status), Fan_Set_FullSpeed([in] boolean Status)
#define WMI_METHOD_ID_FAN_GET_FULLSPEED 1
#define WMI_METHOD_ID_FAN_SET_FULLSPEED 2

// max speed of fan, not used (meaning of the bytes unknown)
// Fan_Get_MaxSpeed([in] uint8 Fan_ID, [out] uint32 FanMaxSpeedSize, [out] uint8 FanMaxSpeed[])
// Fan_Set_MaxSpeed([in, Max(3)] uint8 FanMaxSpeedTable[])
#define WMI_METHOD_ID_FAN_GET_MAXSPEED 3
#define WMI_METHOD_ID_FAN_SET_MAXSPEED 4

// fan table in custom mode
// Fan_Get_Table([in] uint8 FanID, [in] uint8 SensorID, ...): newer firmware uint32 FanTable[] and
// SensorTable[], older firmware uint8 FanTable[] only
// Fan_Set_Table([in, Max(64)] uint8 FanTable[]): both
#define WMI_METHOD_ID_FAN_GET_TABLE 5
#define WMI_METHOD_ID_FAN_SET_TABLE 6

// get speed of fans
// Fan_GetCurrentFanSpeed([in] uint8 FanID, [out] uint16 CurrentFanSpeed)
#define WMI_METHOD_ID_FAN_GET_CURRENT_FAN_SPEED 7
// get temperatures of CPU and GPU used for controlling cooling
// Fan_GetCurrentSensorTemperature([in] uint8 SensorID, [out] uint16 CurrentSensorTemperature)
#define WMI_METHOD_ID_FAN_GET_CURRENT_SENSOR_TEMPERATURE 8
// set speed of a fan, not used
// Fan_SetCurrentFanSpeed([in] uint8 FanID, [in] uint16 CurrentFanSpeed)
#define WMI_METHOD_ID_FAN_SET_CURRENT_FAN_SPEED 9

enum FanTableType { CPU = 0, CPUSensor = 1, GPU = 2, SYS = 3 };

enum TableType { FAN = 0, SENSOR = 1 };

/* Rows (fan, sensor) of the fan table indexed by enum FanTableType, SmartFan version 8 uses v2 */
extern const int fan_table_id_v1[];
extern const int fan_sensor_id_v1[];

extern const int fan_table_id_v2[];
extern const int fan_sensor_id_v2[];

enum fm_events_type {
	LEGION_WMI_FM_GET_CPU_FAN_MAX_SPEED = 0,
	LEGION_WMI_FM_GET_GPU_FAN_MAX_SPEED = 1,
	LEGION_WMI_FM_GET_SYS_FAN_MAX_SPEED = 2
};

struct fan_max_speed {
	int fan_id;
	int sensor_id;
	int max_speed;
};

struct legion_wmi_fm_priv {
	struct wmi_device *wdev;

	struct notifier_block
		hwmon_nb; /* source of the hwmon maximal fan speeds */

	/* Older firmware: source of the hwmon sensors and the hwmon device (the other mode does not provide them) */
	struct notifier_block sensors_nb;
	struct legion_hwmon *hwmon;

	struct legion_wmi_ftable_list *ft_list; /* only valid after ft bind */
	struct fan_max_speed
		fan_max_speeds[FAN_MAX]; /* only valid after ft bind */

	//SysFs
	struct device *fw_attr_dev;
	struct kset *fw_attr_kset;
	int ida_id;
	unsigned int smart_fan_version;
	bool legacy_platform; /* older firmware, see legion_wmi_platform_legacy() */
	bool fan_full_speed; /* Fan_Get/Set_FullSpeed of the older firmware */

	/* Limits of the points of the fan curve (legion_wmi_ftable_curve_limits()) */
	u8 fan_curve_min[MAX_FANCURVE_SIZE];
	u8 fan_curve_max[MAX_FANCURVE_SIZE];

	int fan_cpu_max_speed;
	int fan_gpu_max_speed;
	int fan_sys_max_speed;
};

int legion_wmi_fm_notifier_call(void *data,
				enum gamezone_events_type gamezone_events_type);
int devm_lenovo_wmi_fm_register_notifier(struct device *dev,
					 struct notifier_block *nb);

int legion_wmi_fm_table_get(struct legion_wmi_fm_priv *priv,
			    u32 points[MAX_FANCURVE_SIZE]);
int legion_wmi_fm_table_set(struct legion_wmi_fm_priv *priv,
			    const u8 points[MAX_FANCURVE_SIZE]);

int legion_wmi_fm_full_speed_get(struct legion_wmi_fm_priv *priv, bool *on);
int legion_wmi_fm_full_speed_set(struct legion_wmi_fm_priv *priv, bool on);

int legion_wmi_fm_driver_init(void);
void legion_wmi_fm_driver_exit(void);

#endif /* LEGION_WMI_FM_H_ */
