// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderFanOption.h"


#include <Drivers/SysFsDriverLegionOther.h>

#include <FanControl.pb.h>


#include <KitLog.h>


namespace LenovoLegionDaemon::Kit {

SysFsDataProviderFanOption::SysFsDataProviderFanOption(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}


QByteArray SysFsDataProviderFanOption::serializeAndGetData()
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

    settable(fanOptionMsg,Origin::REQUEST);

    m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionOther::DRIVER_NAME,true);
    auto cleanup =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionOther::DRIVER_NAME,false); });

    if(fanOptionMsg.has_full_speed()) {
        setData(other.m_fan_full_speed.m_current_value,fanOptionMsg.full_speed().current_value());
    }

    m_sysFsDriverManager->processAllUdevEvents(100);

    return {};
}

QByteArray SysFsDataProviderFanOption::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::FanOption>(data,[this,origin](legion::messages::FanOption& fanOption) { settable(fanOption,origin); });
}

void SysFsDataProviderFanOption::settable(legion::messages::FanOption &fanOption, Origin origin) const
{
    if(!fanOption.has_full_speed())
    {
        return;
    }

    fanOption.mutable_full_speed()->clear_default_value();
    fanOption.mutable_full_speed()->clear_supported();

    if(!fanOption.full_speed().has_current_value())
    {
        fanOption.clear_full_speed();
        return;
    }

    SysFsDriverLegionOther::Other other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

    if(getData(other.m_fan_full_speed.m_supported).toUShort() == 0)
    {
        skipField(origin,QString("fan full_speed=").append(QString::number(fanOption.full_speed().current_value())),"not supported");
        fanOption.clear_full_speed();
    }
}


}
