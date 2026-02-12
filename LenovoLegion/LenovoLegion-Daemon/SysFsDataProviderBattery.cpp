// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderBattery.h"
#include "SysFsDriverPowerSuplyBattery0.h"
#include "SysFSDriverLegionGameZone.h"


#include "../LenovoLegion-PrepareBuild/Battery.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderBattery::SysFsDataProviderBattery(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}



QByteArray SysFsDataProviderBattery::serializeAndGetData() const
{
    legion::messages::Battery          battery;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverPowerSuplyBattery0::PowerSuplyBattery0 batery0(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverPowerSuplyBattery0::DRIVER_NAME));
        SysFSDriverLegionGameZone::GameZone::Other        other(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));

        battery.set_current_charge_mode_value(static_cast<legion::messages::Battery::PowerChargeMode>(getData(other.get_power_charge_mode).toUShort()));
        battery.set_baterry_status(getData(batery0.m_powerSuplyBattery0).toStdString());
        battery.set_supported(true);

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            battery.Clear();
        }
        else
        {
            throw;
        }
    }


    byteArray.resize(battery.ByteSizeLong());
    if(!battery.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderBattery::deserializeAndSetData(const QByteArray &)
{
    return {};
}


}
