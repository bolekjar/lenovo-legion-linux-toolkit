// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <Settings.h>

#include <Core/Application.h>
#include <Core/LoggerHolder.h>


#include <QDir>

namespace  LenovoLegionGui {

Settings::Settings(const QString &group):
    m_settings(std::filesystem::path(QCoreApplication::applicationDirPath().toStdString()).append(bj::framework::Application::data_dir).append(CONFIG_NAME.toStdString()).string().c_str(), QSettings::IniFormat)
{
    m_settings.setValue("LenovoLegion","LenovoLegionGui");
    m_settings.beginGroup(group);
}

Settings::~Settings()
{
    m_settings.endGroup();
}

const QString Settings::CONFIG_NAME = "LenovoLegionGui.ini";


ApplicationSettings::ApplicationSettings(QObject* parent)
    : QObject(parent)
    , Settings("Application")
{}

ApplicationSettings* ApplicationSettings::instance()
{
    static ApplicationSettings s_instance;
    return &s_instance;
}

ApplicationSettings& ApplicationSettings::loadStartMinimized(bool& value)
{
    value = m_settings.value("StartMinimized", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadMinimizeToTray(bool& value)
{
    value = m_settings.value("MinimizeToTray", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadAppDebugLogging(bool& value)
{
    value = m_settings.value("AppDebugLogging", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadAppTraceLogging(bool& value)
{
    value = m_settings.value("AppTraceLogging", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadSaveSettingsOnDaemonExit(bool& value)
{
    value = m_settings.value("SaveSettingsOnDaemonExit", true).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadDaemonDebugLogging(bool& value)
{
    value = m_settings.value("DaemonDebugLogging", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadStylesheetTheme(ThemeType& value)
{
    value = static_cast<ThemeType>(m_settings.value("StylesheetTheme", static_cast<int>(ThemeType::NoTheme)).toInt());
    return *this;
}

ApplicationSettings& ApplicationSettings::saveStartMinimized(bool value)
{
    m_settings.setValue("StartMinimized", value);
    emit settingChanged(SettingType::StartMinimized, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveMinimizeToTray(bool value)
{
    m_settings.setValue("MinimizeToTray", value);
    emit settingChanged(SettingType::MinimizeToTray, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveAppDebugLogging(bool value)
{
    m_settings.setValue("AppDebugLogging", value);
    emit settingChanged(SettingType::AppDebugLogging, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveAppTraceLogging(bool value)
{
    m_settings.setValue("AppTraceLogging", value);
    emit settingChanged(SettingType::AppTraceLogging, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveSaveSettingsOnDaemonExit(bool value)
{
    m_settings.setValue("SaveSettingsOnDaemonExit", value);
    emit settingChanged(SettingType::SaveSettingsOnDaemonExit, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveDaemonDebugLogging(bool value)
{
    m_settings.setValue("DaemonDebugLogging", value);
    emit settingChanged(SettingType::DaemonDebugLogging, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveStylesheetTheme(ThemeType value)
{
    m_settings.setValue("StylesheetTheme", static_cast<int>(value));
    emit themeChanged(value);
    return *this;
}

void ApplicationSettings::resetToDefaults()
{
    saveStartMinimized(false)
        .saveMinimizeToTray(false)
        .saveAppDebugLogging(false)
        .saveAppTraceLogging(false)
        .saveSaveSettingsOnDaemonExit(true)
        .saveDaemonDebugLogging(false)
        .saveStylesheetTheme(ThemeType::NoTheme);
}

// ProfileSettings implementation
ProfileSettings::ProfileSettings(const QString& profileName)
    : Settings(QString("Profile_").append(profileName))
{}

void ProfileSettings::saveDescription(const QString& description)
{
    m_settings.setValue("description", description);
}

QString ProfileSettings::loadDescription()
{
    return m_settings.value("description", "").toString();
}

void ProfileSettings::savePowerProfile(const legion::messages::PowerProfile& msg)
{
    if (msg.has_current_value()) {
        m_settings.setValue("PowerProfile/current_value", msg.current_value());
    }

    if(msg.has_custom_fnq_enabled()) {
        m_settings.setValue("PowerProfile/custom_fnq_enabled", msg.custom_fnq_enabled().current_value());
    }
}

void ProfileSettings::loadPowerProfile(legion::messages::PowerProfile& msg)
{
    if (m_settings.contains("PowerProfile/current_value")) {
        msg.set_current_value(static_cast<legion::messages::PowerProfile_Profiles>(
            m_settings.value("PowerProfile/current_value").toInt()));
    }

    if(m_settings.contains("PowerProfile/custom_fnq_enabled")) {
        msg.mutable_custom_fnq_enabled()->set_current_value(m_settings.value("PowerProfile/custom_fnq_enabled").toBool());
    }
}

void ProfileSettings::saveCPUOptions(const legion::messages::CPUOptions& msg)
{
    m_settings.setValue("CPUOptions/count", msg.cpus_size());
    for (int i = 0; i < msg.cpus_size(); ++i) {
        const auto& cpu = msg.cpus(i);
        QString prefix = QString("CPUOptions/cpu%1_").arg(i);

        if(cpu.has_governor())
        {
            m_settings.setValue(prefix + "governor", QString::fromStdString(cpu.governor().data()));
        }
        if (cpu.has_cpu_online()) {
            m_settings.setValue(prefix + "cpu_online", cpu.cpu_online());
        }
    }
}

void ProfileSettings::loadCPUOptions(legion::messages::CPUOptions& msg)
{
    int count = m_settings.value("CPUOptions/count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("CPUOptions/cpu%1_").arg(i);
        auto* cpu = msg.add_cpus();
        if(m_settings.contains(prefix + "governor"))
        {
            cpu->set_governor(m_settings.value(prefix + "governor").toString().toStdString());
        }
        if (m_settings.contains(prefix + "cpu_online")) {
            cpu->set_cpu_online(m_settings.value(prefix + "cpu_online").toBool());
        }
    }
}

void ProfileSettings::saveCPUFrequency(const legion::messages::CPUFrequency& msg)
{
    m_settings.setValue("CPUFrequency/count", msg.cpus_size());
    for (int i = 0; i < msg.cpus_size(); ++i) {
        const auto& cpu = msg.cpus(i);
        QString prefix = QString("CPUFrequency/cpu%1_").arg(i);
        if (cpu.has_scaling_min_freq()) {
            m_settings.setValue(prefix + "scaling_min_freq", cpu.scaling_min_freq());
        }
        if (cpu.has_scaling_max_freq()) {
            m_settings.setValue(prefix + "scaling_max_freq", cpu.scaling_max_freq());
        }
    }
}

void ProfileSettings::loadCPUFrequency(legion::messages::CPUFrequency& msg)
{
    int count = m_settings.value("CPUFrequency/count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("CPUFrequency/cpu%1_").arg(i);
        auto* cpu = msg.add_cpus();
        if (m_settings.contains(prefix + "scaling_min_freq")) {
            cpu->set_scaling_min_freq(m_settings.value(prefix + "scaling_min_freq").toUInt());
        }
        if (m_settings.contains(prefix + "scaling_max_freq")) {
            cpu->set_scaling_max_freq(m_settings.value(prefix + "scaling_max_freq").toUInt());
        }
    }
}

void ProfileSettings::saveFanCurve(const legion::messages::FanCurve& msg)
{
    if (msg.has_current_value()) {
        const auto& points = msg.current_value();
        if (points.has_point1()) m_settings.setValue("FanCurve/point1", points.point1());
        if (points.has_point2()) m_settings.setValue("FanCurve/point2", points.point2());
        if (points.has_point3()) m_settings.setValue("FanCurve/point3", points.point3());
        if (points.has_point4()) m_settings.setValue("FanCurve/point4", points.point4());
        if (points.has_point5()) m_settings.setValue("FanCurve/point5", points.point5());
        if (points.has_point6()) m_settings.setValue("FanCurve/point6", points.point6());
        if (points.has_point7()) m_settings.setValue("FanCurve/point7", points.point7());
        if (points.has_point8()) m_settings.setValue("FanCurve/point8", points.point8());
        if (points.has_point9()) m_settings.setValue("FanCurve/point9", points.point9());
        if (points.has_point10()) m_settings.setValue("FanCurve/point10", points.point10());
    }
    
    // Save default maps - note: these are typically not user-configurable,
    // but we can save them if needed for reference
    // For now, we only save the current_value (user-configured curve)
}

void ProfileSettings::loadFanCurve(legion::messages::FanCurve& msg)
{
    auto* points = msg.mutable_current_value();
    
    if (m_settings.contains("FanCurve/point1")) points->set_point1(m_settings.value("FanCurve/point1").toUInt());
    if (m_settings.contains("FanCurve/point2")) points->set_point2(m_settings.value("FanCurve/point2").toUInt());
    if (m_settings.contains("FanCurve/point3")) points->set_point3(m_settings.value("FanCurve/point3").toUInt());
    if (m_settings.contains("FanCurve/point4")) points->set_point4(m_settings.value("FanCurve/point4").toUInt());
    if (m_settings.contains("FanCurve/point5")) points->set_point5(m_settings.value("FanCurve/point5").toUInt());
    if (m_settings.contains("FanCurve/point6")) points->set_point6(m_settings.value("FanCurve/point6").toUInt());
    if (m_settings.contains("FanCurve/point7")) points->set_point7(m_settings.value("FanCurve/point7").toUInt());
    if (m_settings.contains("FanCurve/point8")) points->set_point8(m_settings.value("FanCurve/point8").toUInt());
    if (m_settings.contains("FanCurve/point9")) points->set_point9(m_settings.value("FanCurve/point9").toUInt());
    if (m_settings.contains("FanCurve/point10")) points->set_point10(m_settings.value("FanCurve/point10").toUInt());
}

void ProfileSettings::saveFanOption(const legion::messages::FanOption& msg)
{
    if (msg.has_full_speed()) {
        m_settings.setValue("FanOption/full_speed", msg.full_speed().current_value());
    }
}

void ProfileSettings::loadFanOption(legion::messages::FanOption& msg)
{
    if (m_settings.contains("FanOption/full_speed")) {
        msg.mutable_full_speed()->set_current_value(m_settings.value("FanOption/full_speed").toBool());
    }
}

void ProfileSettings::saveCPUSMT(const legion::messages::CPUSMT& msg)
{
    if (msg.has_control()) {
        m_settings.setValue("CPUSMT/control", QString(msg.control().data()).trimmed());
    }
}

void ProfileSettings::loadCPUSMT(legion::messages::CPUSMT& msg)
{
    if (m_settings.contains("CPUSMT/control")) {
        msg.set_control(m_settings.value("CPUSMT/control").toString().toStdString());
    }
}

void ProfileSettings::saveCPUPower(const legion::messages::CPUPower& msg)
{
    if (msg.has_cpu_tmp_limit()) {
        m_settings.setValue("CPUPower/cpu_tmp_limit", msg.cpu_tmp_limit().current_value());
    }
    if (msg.has_cpu_clp_limit()) {
        m_settings.setValue("CPUPower/cpu_clp_limit", msg.cpu_clp_limit().current_value());
    }
    if (msg.has_cpu_ltp_limit()) {
        m_settings.setValue("CPUPower/cpu_ltp_limit", msg.cpu_ltp_limit().current_value());
    }
    if (msg.has_cpu_stp_limit()) {
        m_settings.setValue("CPUPower/cpu_stp_limit", msg.cpu_stp_limit().current_value());
    }
    if (msg.has_cpu_pl1_tau()) {
        m_settings.setValue("CPUPower/cpu_pl1_tau", msg.cpu_pl1_tau().current_value());
    }
    if (msg.has_gpu_total_onac()) {
        m_settings.setValue("CPUPower/gpu_total_onac", msg.gpu_total_onac().current_value());
    }
    if (msg.has_gpu_to_cpu_dynamic_boost()) {
        m_settings.setValue("CPUPower/gpu_to_cpu_dynamic_boost", msg.gpu_to_cpu_dynamic_boost().current_value());
    }
}

void ProfileSettings::loadCPUPower(legion::messages::CPUPower& msg)
{
    if (m_settings.contains("CPUPower/cpu_tmp_limit")) {
        msg.mutable_cpu_tmp_limit()->set_current_value(m_settings.value("CPUPower/cpu_tmp_limit").toUInt());
    }
    if (m_settings.contains("CPUPower/cpu_clp_limit")) {
        msg.mutable_cpu_clp_limit()->set_current_value(m_settings.value("CPUPower/cpu_clp_limit").toUInt());
    }
    if (m_settings.contains("CPUPower/cpu_ltp_limit")) {
        msg.mutable_cpu_ltp_limit()->set_current_value(m_settings.value("CPUPower/cpu_ltp_limit").toUInt());
    }
    if (m_settings.contains("CPUPower/cpu_stp_limit")) {
        msg.mutable_cpu_stp_limit()->set_current_value(m_settings.value("CPUPower/cpu_stp_limit").toUInt());
    }
    if (m_settings.contains("CPUPower/cpu_pl1_tau")) {
        msg.mutable_cpu_pl1_tau()->set_current_value(m_settings.value("CPUPower/cpu_pl1_tau").toUInt());
    }
    if (m_settings.contains("CPUPower/gpu_total_onac")) {
        msg.mutable_gpu_total_onac()->set_current_value(m_settings.value("CPUPower/gpu_total_onac").toUInt());
    }
    if (m_settings.contains("CPUPower/gpu_to_cpu_dynamic_boost")) {
        msg.mutable_gpu_to_cpu_dynamic_boost()->set_current_value(m_settings.value("CPUPower/gpu_to_cpu_dynamic_boost").toUInt());
    }
}

void ProfileSettings::saveGPUPower(const legion::messages::GPUPower& msg)
{
    if (msg.has_gpu_power_boost()) {
        m_settings.setValue("GPUPower/gpu_power_boost", msg.gpu_power_boost().current_value());
    }
    if (msg.has_gpu_configurable_tgp()) {
        m_settings.setValue("GPUPower/gpu_configurable_tgp", msg.gpu_configurable_tgp().current_value());
    }
    if (msg.has_gpu_temperature_limit()) {
        m_settings.setValue("GPUPower/gpu_temperature_limit", msg.gpu_temperature_limit().current_value());
    }
}

void ProfileSettings::loadGPUPower(legion::messages::GPUPower& msg)
{
    if (m_settings.contains("GPUPower/gpu_power_boost")) {
        msg.mutable_gpu_power_boost()->set_current_value(m_settings.value("GPUPower/gpu_power_boost").toUInt());
    }
    if (m_settings.contains("GPUPower/gpu_configurable_tgp")) {
        msg.mutable_gpu_configurable_tgp()->set_current_value(m_settings.value("GPUPower/gpu_configurable_tgp").toUInt());
    }
    if (m_settings.contains("GPUPower/gpu_temperature_limit")) {
        msg.mutable_gpu_temperature_limit()->set_current_value(m_settings.value("GPUPower/gpu_temperature_limit").toUInt());
    }
}

void ProfileSettings::saveNvidiaNvml(const legion::messages::NvidiaNvml& msg)
{
    if (msg.has_gpu_offset()) {
        m_settings.setValue("NvidiaNvml/gpu_offset", msg.gpu_offset().value());
    }
    if (msg.has_memory_offset()) {
        m_settings.setValue("NvidiaNvml/memory_offset", msg.memory_offset().value());
    }
}

void ProfileSettings::loadNvidiaNvml(legion::messages::NvidiaNvml& msg)
{
    if (m_settings.contains("NvidiaNvml/gpu_offset")) {
        msg.mutable_gpu_offset()->set_value(m_settings.value("NvidiaNvml/gpu_offset").toInt());
    }
    if (m_settings.contains("NvidiaNvml/memory_offset")) {
        msg.mutable_memory_offset()->set_value(m_settings.value("NvidiaNvml/memory_offset").toInt());
    }
}

void ProfileSettings::saveIntelMSR(const legion::messages::CpuIntelMSR& msg)
{
    if (msg.has_analogio() && msg.analogio().has_offset()) {
        m_settings.setValue("IntelMSR/analogio_offset", msg.analogio().offset());
    }
    if (msg.has_cache() && msg.cache().has_offset()) {
        m_settings.setValue("IntelMSR/cache_offset", msg.cache().offset());
    }
    if (msg.has_cpu() && msg.cpu().has_offset()) {
        m_settings.setValue("IntelMSR/cpu_offset", msg.cpu().offset());
    }
    if (msg.has_gpu() && msg.gpu().has_offset()) {
        m_settings.setValue("IntelMSR/gpu_offset", msg.gpu().offset());
    }
    if (msg.has_uncore() && msg.uncore().has_offset()) {
        m_settings.setValue("IntelMSR/uncore_offset", msg.uncore().offset());
    }
}

void ProfileSettings::loadIntelMSR(legion::messages::CpuIntelMSR& msg)
{
    if (m_settings.contains("IntelMSR/analogio_offset")) {
        msg.mutable_analogio()->set_offset(m_settings.value("IntelMSR/analogio_offset").toInt());
    }
    if (m_settings.contains("IntelMSR/cache_offset")) {
        msg.mutable_cache()->set_offset(m_settings.value("IntelMSR/cache_offset").toInt());
    }
    if (m_settings.contains("IntelMSR/cpu_offset")) {
        msg.mutable_cpu()->set_offset(m_settings.value("IntelMSR/cpu_offset").toInt());
    }
    if (m_settings.contains("IntelMSR/gpu_offset")) {
        msg.mutable_gpu()->set_offset(m_settings.value("IntelMSR/gpu_offset").toInt());
    }
    if (m_settings.contains("IntelMSR/uncore_offset")) {
        msg.mutable_uncore()->set_offset(m_settings.value("IntelMSR/uncore_offset").toInt());
    }
}

void ProfileSettings::saveOther(const legion::messages::OtherSettings& msg)
{
    if (msg.has_touch_pad() && msg.touch_pad().has_current()) {
        m_settings.setValue("Other/disable_touchpad", msg.touch_pad().current());
    }
    if (msg.has_win_key() && msg.win_key().has_current()) {
        m_settings.setValue("Other/disable_win_key", msg.win_key().current());
    }
}

void ProfileSettings::loadOther(legion::messages::OtherSettings& msg)
{
    if (m_settings.contains("Other/disable_touchpad")) {
        msg.mutable_touch_pad()->set_current(m_settings.value("Other/disable_touchpad").toBool());
    }
    if (m_settings.contains("Other/disable_win_key")) {
        msg.mutable_win_key()->set_current(m_settings.value("Other/disable_win_key").toBool());
    }
}

// ProfileManager implementation
QStringList ProfileManager::listProfiles()
{
    QSettings settings(std::filesystem::path(QCoreApplication::applicationDirPath().toStdString()).append(bj::framework::Application::data_dir).append(Settings::CONFIG_NAME.toStdString()).string().c_str(), QSettings::IniFormat);
    QStringList groups = settings.childGroups();
    QStringList profiles;
    
    for (const QString& group : groups) {
        if (group.startsWith("Profile_")) {
            profiles.append(group.mid(8)); // Remove "Profile_" prefix
        }
    }
    
    return profiles;
}

bool ProfileManager::profileExists(const QString& profileName)
{
    return listProfiles().contains(profileName);
}

bool ProfileManager::deleteProfile(const QString& profileName)
{
    QSettings settings(std::filesystem::path(QCoreApplication::applicationDirPath().toStdString()).append(bj::framework::Application::data_dir).append(Settings::CONFIG_NAME.toStdString()).string().c_str(), QSettings::IniFormat);
    QString groupName = QString("Profile_").append(profileName);
    
    settings.beginGroup(groupName);
    settings.remove(""); // Remove all keys in this group
    settings.endGroup();
    
    return true;
}

QString ProfileManager::getProfileDescription(const QString& profileName)
{
    ProfileSettings profile(profileName);
    return profile.loadDescription();
}

}
