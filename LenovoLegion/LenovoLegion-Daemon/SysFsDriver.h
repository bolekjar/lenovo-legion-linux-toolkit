// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ExceptionBuilder.h>

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <QSet>

#include <filesystem>

namespace LenovoLegionDaemon {


class SysFsDriver : public QObject
{
    Q_OBJECT

public:

    DEFINE_EXCEPTION(SysFsDriver);

    enum ERROR_CODES : int {
        VALIDATION_ERROR     = -1,
        DRIVER_NOT_AVAILABLE = -2,
    };

    using DescriptorType              = QMap<QString,std::filesystem::path>;
    using DescriptorsInVectorType     = QVector<DescriptorType>;


    struct KernelEvent
    {
        struct Filter {
            static constexpr std::string_view NAME      = "kernel";
            QString          m_subSystem;
            QSet<QString>    m_properties;
        };

        struct Event {
            QString m_action;
            QString m_driver;
            QString m_sysName;
            QString m_subSystem;
            QString m_devPath;
            QMap<QString,QString>  m_properties;
        };
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

public:

    explicit SysFsDriver(const QString & name,const std::filesystem::path& path,const KernelEvent::Filter& filter = KernelEvent::Filter(),QObject *parent = nullptr,QString module = QString());

    virtual ~SysFsDriver() = default;

    /*
     * Init Driver
     */
    virtual void init() = 0;

    /*
     * Clean Driver
     */
    virtual void clean();

    /*
     * Check if driver is loaded
     */
    virtual bool isLoaded()         const;

    /*
     * Validate driver
     */
    virtual void validate()         const;

    /*
     * Handle Kernel event
     */
    virtual void handleKernelEvent(const KernelEvent::Event& event);

    /*
     * Block kernel event
     */
    virtual void blockKernelEvent(bool block);

    /*
     * Get descriptors
     */
    virtual const DescriptorType& desriptor() const;
    virtual const DescriptorsInVectorType& descriptorsInVector() const;

protected:

    /*
     * Driver descriptor
     */
    DescriptorType m_descriptor;

    /*
     * Vectored descriptors
     */
    DescriptorsInVectorType m_descriptorsInVector;


    /*
     * Kernel event filter
     */
    bool m_blockKernelEvent = false;

signals:


    /*
     * Kernel event
     */
    void kernelEvent(const LenovoLegionDaemon::SysFsDriver::SubsystemEvent& m_name);

public:

    const QString               m_name;
    const std::filesystem::path m_path;
    const KernelEvent::Filter   m_filter;
    const QString               m_module;
};


}
