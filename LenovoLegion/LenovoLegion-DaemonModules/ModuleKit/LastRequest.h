// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once
/*
 * Values set by the clients and restored from the settings, merged into one message (a request may contain only
 * a part of the values). The older Legion firmware does not keep the values of the custom mode, they are written
 * again when the custom mode is set (the same as God mode of Lenovo Legion Toolkit).
 */
#include "KitMessage.h"

#include <QByteArray>

namespace LenovoLegionDaemon::Kit {

template<class Message>
class LastRequest
{
public:

    void remember(const QByteArray& request)
    {
        if(request.isEmpty())
        {
            return;
        }

        Message merged = m_last.isEmpty() ? Message{} : parseMessage<Message>(m_last);

        merged.MergeFrom(parseMessage<Message>(request));

        m_last = serializeMessage(merged);
    }

    const QByteArray& value() const { return m_last; }

private:

    QByteArray m_last;
};

}
