// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderGPURuntimePM.h"

#include <Drivers/SysFsDriverGPURuntimePM.h>

#include <KitLog.h>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderGPURuntimePM::SysFsDataProviderGPURuntimePM(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

QByteArray SysFsDataProviderGPURuntimePM::serializeAndGetData()
{
    legion::messages::GpuRuntimePM pm;

    LOG_T(__PRETTY_FUNCTION__);

    if(m_sysFsDriverManager->isLoaded(SysFsDriverGPURuntimePM::DRIVER_NAME))
    {
        const SysFsDriverGPURuntimePM::RuntimePM driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverGPURuntimePM::DRIVER_NAME));

        pm.set_present(true);
        pm.set_auto_suspend(getData(driver.m_control).trimmed() == "auto");
        pm.set_runtime_status(getData(driver.m_status).trimmed().toStdString());
        pm.set_device(driver.m_device.filename().string());
    }

    return serializeMessage(pm);
}

QByteArray SysFsDataProviderGPURuntimePM::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::GpuRuntimePM pm = parseMessage<legion::messages::GpuRuntimePM>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(pm,DataProvider::Origin::REQUEST);

    if(!pm.has_auto_suspend())
    {
        return {};
    }

    const SysFsDriverGPURuntimePM::RuntimePM driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverGPURuntimePM::DRIVER_NAME));

    setData(driver.m_control,std::string_view(pm.auto_suspend() ? "auto" : "on"));

    return {};
}

QByteArray SysFsDataProviderGPURuntimePM::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::GpuRuntimePM>(data,[this,origin](legion::messages::GpuRuntimePM& pm) { settable(pm,origin); });
}

void SysFsDataProviderGPURuntimePM::settable(legion::messages::GpuRuntimePM &pm, Origin origin) const
{
    pm.clear_present();
    pm.clear_runtime_status();
    pm.clear_device();

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverGPURuntimePM::DRIVER_NAME))
    {
        if(pm.has_auto_suspend())
        {
            skipField(origin,"GPU runtime power management","no discrete NVIDIA GPU");
        }

        pm.Clear();
    }
}

}
