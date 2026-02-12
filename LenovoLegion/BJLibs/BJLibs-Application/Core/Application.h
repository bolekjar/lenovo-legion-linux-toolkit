// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ExceptionBuilder.h"
#include "RunGuard.h"
#include "ApplicationInterface.h"
#include "ApplicationModulesLoader.h"
#include "Logger.h"

#include <atomic>
#include <memory>
#include <array>


namespace bj { namespace framework {

class Application
{

public:
    static const std::array<const char * const,11>      apps_names;     //Application names list
    static const char * const                           app_version;

    static const char * const                    data_dir;
    static const char * const                    log_dir;
    static const char * const                    modules_dir;

public:

    enum class STATE {
        IDLE,
        INITIALIZED,
        RUNNING
    };

    enum ERROR_CODES : int {
        APPLICATION_PATH_STRUCTURE_ERROR    = -1,
        UNEXPECTED_STATE_ERROR              = -2,
        APPLICATION_IS_NOT_IN_MAIN_THREAD   = -3,
        APPLICATION_INTERNALL               = -4,
        INVALID_LOG_LEVEL                   = -5,
        ANOTHER_INSTANCE_RUNNIG             = -6
    };

    typedef std::atomic<bool>                    terminatedFlag_T;
    using   ModulesHandler_T       =             ApplicationModulesHandler;
private:

    std::unique_ptr<ApplicationInterface>        m_application;

    std::atomic<STATE>                           m_state;

    terminatedFlag_T                             m_terminated;


    ApplicationModulesLoader                     m_modules;

protected:

    RunGuard                                     m_runGuard;

public:

    Application(const Application&) = delete;
    Application(Application&&)      = delete;

    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&)      = delete;


public:

    /**********************************************************
     * Specific exception for FTAppplication
     **********************************************************/
    DEFINE_EXCEPTION(Application);

    /*
     * In your application you can create only one einstance.
     */
    Application(std::unique_ptr<ApplicationInterface> app, const char * const app_name);

    virtual ~Application() noexcept;


    /*
     * Init application, no start threads,          no thread safe
     * It must be trigerred in main application
     */
    virtual void init() final;

    /*
     * Start all components, start threads,         no thread safe
     * It must be trigerred in main application
     */
    virtual void start() final;

    /*
     * Stop components, it can be called any time,  no thread safe
     * It must be trigerred in main application
     */
    virtual void stop() noexcept final;

protected:

    /*
     * Check if app must be terminated
     **/
    bool terminated() const;

public:

    /*
     * Enable/Disable severity loging default all enabled
     */
    void setLogingSeverityLevel(const Logger::SEVERITY_BITSET& severity);

    /*
     * Return application state
     */
    STATE  state()               const;
};

}}
