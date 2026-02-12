// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QSettings>


#include "../LenovoLegion-PrepareBuild/PowerProfile.pb.h"
#include "../LenovoLegion-PrepareBuild/CPUOptions.pb.h"
#include "../LenovoLegion-PrepareBuild/CPUFrequency.pb.h"
#include "../LenovoLegion-PrepareBuild/FanControl.pb.h"
#include "../LenovoLegion-PrepareBuild/CpuPower.pb.h"
#include "../LenovoLegion-PrepareBuild/GPUPower.pb.h"
#include "../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h"
#include "../LenovoLegion-PrepareBuild/CpuIntelMSR.pb.h"
#include "../LenovoLegion-PrepareBuild/DaemonSettings.pb.h"
#include "../LenovoLegion-PrepareBuild/Other.pb.h"


namespace LenovoLegionDaemon {


class Settings {

protected:


public:

    explicit Settings(const QString& group);
    virtual ~Settings();

    Settings(Settings&)  = delete;
    Settings(Settings&&) = delete;

    Settings& operator=(Settings&)  = delete;
    Settings& operator=(Settings&&) = delete;


protected:

    QSettings m_settings;
};


class SettingsLoaderPowerProfiles: protected Settings
{
public:
    explicit SettingsLoaderPowerProfiles();
    SettingsLoaderPowerProfiles& loadPowerProfile(legion::messages::PowerProfile& profile);
};


class SettingsSaverPowerProfiles: protected Settings
{
public:
    explicit SettingsSaverPowerProfiles();
    SettingsSaverPowerProfiles& saverPowerProfile(const legion::messages::PowerProfile& profile);
};




class SettingsLoaderCPUControlData: protected Settings
{
public:
    explicit SettingsLoaderCPUControlData();
    SettingsLoaderCPUControlData& loadPowerProfile(legion::messages::CPUOptions &profile);
};

class SettingsSaverCPUControlData: protected Settings
{
public:
    explicit SettingsSaverCPUControlData();
    SettingsSaverCPUControlData& saverPowerProfile(const legion::messages::CPUOptions &profile);
};

// CPU Frequency Settings
class SettingsLoaderCPUFrequency: protected Settings
{
public:
    explicit SettingsLoaderCPUFrequency();
    SettingsLoaderCPUFrequency& loadCPUFrequency(legion::messages::CPUFrequency &frequency);
};

class SettingsSaverCPUFrequency: protected Settings
{
public:
    explicit SettingsSaverCPUFrequency();
    SettingsSaverCPUFrequency& saveCPUFrequency(const legion::messages::CPUFrequency &frequency);
};

// Fan Curve Settings
class SettingsLoaderFanCurve: protected Settings
{
public:
    explicit SettingsLoaderFanCurve();
    SettingsLoaderFanCurve& loadFanCurve(legion::messages::FanCurve &fanCurve);
};

class SettingsSaverFanCurve: protected Settings
{
public:
    explicit SettingsSaverFanCurve();
    SettingsSaverFanCurve& saveFanCurve(const legion::messages::FanCurve &fanCurve);
};

// Fan Option Settings
class SettingsLoaderFanOption: protected Settings
{
public:
    explicit SettingsLoaderFanOption();
    SettingsLoaderFanOption& loadFanOption(legion::messages::FanOption &fanOption);
};

class SettingsSaverFanOption: protected Settings
{
public:
    explicit SettingsSaverFanOption();
    SettingsSaverFanOption& saveFanOption(const legion::messages::FanOption &fanOption);
};

// CPU SMT Settings
class SettingsLoaderCPUSMT: protected Settings
{
public:
    explicit SettingsLoaderCPUSMT();
    SettingsLoaderCPUSMT& loadCPUSMT(legion::messages::CPUSMT &cpuSmt);
};

class SettingsSaverCPUSMT: protected Settings
{
public:
    explicit SettingsSaverCPUSMT();
    SettingsSaverCPUSMT& saveCPUSMT(const legion::messages::CPUSMT &cpuSmt);
};

// CPU Power Settings
class SettingsLoaderCPUPower: protected Settings
{
public:
    explicit SettingsLoaderCPUPower();
    SettingsLoaderCPUPower& loadCPUPower(legion::messages::CPUPower &cpuPower);
};

class SettingsSaverCPUPower: protected Settings
{
public:
    explicit SettingsSaverCPUPower();
    SettingsSaverCPUPower& saveCPUPower(const legion::messages::CPUPower &cpuPower);
};

// GPU Power Settings
class SettingsLoaderGPUPower: protected Settings
{
public:
    explicit SettingsLoaderGPUPower();
    SettingsLoaderGPUPower& loadGPUPower(legion::messages::GPUPower &gpuPower);
};

class SettingsSaverGPUPower: protected Settings
{
public:
    explicit SettingsSaverGPUPower();
    SettingsSaverGPUPower& saveGPUPower(const legion::messages::GPUPower &gpuPower);
};

// Nvidia NVML Settings
class SettingsLoaderNvidiaNvml: protected Settings
{
public:
    explicit SettingsLoaderNvidiaNvml();
    SettingsLoaderNvidiaNvml& loadNvidiaNvml(legion::messages::NvidiaNvml &nvidiaNvml);
};

class SettingsSaverNvidiaNvml: protected Settings
{
public:
    explicit SettingsSaverNvidiaNvml();
    SettingsSaverNvidiaNvml& saveNvidiaNvml(const legion::messages::NvidiaNvml &nvidiaNvml);
};

// Intel MSR Settings
class SettingsLoaderIntelMSR: protected Settings
{
public:
    explicit SettingsLoaderIntelMSR();
    SettingsLoaderIntelMSR& loadIntelMSR(legion::messages::CpuIntelMSR &intelMSR);
};

class SettingsSaverIntelMSR: protected Settings
{
public:
    explicit SettingsSaverIntelMSR();
    SettingsSaverIntelMSR& saveIntelMSR(const legion::messages::CpuIntelMSR &intelMSR);
};

// Daemon Settings
class SettingsLoaderDaemonSettings: protected Settings
{
public:
    explicit SettingsLoaderDaemonSettings();
    SettingsLoaderDaemonSettings& loadDaemonSettings(legion::messages::DaemonSettings &daemonSettings);
};

class SettingsSaverDaemonSettings: protected Settings
{
public:
    explicit SettingsSaverDaemonSettings();
    SettingsSaverDaemonSettings& saveDaemonSettings(const legion::messages::DaemonSettings &daemonSettings);
};

// Other Settings
class SettingsLoaderOther: protected Settings
{
public:
    explicit SettingsLoaderOther();
    SettingsLoaderOther& loadOther(legion::messages::OtherSettings &otherSettings);
};

class SettingsSaverOther: protected Settings
{
public:
    explicit SettingsSaverOther();
    SettingsSaverOther& saveOther(const legion::messages::OtherSettings &otherSettings);
};

}
