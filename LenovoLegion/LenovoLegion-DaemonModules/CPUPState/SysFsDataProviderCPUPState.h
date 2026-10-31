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

    QByteArray settableData(const QByteArray& data,Origin origin) override;


    virtual void init()      override;
    virtual void clean()     override;

private:

    legion::messages::CPUPState readCPPPState() const;
    void settable(legion::messages::CPUPState& pstate,Origin origin) const;

    /*
     * "off" unloads the scaling driver, it is never set by the daemon
     */
    inline static const QStringList SETTABLE_STATUSES {"active","passive"};

    void timerEvent(QTimerEvent *event) override;

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_PSTATE;

    legion::messages::CPUPState m_lastCpuPState;

    google::protobuf::util::MessageDifferencer m_differencer;

    int m_timerId = -1;
};

}
