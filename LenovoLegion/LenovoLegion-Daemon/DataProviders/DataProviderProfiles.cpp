// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderProfiles.h"
#include "DaemonSettingsManager.h"
#include "DataProviderManager.h"

#include "../LenovoLegion-PrepareBuild/Profiles.pb.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

DataProviderProfiles::DataProviderProfiles(DataProviderManager* dataProviderManager) :
    DataProvider(dataProviderManager, dataType),
    m_dataProviderManager(dataProviderManager)
{}

QByteArray DataProviderProfiles::serializeAndGetData()
{
    legion::messages::Profiles profiles;
    QByteArray                 byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    for(const auto& [name,description] : DaemonSettingsManager::getInstance().profiles())
    {
        auto* profile = profiles.add_profiles();

        profile->set_name(name.toStdString());
        profile->set_description(description.toStdString());
    }

    byteArray.resize(profiles.ByteSizeLong());
    if(!profiles.SerializeToArray(byteArray.data(), byteArray.size()))
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Serialize of data message error !");
    }

    return byteArray;
}

QByteArray DataProviderProfiles::deserializeAndSetData(const QByteArray& data)
{
    legion::messages::ProfileCommand command;

    LOG_T(__PRETTY_FUNCTION__);

    if(!command.ParseFromArray(data.data(), data.size()))
    {
        THROW_EXCEPTION(exception_T, DataProvider::ERROR_CODES::INVALID_DATA, "Parse of data message error !");
    }

    const QString name = QString::fromStdString(std::string(command.name()));

    switch(command.action())
    {
    case legion::messages::ProfileCommand::SAVE:
        DaemonSettingsManager::getInstance().saveProfile(m_dataProviderManager,name,QString::fromStdString(std::string(command.description())));
        break;
    case legion::messages::ProfileCommand::LOAD:
        DaemonSettingsManager::getInstance().loadProfile(m_dataProviderManager,name);
        break;
    case legion::messages::ProfileCommand::DELETE:
        DaemonSettingsManager::getInstance().deleteProfile(name);
        break;
    default:
        THROW_EXCEPTION(exception_T, DataProvider::ERROR_CODES::INVALID_DATA, QString("Unknown profile command ").append(QString::number(command.action())).append(" !").toStdString().c_str());
    }

    return serializeAndGetData();
}

void DataProviderProfiles::init()
{}

void DataProviderProfiles::clean()
{}

}
