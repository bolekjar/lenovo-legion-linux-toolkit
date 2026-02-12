// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderFanOption.h"


#include "SysFsDriverLegionOther.h"

#include "../LenovoLegion-PrepareBuild/FanControl.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderFanOption::SysFsDataProviderFanOption(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}


QByteArray SysFsDataProviderFanOption::serializeAndGetData() const
{
    legion::messages::FanOption         fanOptionMsg;
    QByteArray                          byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFsDriverLegionOther::Other other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

        fanOptionMsg.mutable_full_speed()->set_supported(getData(other.m_fan_full_speed.m_supported).toUShort() > 0);
        fanOptionMsg.mutable_full_speed()->set_default_value(getData(other.m_fan_full_speed.m_default_value).toUShort() == 1);
        fanOptionMsg.mutable_full_speed()->set_current_value(getData(other.m_fan_full_speed.m_current_value).toUShort() == 1);
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            fanOptionMsg.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(fanOptionMsg.ByteSizeLong());
    if(!fanOptionMsg.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderFanOption::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverLegionOther::Other       other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));
    legion::messages::FanOption         fanOptionMsg;

    LOG_T(__PRETTY_FUNCTION__);

    if(!fanOptionMsg.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    if(fanOptionMsg.has_full_speed()) {

        if(getData(other.m_fan_full_speed.m_supported).toUShort() > 0)
        {
            setData(other.m_fan_full_speed.m_current_value,fanOptionMsg.full_speed().current_value());
        }
    }

    return {};
}


}
