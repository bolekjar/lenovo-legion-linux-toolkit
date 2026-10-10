// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include "../LenovoLegion-PrepareBuild/MessageRegistry.pb.h"
#include "../LenovoLegion-PrepareBuild/CPUFrequency.pb.h"

#include <google/protobuf/util/message_differencer.h>

namespace LenovoLegionDaemon {

class SysFsDataProviderCPUFrequency : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUFrequency(SysFsDriverManager* sysFsDriverManager,QObject* parent);


    virtual void init()      override;
    virtual void clean()     override;



    virtual QByteArray serializeAndGetData()                     override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)  override;

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_FREQUENCY;


private:

    legion::messages::CPUFrequency readCPPFrequency() const;

    void timerEvent(QTimerEvent *event) override;

private:

    legion::messages::CPUFrequency m_lastCpuFrequency;

    google::protobuf::util::MessageDifferencer m_differencer;

    int m_timerId = -1;
};

}
