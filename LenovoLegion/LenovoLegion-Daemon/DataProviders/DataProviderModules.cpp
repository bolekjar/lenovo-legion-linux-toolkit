// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "DataProviderModules.h"
#include "DataProviderManager.h"
#include "../Modules/ModuleDataProviderAdapter.h"

#include "../LenovoLegion-PrepareBuild/DaemonSettings.pb.h"

#include <Core/Application.h>
#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

DataProviderModules::DataProviderModules(DataProviderManager* dataProviderManager) :
    DataProvider(dataProviderManager, dataType),
    m_dataProviderManager(dataProviderManager)
{}

void DataProviderModules::setNotLoaded(const std::vector<ModuleLoader::NotLoaded> &notLoaded)
{
    m_notLoaded = notLoaded;
}

QByteArray DataProviderModules::serializeAndGetData()
{
    legion::messages::ModuleList list;
    QByteArray                   byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    m_dataProviderManager->forEachDataProviderDo([&list](DataProvider& provider) {
        auto* module = list.add_modules();

        module->set_data_type(provider.m_dataType);

        if(const auto* adapter = dynamic_cast<const ModuleDataProviderAdapter*>(&provider))
        {
            module->set_name(adapter->info().name);
            module->set_version(adapter->info().version);
            module->set_framework(false);

            if(adapter->running())
            {
                module->set_state(legion::messages::ModuleList::Module::STATE_RUNNING);
            }
            else
            {
                module->set_state(legion::messages::ModuleList::Module::STATE_FAILED);
                module->set_error(adapter->error().toStdString());
            }
        }
        else
        {
            /*
             * Provider of the framework: named by its data type, version of the daemon
             */
            module->set_name(legion::messages::DataType_Name(static_cast<legion::messages::DataType>(provider.m_dataType)));
            module->set_version(bj::framework::Application::app_version);
            module->set_framework(true);
            module->set_state(legion::messages::ModuleList::Module::STATE_RUNNING);
        }
    });

    for(const auto& notLoaded : m_notLoaded)
    {
        auto* module = list.add_modules();

        module->set_name(notLoaded.m_name.toStdString());
        module->set_state(legion::messages::ModuleList::Module::STATE_NOT_LOADED);
        module->set_error(notLoaded.m_reason.toStdString());
        module->set_framework(false);
    }

    byteArray.resize(list.ByteSizeLong());
    if(!list.SerializeToArray(byteArray.data(), byteArray.size()))
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Serialize of data message error !");
    }

    return byteArray;
}

}
