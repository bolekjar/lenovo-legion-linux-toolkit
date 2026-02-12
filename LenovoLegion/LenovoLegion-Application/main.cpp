// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <Application.h>


#include <Core/Application.h>
#include <Core/LoggerHolder.h>

#include <QDebug>

int main(int argc, char *argv[])
{
    try {
        bj::framework::Application app(std::make_unique<LenovoLegionGui::Application>(argc,argv),bj::framework::Application::apps_names[0]);

#ifdef QT_NO_DEBUG
        app.setLogingSeverityLevel(bj::framework::Logger::SEVERITY_BITSET(
             (1 << bj::framework::Logger::SEVERITY::INFO)     |
             (1 << bj::framework::Logger::SEVERITY::WARNING)  |
             (1 << bj::framework::Logger::SEVERITY::ERROR)
             ));
#endif

        app.init();
        app.start();
    }
    catch(const bj::framework::Logger::exception_T& ex)
    {
        qCritical() << bj::framework::exception::ExceptionBuilder::print(ex).c_str();
    }
    catch(const bj::framework::exception::Exception& ex)
    {
        LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
        return EXIT_FAILURE;
    }
    catch(const std::exception& ex)
    {
         LOG_E(bj::framework::exception::ExceptionBuilder::print(__FILE__,__PRETTY_FUNCTION__,__LINE__,1,ex).c_str());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_E(bj::framework::exception::ExceptionBuilder::print(__FILE__,__PRETTY_FUNCTION__,__LINE__,1,"Internal error !").c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
