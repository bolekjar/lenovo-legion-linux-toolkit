// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "DataProvider.h"
#include <Core/ExceptionBuilder.h>

#include <nvml.h>
namespace  LenovoLegionDaemon {

class DataProviderNvidiaNvml : public DataProvider
{
    Q_OBJECT

public:

    DEFINE_EXCEPTION(DataProviderNvidiaNvml)

    enum  : quint8 {
        ERROR_NVML_INIT_FAILED            = 1,
        ERROR_NVML_DEVICE_COUNT_FAILED    = 2
    };

public:

    DataProviderNvidiaNvml(QObject* parent);
    virtual ~DataProviderNvidiaNvml() override;

    virtual QByteArray serializeAndGetData()                      const override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)         override;


    virtual void init() override;
    virtual void clean() override;

private:

    void cleanUp();

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


    nvmlDevice_t m_device;  
public:

    static constexpr quint8  dataType = 13;
};


}
