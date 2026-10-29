// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Data provider of a daemon module. Same interface as the daemon DataProvider, without QObject:
 *  - dataProviderEvent() is a plain method ("emit dataProviderEvent(...)" still works) delivered to the module,
 *  - startTimer()/killTimer()/timerEvent() use timers of the framework (IModuleHost::addTimer).
 */

#include "SysFsDriver.h"

#include <IModuleHost.h>

#include <QByteArray>
#include <QTimerEvent>
#include <QtTypes>

#include <chrono>
#include <functional>
#include <map>

namespace LenovoLegionDaemon::Kit {

class DataProvider
{
public:

    using exception_T = ProviderException;

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

    using EventSink = std::function<void(const SubsystemEvent& event)>;

public:

    explicit DataProvider(quint8 dataType);
    virtual ~DataProvider();

    DataProvider(const DataProvider&)            = delete;
    DataProvider& operator=(const DataProvider&) = delete;

    virtual QByteArray serializeAndGetData()                            { return {}; }
    virtual QByteArray serializeAndGetData(const QByteArray&)           { return {}; }
    virtual QByteArray deserializeAndSetData(const QByteArray&)         { return {}; }

    virtual void init()                                                 {}
    virtual void clean()                                                {}

    virtual void kernelEventHandler(const SysFsDriver::SubsystemEvent&) {}

    /*
     * Connection to the framework (set by ProviderModule before init)
     */
    void attach(Module::IModuleHost* host,EventSink sink);

protected:

    /*
     * Provider event, delivered to the module
     */
    void dataProviderEvent(const SubsystemEvent& event);

    /*
     * Timers of the framework, timerEvent() is called from the daemon event loop
     */
    int  startTimer(std::chrono::milliseconds interval);
    void killTimer(int timerId);

    virtual void timerEvent(QTimerEvent*)                               {}

public:

    const quint8  m_dataType;

private:

    Module::IModuleHost*                m_host = nullptr;
    EventSink                           m_sink;
    std::map<int,Module::TimerId>       m_timers;
    int                                 m_nextTimerId = 1;
};

}
