// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include <Core/ApplicationModulesHandler.h>
#include <Core/Logger.h>

#include <QDir>

namespace bj { namespace framework {


class ApplicationInterface
{

public:

    using ApplicationModulesHandler_T = ApplicationModulesHandler;

public:



    virtual  ~ApplicationInterface() {}

    /*
     * Application function, main application thread
     */
    virtual void appRollBackImpl() noexcept                                 = 0;
    virtual void appStartImpl()                                             = 0;
    virtual void appInitImpl(std::unique_ptr<ApplicationModulesHandler_T>)  = 0;
    virtual void appStopImpl()     noexcept                                 = 0;
    virtual void appMainLoop()                                              = 0;
    virtual void setLogingSeverityLevel(const Logger::SEVERITY_BITSET& severity)   = 0;

    /*
     * Application getters, must be available before init
     */
    virtual const QDir  modulesPath() const     = 0;
};

}}
