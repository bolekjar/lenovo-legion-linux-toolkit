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

#include <Message.h>

#include <QObject>
#include <QLocalSocket>

namespace LenovoLegionGui {


class ProtocolProcessor : public ProtocolProcessorBase
{
    Q_OBJECT


public:

    enum ERROR_CODES : int {
        TIMEOUT_ERROR     = -1,
        INVALID_MESSAGE   = -2
    };


    DEFINE_EXCEPTION(ProtocolProcessor);

public:

    explicit ProtocolProcessor(QObject *parent = nullptr);
    virtual ~ProtocolProcessor();


    QByteArray getDataRequest(quint8 dataType, const QByteArray& data = {});
    QByteArray setDataRequest(quint8 dataType, const QByteArray& data);

};


}
