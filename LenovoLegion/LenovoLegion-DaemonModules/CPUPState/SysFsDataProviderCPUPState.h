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
#include <CPUOptions.pb.h>

#include <google/protobuf/util/message_differencer.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderCPUPState : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUPState(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                         override;
    virtual QByteArray deserializeAndSetData(const QByteArray &data) override;


    virtual void init()      override;
    virtual void clean()     override;

private:

    legion::messages::CPUPState readCPPPState() const;
    void timerEvent(QTimerEvent *event) override;

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_PSTATE;

    legion::messages::CPUPState m_lastCpuPState;

    google::protobuf::util::MessageDifferencer m_differencer;

    int m_timerId = -1;
};

}
