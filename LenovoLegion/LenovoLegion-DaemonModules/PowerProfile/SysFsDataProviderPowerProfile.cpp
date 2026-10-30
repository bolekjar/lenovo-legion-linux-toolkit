// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderPowerProfile.h"
#include <Drivers/SysFsDriverLegionGameZone.h>
#include <Drivers/SysFsDriverLegionOther.h>

#include <PowerProfile.pb.h>


#include <KitLog.h>

#include <algorithm>


namespace LenovoLegionDaemon::Kit {

SysFsDataProviderPowerProfile::SysFsDataProviderPowerProfile(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}



QByteArray SysFsDataProviderPowerProfile::serializeAndGetData()
{
    legion::messages::PowerProfile     powerProfile;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverLegionGameZone::GameZone::SmartFan smartFan(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));
        SysFsDriverLegionGameZone::GameZone::Other    other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));
        SysFsDriverLegionOther::Other                 otherInOther(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

        powerProfile.set_current_value(static_cast<legion::messages::PowerProfile::Profiles>(getData(smartFan.m_current_value).toUShort()));
        powerProfile.set_thermal_mode(static_cast<legion::messages::PowerProfile::Profiles>(getData(other.get_thermal_mode).toUShort()));

        powerProfile.mutable_custom_fnq_enabled()->set_supported(getData(otherInOther.m_god_mode_fnq_switchable.m_supported).toShort() > 0);
        powerProfile.mutable_custom_fnq_enabled()->set_current_value(getData(otherInOther.m_god_mode_fnq_switchable.m_current_value).toShort() == 1);
        powerProfile.mutable_custom_fnq_enabled()->set_default_value(getData(otherInOther.m_god_mode_fnq_switchable.m_default_value).toShort() == 1);

        for(const auto profile : supportedProfiles())
        {
            powerProfile.add_supported_profiles(profile);
        }

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            powerProfile.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(powerProfile.ByteSizeLong());
    if(!powerProfile.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderPowerProfile::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverLegionGameZone::GameZone::SmartFan smartFan(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));
    SysFsDriverLegionGameZone::GameZone::Other    other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));
    SysFsDriverLegionOther::Other                 otherInOther(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

    legion::messages::PowerProfile                powerProfile;

    LOG_T(__PRETTY_FUNCTION__);

    if(!powerProfile.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    settable(powerProfile,Origin::REQUEST);

    if(powerProfile.has_current_value()) {
        setData(smartFan.m_current_value,static_cast<quint8>(powerProfile.current_value()));
    }

    if(powerProfile.has_custom_fnq_enabled())
    {
        setData(otherInOther.m_god_mode_fnq_switchable.m_current_value, powerProfile.custom_fnq_enabled().current_value());
    }

    return {};
}

QByteArray SysFsDataProviderPowerProfile::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::PowerProfile>(data,[this,origin](legion::messages::PowerProfile& profile) { settable(profile,origin); });
}

std::vector<legion::messages::PowerProfile::Profiles> SysFsDataProviderPowerProfile::supportedProfiles() const
{
    SysFsDriverLegionGameZone::GameZone::SmartFan smartFan(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));

    std::vector<legion::messages::PowerProfile::Profiles> profiles {
        legion::messages::PowerProfile::POWER_PROFILE_QUIET,
        legion::messages::PowerProfile::POWER_PROFILE_BALANCED,
        legion::messages::PowerProfile::POWER_PROFILE_PERFORMANCE,
        legion::messages::PowerProfile::POWER_PROFILE_CUSTOM
    };

    if(getData(smartFan.m_extreme_supported).toUShort() == 1)
    {
        profiles.push_back(legion::messages::PowerProfile::POWER_PROFILE_EXTREME);
    }

    return profiles;
}

void SysFsDataProviderPowerProfile::settable(legion::messages::PowerProfile &profile, Origin origin) const
{
    profile.clear_thermal_mode();
    profile.clear_supported_profiles();

    if(profile.has_current_value())
    {
        const auto profiles = supportedProfiles();

        if(std::find(profiles.begin(),profiles.end(),profile.current_value()) == profiles.end())
        {
            skipField(origin,QString("power profile=").append(QString::number(profile.current_value())),"not supported");
            profile.clear_current_value();
        }
    }

    if(profile.has_custom_fnq_enabled())
    {
        profile.mutable_custom_fnq_enabled()->clear_default_value();
        profile.mutable_custom_fnq_enabled()->clear_supported();

        SysFsDriverLegionOther::Other other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

        if(!profile.custom_fnq_enabled().has_current_value())
        {
            profile.clear_custom_fnq_enabled();
        }
        else if(getData(other.m_god_mode_fnq_switchable.m_supported).toShort() <= 0)
        {
            skipField(origin,QString("custom Fn+Q=").append(QString::number(profile.custom_fnq_enabled().current_value())),"not supported");
            profile.clear_custom_fnq_enabled();
        }
    }
}


}
