// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>
#include <Drivers/SysFsDriverCPUAmdPState.h>

#include <MessageRegistry.pb.h>
#include <CPUOptions.pb.h>

#include <google/protobuf/util/message_differencer.h>

namespace LenovoLegionDaemon::Kit {

/*
 * AMD P-State: mode of the driver, boost and EPP of all policies, preferred core (read only). The values are watched by
 * a timer (another tool may change them), a change is told by CPU_AMD_PSTATE_CHANGED.
 */
class SysFsDataProviderCPUAmdPState : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUAmdPState(SysFsDriverManager* sysFsDriverManager);

    QByteArray serializeAndGetData()                         override;
    QByteArray deserializeAndSetData(const QByteArray &data) override;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

    void init()      override;
    void clean()     override;

private:

    legion::messages::CPUAmdPState read() const;

    QStringList settableEpp(const SysFsDriverCPUAmdPState::AmdPState& amd) const;   // available EPP values which can be written
    void settable(legion::messages::CPUAmdPState& pstate,Origin origin) const;

    void timerEvent(QTimerEvent *event) override;

    /*
     * "disable" unloads the scaling driver, it is never set by the daemon
     */
    inline static const QStringList SETTABLE_STATUSES {"active","guided","passive"};

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_AMD_PSTATE;

private:

    legion::messages::CPUAmdPState             m_last;
    google::protobuf::util::MessageDifferencer m_differencer;
    int                                        m_timerId = -1;
};

}
