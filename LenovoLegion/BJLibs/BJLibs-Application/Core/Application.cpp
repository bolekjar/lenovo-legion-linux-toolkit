// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Application.h"
#include "ApplicationEnviroment.h"

#include <unistd.h>


namespace bj { namespace framework {

const std::array<const char * const,11>             Application::apps_names  = {BL_APP_NAME_STR, BL_APP_NAME2_STR,BL_APP_NAME3_STR,BL_APP_NAME4_STR,BL_APP_NAME5_STR,BL_APP_NAME6_STR,BL_APP_NAME7_STR,BL_APP_NAME8_STR BL_APP_NAME9_STR,BL_APP_NAME10_STR,nullptr};
const char * const                                  Application::app_version = BL_APP_VERSION_STR;
const char * const                                  Application::data_dir    = BL_DATA_DIR_STR;
const char * const                                  Application::log_dir     = BL_LOG_DIR_STR;
const char * const                                  Application::modules_dir = BL_MODULES_DIR_STR;

Application::Application(std::unique_ptr<ApplicationInterface> app,const char * const app_name) :
     m_application(std::move(app)),
     m_state(STATE::IDLE),
     m_terminated(true),
     m_runGuard(app_name)
{}

Application::~Application() noexcept
{}


void Application::init()
{
    if(m_state != STATE::IDLE){
        THROW_EXCEPTION(exception_T,ERROR_CODES::UNEXPECTED_STATE_ERROR,"Application was initialized !")
    }

    try {
        if(!m_runGuard.tryToRun())
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ANOTHER_INSTANCE_RUNNIG,"Application is already runned !")
        }

        m_modules.setPath(m_application->modulesPath());
        m_application->appInitImpl(std::make_unique<ModulesHandler_T>(m_modules));
    } catch(...) //rollback
    {
        m_application->appRollBackImpl();
        throw;
    }
    m_state = STATE::INITIALIZED;
}

void Application::start()
{
    if(m_state != STATE::INITIALIZED)
        THROW_EXCEPTION(exception_T,ERROR_CODES::UNEXPECTED_STATE_ERROR,"Application was not initialized !")

    try {
        m_state = STATE::RUNNING;
        m_application->appStartImpl();

        /*
         * Signals at the end application must be runnig !
         */

        /*
         * Main loop application
         */
        m_terminated = false;
        m_application->appMainLoop();

    } catch (...)
    {
        stop();
        throw;
    }

    stop();
}

void Application::stop() noexcept
{
    if(m_state != STATE::RUNNING)
        return;

    m_application->appStopImpl();


    /*
     * Signals must be at the end!
     **/


    m_terminated = true;

    m_state = STATE::INITIALIZED;
}

bool Application::terminated() const
{
    return m_terminated;
}

void Application::setLogingSeverityLevel(const Logger::SEVERITY_BITSET &severity) {
    m_application->setLogingSeverityLevel(severity);
}

Application::STATE Application::state() const
{
    return m_state;
}

}}
