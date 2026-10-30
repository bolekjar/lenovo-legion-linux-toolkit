// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include <Other.pb.h>

#include <MessageRegistry.pb.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderOtherGpuSwitch : public SysFsDataProvider
{
    enum GSyncState {
        GSYNC_DISABLED    = 0,
        GSYNC_ENABLED     = 1
    };

    enum IGPUModeState {
        IGPU_MODE_DEFAULT     = 0,
        IGPU_MODE_IGPUONLY    = 1,
        IGPU_MODE_AUTO        = 2
    };

    struct PackedGPUSettings {
        const GSyncState    m_gSyncState;
        const IGPUModeState m_iGPUModeState;
    };

public:

    SysFsDataProviderOtherGpuSwitch(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                    ;
    virtual QByteArray deserializeAndSetData(const QByteArray&) ;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    void settable(legion::messages::GpuSwitchValue& gpuSwitch,Origin origin) const;

    PackedGPUSettings   packGPUSettings(const legion::messages::GpuSwitchValue& gpuSwitchValue) const;
    legion::messages::GpuSwitchValue unPackGPUSettings(const PackedGPUSettings& packedGPUSettings) const;

public:

    static constexpr quint8  dataType = legion::messages::DataType::OTHER_GPU_SWITCH;
};

}
