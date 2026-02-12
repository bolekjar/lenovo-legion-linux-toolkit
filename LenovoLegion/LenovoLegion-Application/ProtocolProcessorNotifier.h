// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ProtocolProcessorBase.h"

#include <Core/ExceptionBuilder.h>


#include "../LenovoLegion-PrepareBuild/Notification.pb.h"


namespace LenovoLegionGui {


class ProtocolProcessorNotifier : public ProtocolProcessorBase
{
    Q_OBJECT

public:

    enum ERROR_CODES : int {
        INVALID_MESSAGE   = -1
    };

    DEFINE_EXCEPTION(ProtocolProcessorNotifier);

public:

    explicit ProtocolProcessorNotifier(QObject *parent = nullptr);
    virtual ~ProtocolProcessorNotifier();

protected:

    void timerEvent(QTimerEvent *event) override;

signals:

    void daemonNotification(const legion::messages::Notification& msg);

private:

    int m_timerId;
};


}
