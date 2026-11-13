// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <DataProvider.h>
#include <KitException.h>

#include <MessageRegistry.pb.h>
#include <NvidiaNvml.pb.h>

#include <nvml.h>

#include <chrono>
#include <filesystem>
#include <optional>
namespace LenovoLegionDaemon::Kit {

class DataProviderNvidiaNvml : public DataProvider
{

public:

    DEFINE_EXCEPTION(DataProviderNvidiaNvml)

    enum  : quint8 {
        ERROR_NVML_INIT_FAILED            = 1,
        ERROR_NVML_DEVICE_COUNT_FAILED    = 2
    };

public:

    DataProviderNvidiaNvml();
    virtual ~DataProviderNvidiaNvml() override;

    virtual QByteArray serializeAndGetData()                            override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)         override;

    QByteArray settableData(const QByteArray& data,Origin origin)       override;


    virtual void init() override;
    virtual void clean() override;

protected:

    void timerEvent(QTimerEvent *event) override;

private:

    void cleanUp();

    /*
     * NVML is opened lazily by the first request and closed after idle time (idle client may prevent runtime D3 of the GPU),
     * returns false if NVML or the GPU is not available. Failed open is retried at most once per OPEN_RETRY.
     */
    bool open();
    void close();

    void settable(legion::messages::NvidiaNvml& nvml,Origin origin) const;

    /*
     * Data read from the running GPU (NVML queries wake up a suspended GPU)
     */
    void readDynamicData(legion::messages::NvidiaNvml& gpuData);

    /*
     * GPU is in runtime D3 (or going to it), unknown status is not suspended
     */
    bool isSuspended() const;

    /*
     * *************Static data*********************
     */
    quint32      m_maxGraphicsClock;
    quint32      m_maxSmClock;
    quint32      m_maxMemClock;
    quint32      m_shutdownTempThreshold;
    quint32      m_slowdownTempThreshold;
    quint32      m_powerLimitMax;
    quint32      m_powerLimitMin;
    quint32      m_pciGenerationMax;
    quint32      m_pciWidthMax;
    qint32       m_minGpuOffset;
    qint32       m_minMemOffset;
    qint32       m_maxGpuOffset;
    qint32       m_maxMemOffset;
    QString      m_GPUName;
    /*
     * *********************************************
     */


    nvmlDevice_t m_device;                          // not null while NVML is open
    bool         m_hasGpu;                          // static data were read (kept while NVML is closed)

    std::filesystem::path   m_runtimeStatusPath;    // /sys/bus/pci/devices/<bdf>/power/runtime_status
    std::optional<int>      m_gpuOffset;            // last known clock offsets, reported while the GPU is suspended
    std::optional<int>      m_memOffset;

    using clock_T = std::chrono::steady_clock;

    int                         m_idleTimerId;
    clock_T::time_point         m_lastUse;          // last request which used NVML
    std::optional<clock_T::time_point> m_lastFailedOpen;
    bool                        m_openFailureLogged;

    static constexpr std::chrono::seconds IDLE_CLOSE{30};
    static constexpr std::chrono::seconds OPEN_RETRY{10};
    static constexpr std::chrono::seconds IDLE_CHECK{10};
public:

    static constexpr quint8  dataType = legion::messages::DataType::NVIDIA_NWML;
};


}
