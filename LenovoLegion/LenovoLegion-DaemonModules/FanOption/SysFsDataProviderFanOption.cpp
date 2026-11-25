// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderFanOption.h"


#include <Drivers/SysFsDriverLegionFanMode.h>
#include <Drivers/SysFsDriverLegionOther.h>

#include <FanControl.pb.h>


#include <KitLog.h>


namespace LenovoLegionDaemon::Kit {

SysFsDataProviderFanOption::SysFsDataProviderFanOption(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}


SysFsDataProviderFanOption::FullSpeed SysFsDataProviderFanOption::fullSpeed() const
{
    if(!m_sysFsDriverManager->isLoaded(SysFsDriverLegionOther::DRIVER_NAME) && m_sysFsDriverManager->isLoaded(SysFsDriverLegionFanMode::DRIVER_NAME))
    {
        const SysFsDriverLegionFanMode::FanMode fanMode(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionFanMode::DRIVER_NAME));

        if(fanMode.m_fanFullSpeed.exists())
        {
            return { fanMode.m_fanFullSpeed.m_current_value,fanMode.m_fanFullSpeed.m_default_value,fanMode.m_fanFullSpeed.m_supported,SysFsDriverLegionFanMode::DRIVER_NAME };
        }
    }

    /*
     * Other mode (throws DRIVER_NOT_AVAILABLE when it is not loaded)
     */
    const SysFsDriverLegionOther::Other other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

    return { other.m_fan_full_speed.m_current_value,other.m_fan_full_speed.m_default_value,other.m_fan_full_speed.m_supported,SysFsDriverLegionOther::DRIVER_NAME };
}

QByteArray SysFsDataProviderFanOption::serializeAndGetData()
{
    legion::messages::FanOption         fanOptionMsg;
    QByteArray                          byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        const FullSpeed full = fullSpeed();

        /*
         * Not shown by the kernel driver when the firmware does not have it (capability data of the other mode)
         */
        if(full.m_supported.empty())
        {
            fanOptionMsg.mutable_full_speed()->set_supported(false);
        }
        else
        {
            fanOptionMsg.mutable_full_speed()->set_supported(getData(full.m_supported).toUShort() > 0);
            fanOptionMsg.mutable_full_speed()->set_default_value(getData(full.m_default_value).toUShort() == 1);
            fanOptionMsg.mutable_full_speed()->set_current_value(getData(full.m_current_value).toUShort() == 1);
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        /*
         * Lenovo Legion kernel driver not loaded: an error, not an empty message. The GUI shows the data type as
         * not available (pages grayed, Help system check) and no empty settings are saved over the saved ones.
         */
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
        }

        throw;
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
    const FullSpeed                     full = fullSpeed();
    legion::messages::FanOption         fanOptionMsg;

    LOG_T(__PRETTY_FUNCTION__);

    if(!fanOptionMsg.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    settable(fanOptionMsg,Origin::REQUEST);

    m_sysFsDriverManager->blockKernelEvent(full.m_driverName,true);
    auto cleanup =  qScopeGuard([this,&full] { m_sysFsDriverManager->blockKernelEvent(full.m_driverName,false); });

    if(fanOptionMsg.has_full_speed()) {
        setData(full.m_current_value,fanOptionMsg.full_speed().current_value());
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

    if(const auto supported = fullSpeed().m_supported; supported.empty() || getData(supported).toUShort() == 0)
    {
        skipField(origin,QString("fan full_speed=").append(QString::number(fanOption.full_speed().current_value())),"not supported");
        fanOption.clear_full_speed();
    }
}


}
