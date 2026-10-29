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

    struct SubsystemEvent
    {
        enum Action : int
        {
            CHANGED         = 0,
        };

        quint8  m_dataType;
        Action  m_action;
        QString m_DataProviderSpecificEventType;
        QString m_DataProviderSpecificEventValue;
    };

public:

    DataProvider(QObject* parent,quint8  dataType);
    DataProvider(const DataProvider&) = delete;
    DataProvider(const DataProvider&&) = delete;
    DataProvider& operator=(const DataProvider&) = delete;
    DataProvider& operator=(const DataProvider&&) = delete;


    virtual ~DataProvider() = default;

    virtual QByteArray serializeAndGetData()                                                                                             {return {};};
    virtual QByteArray serializeAndGetData(const QByteArray&)                                                                            {return {};};
    virtual QByteArray deserializeAndSetData(const QByteArray&)                                                                          {return {};};

    virtual void init()                 {};
    virtual void clean()                {};


    /*
     * Restore saved settings to the hardware / save current state (daemon modules)
     */
    virtual void loadSettings()         {};
    virtual void saveSettings()         {};


signals:

    void dataProviderEvent(const LenovoLegionDaemon::DataProvider::SubsystemEvent& event);

    /*
     * Ready notification payload for the client (daemon modules), sent as it is
     */
    void notification(const QByteArray& payload);

private:

public:

   /*
    * Data type identifier
    */
   const quint8  m_dataType;
};

}
