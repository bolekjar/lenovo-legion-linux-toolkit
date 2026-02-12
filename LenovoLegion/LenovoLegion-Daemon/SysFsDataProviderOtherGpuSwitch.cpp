// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderOtherGpuSwitch.h"
#include "SysFSDriverLegionGameZone.h"


#include "../LenovoLegion-PrepareBuild/Other.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderOtherGpuSwitch::SysFsDataProviderOtherGpuSwitch(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}


QByteArray SysFsDataProviderOtherGpuSwitch::serializeAndGetData() const
{
    legion::messages::GpuSwitchValue    gpuSwitchMsg;
    QByteArray                          byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    gpuSwitchMsg.set_supported(false);

    try {
        SysFSDriverLegionGameZone::GameZone gameZone(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));

        if(getData(gameZone.m_gsync.m_supported).toUShort() > 0 && getData(gameZone.m_igpuMode.m_supported).toUShort() > 0)
        {
            gpuSwitchMsg = unPackGPUSettings({static_cast<GSyncState>(getData(gameZone.m_gsync.m_current_value).toUShort()),static_cast<IGPUModeState>(getData(gameZone.m_igpuMode.m_current_value).toShort())});
            gpuSwitchMsg.set_supported(true);
        }

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            gpuSwitchMsg.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(gpuSwitchMsg.ByteSizeLong());
    if(!gpuSwitchMsg.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderOtherGpuSwitch::deserializeAndSetData(const QByteArray &data)
{
    SysFSDriverLegionGameZone::GameZone gameZone(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));
    legion::messages::GpuSwitchValue    gpuSwitchMsg;

    LOG_T(__PRETTY_FUNCTION__);

    if(!gpuSwitchMsg.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    // Convert from GpuSwitchValue to packed settings
    PackedGPUSettings packedSettings = packGPUSettings(gpuSwitchMsg);

    setData(gameZone.m_igpuMode.m_current_value, packedSettings.m_iGPUModeState);
    setData(gameZone.m_gsync.m_current_value,packedSettings.m_gSyncState);

    LOG_D(QString("GPU Switch: Setting hybrid mode to: ").append(QString::number(gpuSwitchMsg.current())));

    return {};
}

legion::messages::GpuSwitchValue SysFsDataProviderOtherGpuSwitch::unPackGPUSettings(const PackedGPUSettings &packedGPUSettings) const
{
    legion::messages::GpuSwitchValue   gpuSwitchValue;

    if(packedGPUSettings.m_gSyncState == GSYNC_DISABLED)
    {
        switch(packedGPUSettings.m_iGPUModeState)
        {
        case IGPU_MODE_IGPUONLY:
            gpuSwitchValue.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONIGPUONLY);
            break;
        case IGPU_MODE_AUTO:
            gpuSwitchValue.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONAUTO);
            break;
        case IGPU_MODE_DEFAULT:
            gpuSwitchValue.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ON);
            break;
        }
    }
    else if(packedGPUSettings.m_gSyncState == GSYNC_ENABLED && packedGPUSettings.m_iGPUModeState == IGPU_MODE_DEFAULT)
    {
        gpuSwitchValue.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_OFF);
    }

    return gpuSwitchValue;
}

SysFsDataProviderOtherGpuSwitch::PackedGPUSettings SysFsDataProviderOtherGpuSwitch::packGPUSettings(const legion::messages::GpuSwitchValue &gpuSwitchValue) const
{
    GSyncState      gSyncState     = GSYNC_DISABLED;
    IGPUModeState   iGPUModeState  = IGPU_MODE_DEFAULT;

    // G-Sync State
    if(gpuSwitchValue.has_current())
    {
        switch(gpuSwitchValue.current())
        {
        case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ON:
        {
                gSyncState = GSYNC_DISABLED;
                iGPUModeState = IGPU_MODE_DEFAULT;
        }
                break;
        case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_OFF:
        {
                gSyncState = GSYNC_ENABLED;
                iGPUModeState = IGPU_MODE_DEFAULT;
        }
                break;
        case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONIGPUONLY:
        {
                gSyncState = GSYNC_DISABLED;
                iGPUModeState = IGPU_MODE_IGPUONLY;
        }
                break;
        case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONAUTO:
        {
                gSyncState = GSYNC_DISABLED;
                iGPUModeState = IGPU_MODE_AUTO;
        }
                break;
        default:
        {
                gSyncState = GSYNC_DISABLED;
                iGPUModeState = IGPU_MODE_DEFAULT;
        }
                break;
        }
    }

    // iGPU Mode State
    return {gSyncState,iGPUModeState};
}


}
