// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ModuleHost.h"

#include "../SysFsDriverManager.h"

#include <Core/LoggerHolder.h>

#include <QTimer>

namespace LenovoLegionDaemon {

ModuleHost::ModuleHost(const QString &moduleName, SysFsDriverManager *sysFsDriverManager, QObject *parent) :
    QObject(parent),
    m_moduleName(moduleName),
    m_sysFsDriverManager(sysFsDriverManager),
    m_module(nullptr),
    m_logger(moduleName),
    m_kernelEventsBlocked(false),
    m_nextTimerId(0)
{}

ModuleHost::~ModuleHost()
{
    release();
}

void ModuleHost::setModule(Module::IModule *module)
{
    m_module = module;
}

void ModuleHost::release()
{
    for(auto& timer : m_timers)
    {
        timer.second->stop();
        timer.second->deleteLater();
    }

    m_timers.clear();

    if(!m_sysFsDriverManager.isNull())
    {
        m_sysFsDriverManager->removeKernelEventListeners(this);
    }

    m_kernelEventsBlocked = false;
}

bool ModuleHost::callModule(const QString &what, const std::function<void ()> &call) noexcept
{
    try {
        call();
        return true;
    }
    catch(const bj::framework::exception::Exception& ex)
    {
        LOG_E(QString("Module ").append(m_moduleName).append(" ").append(what).append(" failed: ").append(bj::framework::exception::ExceptionBuilder::print(ex).c_str()));
    }
    catch(const std::exception& ex)
    {
        LOG_E(QString("Module ").append(m_moduleName).append(" ").append(what).append(" failed: ").append(ex.what()));
    }
    catch(...)
    {
        LOG_E(QString("Module ").append(m_moduleName).append(" ").append(what).append(" failed: unknown error !"));
    }

    return false;
}

const QString &ModuleHost::moduleName() const
{
    return m_moduleName;
}

Module::ILogger &ModuleHost::logger()
{
    return m_logger;
}

void ModuleHost::addKernelEventFilter(const Module::KernelEvent::Filter &filter)
{
    if(m_sysFsDriverManager.isNull())
    {
        throw Module::Error("Kernel events are not available");
    }

    m_sysFsDriverManager->addKernelEventListener(this,filter,[this](const Module::KernelEvent& event) {

        if(m_kernelEventsBlocked || m_module == nullptr)
        {
            return;
        }

        callModule("kernel event",[this,&event] { m_module->onKernelEvent(event); });
    });
}

void ModuleHost::blockKernelEvents(bool block)
{
    m_kernelEventsBlocked = block;
}

void ModuleHost::processPendingKernelEvents(std::chrono::milliseconds timeout)
{
    if(!m_sysFsDriverManager.isNull())
    {
        m_sysFsDriverManager->processAllUdevEvents(static_cast<int>(timeout.count()));
    }
}

void ModuleHost::notify(const QByteArray &payload)
{
    emit notification(payload);
}

Module::TimerId ModuleHost::addTimer(std::chrono::milliseconds interval, Module::TimerCallback callback)
{
    const Module::TimerId id    = m_nextTimerId++;
    QTimer*               timer = new QTimer(this);

    connect(timer,&QTimer::timeout,this,[this,callback = std::move(callback)] {

        if(m_module == nullptr)
        {
            return;
        }

        callModule("timer",callback);
    });

    timer->start(interval);

    m_timers[id] = timer;

    return id;
}

void ModuleHost::removeTimer(Module::TimerId id)
{
    const auto it = m_timers.find(id);

    if(it != m_timers.end())
    {
        it->second->stop();
        it->second->deleteLater();
        m_timers.erase(it);
    }
}


ModuleHost::Logger::Logger(const QString &moduleName) :
    m_prefix(QString("[").append(moduleName).append("] "))
{}

void ModuleHost::Logger::trace(const QString &message)
{
    LOG_T(QString(m_prefix).append(message));
}

void ModuleHost::Logger::debug(const QString &message)
{
    LOG_D(QString(m_prefix).append(message));
}

void ModuleHost::Logger::info(const QString &message)
{
    LOG_I(QString(m_prefix).append(message));
}

void ModuleHost::Logger::warning(const QString &message)
{
    LOG_W(QString(m_prefix).append(message));
}

void ModuleHost::Logger::error(const QString &message)
{
    LOG_E(QString(m_prefix).append(message));
}

}
