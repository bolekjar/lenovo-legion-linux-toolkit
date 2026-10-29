// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Sysfs driver of a daemon module (internal part of the module, not shared at runtime).
 * Same interface as the daemon SysFsDriver, without QObject: kernelEvent() is a plain method
 * ("emit kernelEvent(...)" still works, emit is an empty Qt macro) delivered to the owning module.
 */

#include "KitException.h"
#include "KitLog.h"

#include <ModuleTypes.h>

#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>

#include <filesystem>
#include <functional>

namespace LenovoLegionDaemon::Kit {

class SysFsDriver
{
public:

    using exception_T = DriverException;

    enum ERROR_CODES : int {
        VALIDATION_ERROR     = -1,
        DRIVER_NOT_AVAILABLE = -2,
    };

    using DescriptorType              = QMap<QString,std::filesystem::path>;
    using DescriptorsInVectorType     = QVector<DescriptorType>;

    struct KernelEvent
    {
        using Filter = Module::KernelEvent::Filter;
        using Event  = Module::KernelEvent;
    };

    struct SubsystemEvent
    {
        enum Action : int
        {
            RELOADED        = 0,
            CHANGED         = 1,
            DRIVER_SPECIFIC = 2
        };

        QString m_driverName;
        Action  m_action;
        QString m_DriverSpecificEventType;
        QString m_DriverSpecificEventValue;
    };

    using EventSink = std::function<void(const SubsystemEvent& event)>;

public:

    SysFsDriver(const QString & name,const std::filesystem::path& path,const KernelEvent::Filter& filter = KernelEvent::Filter(),QString module = QString());
    virtual ~SysFsDriver() = default;

    SysFsDriver(const SysFsDriver&)            = delete;
    SysFsDriver& operator=(const SysFsDriver&) = delete;

    virtual void init() = 0;
    virtual void clean();
    virtual bool isLoaded()         const;
    virtual void validate()         const;
    virtual void handleKernelEvent(const KernelEvent::Event& event);
    virtual void blockKernelEvent(bool block);

    virtual const DescriptorType& desriptor() const;
    virtual const DescriptorsInVectorType& descriptorsInVector() const;

    /*
     * Receiver of driver events (set by SysFsDriverManager)
     */
    void setEventSink(EventSink sink);

protected:

    /*
     * Driver event, delivered to the module
     */
    void kernelEvent(const SubsystemEvent& event);

protected:

    DescriptorType          m_descriptor;
    DescriptorsInVectorType m_descriptorsInVector;
    bool                    m_blockKernelEvent = false;

public:

    const QString               m_name;
    const std::filesystem::path m_path;
    const KernelEvent::Filter   m_filter;
    const QString               m_module;

private:

    EventSink                   m_sink;
};

}
