// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderPowerProfile.h"
#include "SysFSDriverLegionGameZone.h"
#include "SysFsDriverLegionOther.h"

#include "../LenovoLegion-PrepareBuild/PowerProfile.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderPowerProfile::SysFsDataProviderPowerProfile(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}



QByteArray SysFsDataProviderPowerProfile::serializeAndGetData() const
{
    legion::messages::PowerProfile     powerProfile;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFSDriverLegionGameZone::GameZone::SmartFan smartFan(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));
        SysFSDriverLegionGameZone::GameZone::Other    other(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));
        SysFsDriverLegionOther::Other                 otherInOther(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

        powerProfile.set_current_value(static_cast<legion::messages::PowerProfile::Profiles>(getData(smartFan.m_current_value).toUShort()));
        powerProfile.set_thermal_mode(static_cast<legion::messages::PowerProfile::Profiles>(getData(other.get_thermal_mode).toUShort()));

        powerProfile.mutable_custom_fnq_enabled()->set_supported(getData(otherInOther.m_god_mode_fnq_switchable.m_supported).toShort() > 0);
        powerProfile.mutable_custom_fnq_enabled()->set_current_value(getData(otherInOther.m_god_mode_fnq_switchable.m_current_value).toShort() == 1);
        powerProfile.mutable_custom_fnq_enabled()->set_default_value(getData(otherInOther.m_god_mode_fnq_switchable.m_default_value).toShort() == 1);

        powerProfile.add_supported_profiles(legion::messages::PowerProfile::POWER_PROFILE_QUIET);
        powerProfile.add_supported_profiles(legion::messages::PowerProfile::POWER_PROFILE_BALANCED);
        powerProfile.add_supported_profiles(legion::messages::PowerProfile::POWER_PROFILE_PERFORMANCE);
        powerProfile.add_supported_profiles(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM);

        if(static_cast<legion::messages::PowerProfile::Profiles>(getData(smartFan.m_extreme_supported).toUShort() == 1))
        {
            powerProfile.add_supported_profiles(legion::messages::PowerProfile::POWER_PROFILE_EXTREME);
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
    SysFSDriverLegionGameZone::GameZone::SmartFan smartFan(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));
    SysFSDriverLegionGameZone::GameZone::Other    other(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));
    SysFsDriverLegionOther::Other                 otherInOther(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

    legion::messages::PowerProfile                powerProfile;

    LOG_T(__PRETTY_FUNCTION__);

    if(!powerProfile.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    if(powerProfile.has_current_value()) {
        setData(smartFan.m_current_value,static_cast<quint8>(powerProfile.current_value()));
    }

    if(powerProfile.has_custom_fnq_enabled())
    {
        if(getData(otherInOther.m_god_mode_fnq_switchable.m_supported).toShort() > 0)
        {
            setData(otherInOther.m_god_mode_fnq_switchable.m_current_value, powerProfile.custom_fnq_enabled().current_value());
        }
    }

    return {};
}


}
