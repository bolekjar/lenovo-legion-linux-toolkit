// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "SysFsDriver.h"

#include <Core/ExceptionBuilder.h>

#include <QObject>
#include <QString>

#include <map>

#include <libudev.h>

class QSocketNotifier;

namespace LenovoLegionDaemon {

class SysFsDriverManager : public QObject
{
    Q_OBJECT

private:

    const static SysFsDriver::KernelEvent::Filter MODULE_SUBSYSTEM_EVENT_FILTER;
public:

    DEFINE_EXCEPTION(SysFsDriverManager);

    enum ERROR_CODES : int {
        DRIVER_NOT_FOUND        = -1,
        DRIVER_ALREADY_LOADED   = -2,
        UDEV_INITIALIZED_ERROR  = -3,
    };

    struct ModuleSubsystemEvent
    {
        enum class Action
        {
            ADD,
            REMOVE,
        };

        QString m_moduleName;
        Action  m_action;
    };

public:

    explicit SysFsDriverManager(QObject *parent = nullptr);

    ~SysFsDriverManager();

    void addDriver(SysFsDriver* driver);

    void initDrivers();

    void cleanDrivers();


    void  blockKernelEvent(const QString& driverName,bool block);
    void  blockSignals(const QString& driverName,bool block);
    void  refreshDriver(const QString& driverName);

    const SysFsDriver::DescriptorType&          getDriverDesriptor(const QString& driverName) const;
    const SysFsDriver::DescriptorsInVectorType& getDriverDescriptorsInVector(const QString& driverName) const;

    void processAllUdevEvents(int timeoutInMiliseconds);

private slots:

    void onDataReceived(int socket);
    void onKernelEvent(const LenovoLegionDaemon::SysFsDriver::SubsystemEvent& event);

signals:

    void moduleSubsystem(const LenovoLegionDaemon::SysFsDriverManager::ModuleSubsystemEvent& event);
    void kernelEvent(const LenovoLegionDaemon::SysFsDriver::SubsystemEvent& event);

private:

    void addUdevMonitorFilter(const SysFsDriver::KernelEvent::Filter& filter);
    void reconnectUdevMonitor();

private:

    struct udev         *m_udev;
    struct udev_monitor *m_mon;

    QSocketNotifier     *m_socketNotifier;

    std::map<QString,SysFsDriver *> m_drivers;
};

}
