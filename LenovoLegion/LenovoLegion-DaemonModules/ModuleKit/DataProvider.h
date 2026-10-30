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
 *
 * Settable data: rules which fields of the message can be written to the hardware are only in
 * settableData(). deserializeAndSetData() writes only settableData(request,REQUEST) and saving of
 * the settings stores only settableData(current state,STATE), so a read-only or invalid value
 * (e.g. SMT control "notsupported") is neither written nor saved.
 */

#include "SysFsDriver.h"
#include "KitMessage.h"

#include <IModuleHost.h>

#include <QByteArray>
#include <QStringList>
#include <QTimerEvent>
#include <QtTypes>

#include <algorithm>
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

    /*
     * Origin of the message checked by settableData()
     */
    enum class Origin
    {
        REQUEST,    // set request (client, saved settings, profile), skipped field is a warning
        STATE       // current state read from the hardware (saving of the settings)
    };

public:

    explicit DataProvider(quint8 dataType);
    virtual ~DataProvider();

    DataProvider(const DataProvider&)            = delete;
    DataProvider& operator=(const DataProvider&) = delete;

    virtual QByteArray serializeAndGetData()                            { return {}; }
    virtual QByteArray serializeAndGetData(const QByteArray&)           { return {}; }
    virtual QByteArray deserializeAndSetData(const QByteArray&)         { return {}; }

    /*
     * Only the fields of the message which can be set in the current hardware state,
     * read-only fields are removed silently, invalid values are removed and logged (skipField)
     */
    virtual QByteArray settableData(const QByteArray& data,Origin)      { return data; }

    virtual void init()                                                 {}
    virtual void clean()                                                {}

    virtual void kernelEventHandler(const SysFsDriver::SubsystemEvent&) {}

    /*
     * Connection to the framework (set by ProviderModule before init)
     */
    void attach(Module::IModuleHost* host,EventSink sink);

protected:

    /*
     * Field removed by settableData(): warning for a request, debug for the current state
     */
    static void skipField(Origin origin,const QString& field,const QString& reason);

    /*
     * Limit with descriptors per power mode (CPUPower::Limit, GPUPower::Limit): descriptors are read-only and removed,
     * value must be supported in the mode and must be one of its steps (if any) or within <min_value,max_value>.
     * Descriptors are taken from the current state (request contains only the value).
     * Returns false if the limit has no settable value (caller clears the limit).
     */
    template<class Limit>
    static bool settableLimit(Limit& limit,const Limit& current,int mode,Origin origin,const QString& field)
    {
        const auto& descriptors = current.mode_descriptor_map();

        limit.clear_mode_descriptor_map();

        if(!limit.has_current_value())
        {
            return false;
        }

        const auto    value = limit.current_value();
        const QString name  = QString(field).append('=').append(QString::number(value));
        const auto    it    = descriptors.find(mode);

        if(it == descriptors.end() || it->second.supported() == 0)
        {
            skipField(origin,name,QString("not supported in power mode ").append(QString::number(mode)));
            return false;
        }

        const auto& descriptor = it->second;

        if(descriptor.steps_size() > 0)
        {
            if(std::find(descriptor.steps().begin(),descriptor.steps().end(),value) == descriptor.steps().end())
            {
                QStringList steps;

                for(const auto step : descriptor.steps())
                {
                    steps.append(QString::number(step));
                }

                skipField(origin,name,QString("allowed values: ").append(steps.join(',')));
                return false;
            }
        }
        else if(value < descriptor.min_value() || value > descriptor.max_value())
        {
            skipField(origin,name,QString("range <").append(QString::number(descriptor.min_value())).append(',').append(QString::number(descriptor.max_value())).append('>'));
            return false;
        }

        return true;
    }

    /*
     * settableData() of a typed message: parse, filter(message), serialize
     */
    template<class Message,class Filter>
    static QByteArray settableMessage(const QByteArray& data,Filter&& filter)
    {
        Message message = parseMessage<Message>(data);

        filter(message);

        return serializeMessage(message);
    }

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
