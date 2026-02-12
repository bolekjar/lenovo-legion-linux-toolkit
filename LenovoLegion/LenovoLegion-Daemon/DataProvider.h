// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "Core/ExceptionBuilder.h"
#include <QObject>

#include "SysFsDriver.h"

namespace LenovoLegionDaemon {


class DataProvider : public QObject
{
    Q_OBJECT
public:
    DEFINE_EXCEPTION(DataProvider)
public:

    enum ERROR_CODES : int {
        INVALID_DATA                        = -1,
        SERIALIZE_ERROR                     = -2
    };

public:

    DataProvider(QObject* parent,quint8  dataType);
    DataProvider(const DataProvider&) = delete;
    DataProvider(const DataProvider&&) = delete;
    DataProvider& operator=(const DataProvider&) = delete;
    DataProvider& operator=(const DataProvider&&) = delete;


    virtual ~DataProvider() = default;

    virtual QByteArray serializeAndGetData()                                                                                             const {return {};};
    virtual QByteArray serializeAndGetData(const QByteArray&)                                                                            const {return {};};
    virtual QByteArray deserializeAndSetData(const QByteArray&)                                                                                {return {};};

    virtual void init()                 {};
    virtual void clean()                {};

    virtual void kernelEventHandler(const LenovoLegionDaemon::SysFsDriver::SubsystemEvent &)   {};

private:

public:

   /*
    * Data type identifier
    */
   const quint8  m_dataType;
};

}
