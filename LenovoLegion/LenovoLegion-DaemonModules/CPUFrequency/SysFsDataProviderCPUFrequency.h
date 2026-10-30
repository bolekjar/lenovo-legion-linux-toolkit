// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include <MessageRegistry.pb.h>
#include <CPUFrequency.pb.h>

#include <google/protobuf/util/message_differencer.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderCPUFrequency : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUFrequency(SysFsDriverManager* sysFsDriverManager);


    virtual void init()      override;
    virtual void clean()     override;



    virtual QByteArray serializeAndGetData()                     override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)  override;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_FREQUENCY;


private:

    /*
     * scalingLimitsOnly - read only online, scaling_min_freq and scaling_max_freq (fields tracked by m_differencer),
     *                     used by the periodic check, structure of the message is the same as for the full read
     */
    legion::messages::CPUFrequency readCPPFrequency(bool scalingLimitsOnly = false) const;

    void settable(legion::messages::CPUFrequency& frequency,Origin origin) const;

    void timerEvent(QTimerEvent *event) override;

private:

    legion::messages::CPUFrequency m_lastCpuFrequency;

    google::protobuf::util::MessageDifferencer m_differencer;

    int m_timerId = -1;
};

}
