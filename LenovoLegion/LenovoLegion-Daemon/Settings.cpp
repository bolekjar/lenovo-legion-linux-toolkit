// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <Settings.h>
#include <Core/Application.h>

namespace LenovoLegionDaemon {

Settings::Settings(const QString &group) :
    m_settings(QCoreApplication::applicationDirPath()
               .append(QDir::separator())
               .append(bj::framework::Application::data_dir)
               .append(QDir::separator())
               .append("/LenovoLegion-Daemon.ini"), QSettings::IniFormat)
{
    m_settings.setValue("LenovoLegion", "LenovoLegion-Daemon");
    m_settings.beginGroup(group);
}

Settings::~Settings()
{
    m_settings.endGroup();
}

SettingsLoaderPowerProfiles::SettingsLoaderPowerProfiles() :
    Settings("PowerProfileData")
{
}

SettingsLoaderPowerProfiles& SettingsLoaderPowerProfiles::loadPowerProfile(legion::messages::PowerProfile &profile)
{
    if (m_settings.contains("PowerProfile")) {
        profile.set_current_value(static_cast<legion::messages::PowerProfile::Profiles>(
            m_settings.value("PowerProfile").toUInt()));
    }

    if(m_settings.contains("custom_fnq_enabled")) {
        profile.mutable_custom_fnq_enabled()->set_current_value(m_settings.value("custom_fnq_enabled").toBool());
    }

    return *this;
}

SettingsSaverPowerProfiles::SettingsSaverPowerProfiles() :
    Settings("PowerProfileData")
{
}

SettingsSaverPowerProfiles& SettingsSaverPowerProfiles::saverPowerProfile(const legion::messages::PowerProfile &profile)
{
    m_settings.setValue("PowerProfile", static_cast<quint32>(profile.current_value()));
    m_settings.setValue("custom_fnq_enabled", profile.custom_fnq_enabled().current_value());
    return *this;
}

SettingsLoaderCPUControlData::SettingsLoaderCPUControlData() :
    Settings("CPUsControlData")
{
}

SettingsLoaderCPUControlData& SettingsLoaderCPUControlData::loadPowerProfile(legion::messages::CPUOptions &profile)
{
    size_t size = m_settings.beginReadArray("CPUControl");
    for (size_t i = 0; i < size; ++i) {
        legion::messages::CPUOptions::CPUX* cpux = profile.add_cpus();
        m_settings.setArrayIndex(i);
        if (m_settings.contains("online")) {
            cpux->set_cpu_online(m_settings.value("online").toBool());
        }
        if (m_settings.contains("governor")) {
            cpux->set_governor(m_settings.value("governor").toString().toStdString());
        }
    }
    m_settings.endArray();
    return *this;
}

SettingsSaverCPUControlData::SettingsSaverCPUControlData() :
    Settings("CPUsControlData")
{
}

SettingsSaverCPUControlData& SettingsSaverCPUControlData::saverPowerProfile(const legion::messages::CPUOptions &profile)
{
    m_settings.beginWriteArray("CPUControl");
    for (int i = 0; i < profile.cpus().size(); ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("online", profile.cpus().at(i).cpu_online());
        m_settings.setValue("governor", QString(profile.cpus().at(i).governor().data()).trimmed());
    }
    m_settings.endArray();
    return *this;
}

// CPU Frequency Settings
SettingsLoaderCPUFrequency::SettingsLoaderCPUFrequency() :
    Settings("CPUFrequencyData")
{
}

SettingsLoaderCPUFrequency& SettingsLoaderCPUFrequency::loadCPUFrequency(legion::messages::CPUFrequency &frequency)
{
    size_t size = m_settings.beginReadArray("CPUFrequency");
    for (size_t i = 0; i < size; ++i) {
        legion::messages::CPUFrequency::CPUX* cpux = frequency.add_cpus();
        m_settings.setArrayIndex(i);
        if (m_settings.contains("scaling_min_freq")) {
            cpux->set_scaling_min_freq(m_settings.value("scaling_min_freq").toUInt());
        }
        if (m_settings.contains("scaling_max_freq")) {
            cpux->set_scaling_max_freq(m_settings.value("scaling_max_freq").toUInt());
        }
    }
    m_settings.endArray();
    return *this;
}

SettingsSaverCPUFrequency::SettingsSaverCPUFrequency() :
    Settings("CPUFrequencyData")
{
}

SettingsSaverCPUFrequency& SettingsSaverCPUFrequency::saveCPUFrequency(const legion::messages::CPUFrequency &frequency)
{
    m_settings.beginWriteArray("CPUFrequency");
    for (int i = 0; i < frequency.cpus().size(); ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("scaling_min_freq", frequency.cpus().at(i).scaling_min_freq());
        m_settings.setValue("scaling_max_freq", frequency.cpus().at(i).scaling_max_freq());
    }
    m_settings.endArray();
    return *this;
}

// Fan Curve Settings
SettingsLoaderFanCurve::SettingsLoaderFanCurve() :
    Settings("FanCurveData")
{
}

SettingsLoaderFanCurve& SettingsLoaderFanCurve::loadFanCurve(legion::messages::FanCurve &fanCurve)
{
    if (m_settings.contains("point1")) {
        fanCurve.mutable_current_value()->set_point1(m_settings.value("point1").toInt());
    }
    if (m_settings.contains("point2")) {
        fanCurve.mutable_current_value()->set_point2(m_settings.value("point2").toInt());
    }
    if (m_settings.contains("point3")) {
        fanCurve.mutable_current_value()->set_point3(m_settings.value("point3").toInt());
    }
    if (m_settings.contains("point4")) {
        fanCurve.mutable_current_value()->set_point4(m_settings.value("point4").toInt());
    }
    if (m_settings.contains("point5")) {
        fanCurve.mutable_current_value()->set_point5(m_settings.value("point5").toInt());
    }
    if (m_settings.contains("point6")) {
        fanCurve.mutable_current_value()->set_point6(m_settings.value("point6").toInt());
    }
    if (m_settings.contains("point7")) {
        fanCurve.mutable_current_value()->set_point7(m_settings.value("point7").toInt());
    }
    if (m_settings.contains("point8")) {
        fanCurve.mutable_current_value()->set_point8(m_settings.value("point8").toInt());
    }
    if (m_settings.contains("point9")) {
        fanCurve.mutable_current_value()->set_point9(m_settings.value("point9").toInt());
    }
    if (m_settings.contains("point10")) {
        fanCurve.mutable_current_value()->set_point10(m_settings.value("point10").toInt());
    }
    return *this;
}

SettingsSaverFanCurve::SettingsSaverFanCurve() :
    Settings("FanCurveData")
{
}

SettingsSaverFanCurve& SettingsSaverFanCurve::saveFanCurve(const legion::messages::FanCurve &fanCurve)
{
    m_settings.setValue("point1", fanCurve.current_value().point1());
    m_settings.setValue("point2", fanCurve.current_value().point2());
    m_settings.setValue("point3", fanCurve.current_value().point3());
    m_settings.setValue("point4", fanCurve.current_value().point4());
    m_settings.setValue("point5", fanCurve.current_value().point5());
    m_settings.setValue("point6", fanCurve.current_value().point6());
    m_settings.setValue("point7", fanCurve.current_value().point7());
    m_settings.setValue("point8", fanCurve.current_value().point8());
    m_settings.setValue("point9", fanCurve.current_value().point9());
    m_settings.setValue("point10", fanCurve.current_value().point10());
    return *this;
}

// Fan Option Settings
SettingsLoaderFanOption::SettingsLoaderFanOption() :
    Settings("FanOptionData")
{
}

SettingsLoaderFanOption& SettingsLoaderFanOption::loadFanOption(legion::messages::FanOption &fanOption)
{
    if (m_settings.contains("full_speed")) {
        fanOption.mutable_full_speed()->set_current_value(m_settings.value("full_speed").toBool());
    }
    return *this;
}

SettingsSaverFanOption::SettingsSaverFanOption() :
    Settings("FanOptionData")
{
}

SettingsSaverFanOption& SettingsSaverFanOption::saveFanOption(const legion::messages::FanOption &fanOption)
{
    m_settings.setValue("full_speed", fanOption.full_speed().current_value());
    return *this;
}

// CPU SMT Settings
SettingsLoaderCPUSMT::SettingsLoaderCPUSMT() :
    Settings("CPUSMTData")
{
}

SettingsLoaderCPUSMT& SettingsLoaderCPUSMT::loadCPUSMT(legion::messages::CPUSMT &cpuSmt)
{
    if (m_settings.contains("control")) {
        cpuSmt.set_control(m_settings.value("control").toString().toStdString());
    }
    return *this;
}

SettingsSaverCPUSMT::SettingsSaverCPUSMT() :
    Settings("CPUSMTData")
{
}

SettingsSaverCPUSMT& SettingsSaverCPUSMT::saveCPUSMT(const legion::messages::CPUSMT &cpuSmt)
{
    m_settings.setValue("control", QString::fromStdString(cpuSmt.control().data()));
    return *this;
}

// CPU Power Settings
SettingsLoaderCPUPower::SettingsLoaderCPUPower() :
    Settings("CPUPowerData")
{
}

SettingsLoaderCPUPower& SettingsLoaderCPUPower::loadCPUPower(legion::messages::CPUPower &cpuPower)
{
    if (m_settings.contains("cpu_stp_limit")) {
        cpuPower.mutable_cpu_stp_limit()->set_current_value(m_settings.value("cpu_stp_limit").toUInt());
    }
    if (m_settings.contains("cpu_ltp_limit")) {
        cpuPower.mutable_cpu_ltp_limit()->set_current_value(m_settings.value("cpu_ltp_limit").toUInt());
    }
    if (m_settings.contains("cpu_clp_limit")) {
        cpuPower.mutable_cpu_clp_limit()->set_current_value(m_settings.value("cpu_clp_limit").toUInt());
    }
    if (m_settings.contains("cpu_tmp_limit")) {
        cpuPower.mutable_cpu_tmp_limit()->set_current_value(m_settings.value("cpu_tmp_limit").toUInt());
    }
    if (m_settings.contains("cpu_pl1_tau")) {
        cpuPower.mutable_cpu_pl1_tau()->set_current_value(m_settings.value("cpu_pl1_tau").toUInt());
    }
    if (m_settings.contains("gpu_total_onac")) {
        cpuPower.mutable_gpu_total_onac()->set_current_value(m_settings.value("gpu_total_onac").toUInt());
    }
    if (m_settings.contains("gpu_to_cpu_dynamic_boost")) {
        cpuPower.mutable_gpu_to_cpu_dynamic_boost()->set_current_value(m_settings.value("gpu_to_cpu_dynamic_boost").toUInt());
    }
    return *this;
}

SettingsSaverCPUPower::SettingsSaverCPUPower() :
    Settings("CPUPowerData")
{
}

SettingsSaverCPUPower& SettingsSaverCPUPower::saveCPUPower(const legion::messages::CPUPower &cpuPower)
{
    if (cpuPower.has_cpu_stp_limit()) {
        m_settings.setValue("cpu_stp_limit", cpuPower.cpu_stp_limit().current_value());
    }
    if (cpuPower.has_cpu_ltp_limit()) {
        m_settings.setValue("cpu_ltp_limit", cpuPower.cpu_ltp_limit().current_value());
    }
    if (cpuPower.has_cpu_clp_limit()) {
        m_settings.setValue("cpu_clp_limit", cpuPower.cpu_clp_limit().current_value());
    }
    if (cpuPower.has_cpu_tmp_limit()) {
        m_settings.setValue("cpu_tmp_limit", cpuPower.cpu_tmp_limit().current_value());
    }
    if (cpuPower.has_cpu_pl1_tau()) {
        m_settings.setValue("cpu_pl1_tau", cpuPower.cpu_pl1_tau().current_value());
    }
    if (cpuPower.has_gpu_total_onac()) {
        m_settings.setValue("gpu_total_onac", cpuPower.gpu_total_onac().current_value());
    }
    if (cpuPower.has_gpu_to_cpu_dynamic_boost()) {
        m_settings.setValue("gpu_to_cpu_dynamic_boost", cpuPower.gpu_to_cpu_dynamic_boost().current_value());
    }
    return *this;
}

// GPU Power Settings
SettingsLoaderGPUPower::SettingsLoaderGPUPower() :
    Settings("GPUPowerData")
{
}

SettingsLoaderGPUPower& SettingsLoaderGPUPower::loadGPUPower(legion::messages::GPUPower &gpuPower)
{
    if (m_settings.contains("gpu_power_boost")) {
        gpuPower.mutable_gpu_power_boost()->set_current_value(m_settings.value("gpu_power_boost").toUInt());
    }
    if (m_settings.contains("gpu_configurable_tgp")) {
        gpuPower.mutable_gpu_configurable_tgp()->set_current_value(m_settings.value("gpu_configurable_tgp").toUInt());
    }
    if (m_settings.contains("gpu_temperature_limit")) {
        gpuPower.mutable_gpu_temperature_limit()->set_current_value(m_settings.value("gpu_temperature_limit").toUInt());
    }
    return *this;
}

SettingsSaverGPUPower::SettingsSaverGPUPower() :
    Settings("GPUPowerData")
{
}

SettingsSaverGPUPower& SettingsSaverGPUPower::saveGPUPower(const legion::messages::GPUPower &gpuPower)
{
    if (gpuPower.has_gpu_power_boost()) {
        m_settings.setValue("gpu_power_boost", gpuPower.gpu_power_boost().current_value());
    }
    if (gpuPower.has_gpu_configurable_tgp()) {
        m_settings.setValue("gpu_configurable_tgp", gpuPower.gpu_configurable_tgp().current_value());
    }
    if (gpuPower.has_gpu_temperature_limit()) {
        m_settings.setValue("gpu_temperature_limit", gpuPower.gpu_temperature_limit().current_value());
    }
    return *this;
}
// Nvidia NVML Settings
SettingsLoaderNvidiaNvml::SettingsLoaderNvidiaNvml() :
    Settings("NvidiaNvmlData")
{
}

SettingsLoaderNvidiaNvml& SettingsLoaderNvidiaNvml::loadNvidiaNvml(legion::messages::NvidiaNvml &nvidiaNvml)
{
    if (m_settings.contains("gpu_offset")) {
        nvidiaNvml.mutable_gpu_offset()->set_value(m_settings.value("gpu_offset").toInt());
    }
    if (m_settings.contains("memory_offset")) {
        nvidiaNvml.mutable_memory_offset()->set_value(m_settings.value("memory_offset").toInt());
    }
    return *this;
}

SettingsSaverNvidiaNvml::SettingsSaverNvidiaNvml() :
    Settings("NvidiaNvmlData")
{
}

SettingsSaverNvidiaNvml& SettingsSaverNvidiaNvml::saveNvidiaNvml(const legion::messages::NvidiaNvml &nvidiaNvml)
{
    if (nvidiaNvml.has_gpu_offset()) {
        m_settings.setValue("gpu_offset", nvidiaNvml.gpu_offset().value());
    }
    if (nvidiaNvml.has_memory_offset()) {
        m_settings.setValue("memory_offset", nvidiaNvml.memory_offset().value());
    }
    return *this;
}

// Daemon Settings
SettingsLoaderDaemonSettings::SettingsLoaderDaemonSettings() :
    Settings("DaemonSettings")
{
}

SettingsLoaderDaemonSettings& SettingsLoaderDaemonSettings::loadDaemonSettings(legion::messages::DaemonSettings &daemonSettings)
{
    daemonSettings.set_apply_settings_on_start(m_settings.value("apply_settings_on_start", true).toBool());
    daemonSettings.set_save_settings_on_exit(m_settings.value("save_settings_on_exit", true).toBool());
    daemonSettings.set_debug_logging(m_settings.value("debug_logging", false).toBool());
    daemonSettings.set_trace_logging(m_settings.value("trace_logging", false).toBool());
    // Note: save_now is not loaded - it's a command flag, not a persistent setting
    return *this;
}

SettingsSaverDaemonSettings::SettingsSaverDaemonSettings() :
    Settings("DaemonSettings")
{
}

SettingsSaverDaemonSettings& SettingsSaverDaemonSettings::saveDaemonSettings(const legion::messages::DaemonSettings &daemonSettings)
{
    m_settings.setValue("apply_settings_on_start", daemonSettings.apply_settings_on_start());
    m_settings.setValue("save_settings_on_exit", daemonSettings.save_settings_on_exit());
    m_settings.setValue("debug_logging", daemonSettings.debug_logging());
    m_settings.setValue("trace_logging", daemonSettings.trace_logging());
    // Note: save_now is not saved - it's a command flag, not a persistent setting
    return *this;
}

// Intel MSR Settings
SettingsLoaderIntelMSR::SettingsLoaderIntelMSR() :
    Settings("IntelMSRData")
{
}

SettingsLoaderIntelMSR& SettingsLoaderIntelMSR::loadIntelMSR(legion::messages::CpuIntelMSR &intelMSR)
{
    // Analogio voltage settings
    if (m_settings.contains("analogio_offset")) {
        intelMSR.mutable_analogio()->set_offset(m_settings.value("analogio_offset").toInt());
    }
    
    // Cache voltage settings
    if (m_settings.contains("cache_offset")) {
        intelMSR.mutable_cache()->set_offset(m_settings.value("cache_offset").toInt());
    }
    
    // CPU voltage settings
    if (m_settings.contains("cpu_offset")) {
        intelMSR.mutable_cpu()->set_offset(m_settings.value("cpu_offset").toInt());
    }
    
    // GPU voltage settings
    if (m_settings.contains("gpu_offset")) {
        intelMSR.mutable_gpu()->set_offset(m_settings.value("gpu_offset").toInt());
    }
    
    // Uncore voltage settings
    if (m_settings.contains("uncore_offset")) {
        intelMSR.mutable_uncore()->set_offset(m_settings.value("uncore_offset").toInt());
    }
    
    return *this;
}

SettingsSaverIntelMSR::SettingsSaverIntelMSR() :
    Settings("IntelMSRData")
{
}

SettingsSaverIntelMSR& SettingsSaverIntelMSR::saveIntelMSR(const legion::messages::CpuIntelMSR &intelMSR)
{
    // Save analogio offset if present
    if (intelMSR.has_analogio() && intelMSR.analogio().has_offset()) {
        m_settings.setValue("analogio_offset", intelMSR.analogio().offset());
    }
    
    // Save cache offset if present
    if (intelMSR.has_cache() && intelMSR.cache().has_offset()) {
        m_settings.setValue("cache_offset", intelMSR.cache().offset());
    }
    
    // Save CPU offset if present
    if (intelMSR.has_cpu() && intelMSR.cpu().has_offset()) {
        m_settings.setValue("cpu_offset", intelMSR.cpu().offset());
    }
    
    // Save GPU offset if present
    if (intelMSR.has_gpu() && intelMSR.gpu().has_offset()) {
        m_settings.setValue("gpu_offset", intelMSR.gpu().offset());
    }
    
    // Save uncore offset if present
    if (intelMSR.has_uncore() && intelMSR.uncore().has_offset()) {
        m_settings.setValue("uncore_offset", intelMSR.uncore().offset());
    }
    
    return *this;
}

// Other Settings
SettingsLoaderOther::SettingsLoaderOther() :
    Settings("OtherData")
{
}

SettingsLoaderOther& SettingsLoaderOther::loadOther(legion::messages::OtherSettings &otherSettings)
{
    // Load touchpad setting
    if (m_settings.contains("disable_touchpad")) {
        otherSettings.mutable_touch_pad()->set_current(m_settings.value("disable_touchpad").toBool());
    }
    
    // Load win key setting
    if (m_settings.contains("disable_win_key")) {
        otherSettings.mutable_win_key()->set_current(m_settings.value("disable_win_key").toBool());
    }
    
    return *this;
}

SettingsSaverOther::SettingsSaverOther() :
    Settings("OtherData")
{
}

SettingsSaverOther& SettingsSaverOther::saveOther(const legion::messages::OtherSettings &otherSettings)
{
    // Save touchpad setting if present
    if (otherSettings.has_touch_pad() && otherSettings.touch_pad().has_current()) {
        m_settings.setValue("disable_touchpad", otherSettings.touch_pad().current());
    }
    
    // Save win key setting if present
    if (otherSettings.has_win_key() && otherSettings.win_key().has_current()) {
        m_settings.setValue("disable_win_key", otherSettings.win_key().current());
    }
    
    return *this;
}

}
