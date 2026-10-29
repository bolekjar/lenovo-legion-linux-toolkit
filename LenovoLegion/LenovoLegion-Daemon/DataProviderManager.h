// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "DataProvider.h"
#include <Core/ExceptionBuilder.h>

#include <QObject>

#include <map>

namespace  LenovoLegionDaemon {

class DataProviderManager : public QObject
{

    Q_OBJECT

public:

    DEFINE_EXCEPTION(SysFsData);


    enum ERROR_CODES : int {
        DATA_PROVIDER_NOT_FOUND              = 1,
        DATA_PROVIDER_ALREADY_LOADED         = 2
    };

public:

    explicit DataProviderManager(QObject* parent);

    void addDataProvider(DataProvider* driver);

    void initDataProviders();

    void cleanDataProviders();

    DataProvider& getDataProvider(const quint8 dataType);

    void forEachDataProviderDo(const std::function<void(DataProvider&)>& func) const;

signals:

    void dataProviderEvent(const LenovoLegionDaemon::DataProvider::SubsystemEvent& event);

    /*
     * Ready notification payload of a data provider (daemon modules)
     */
    void providerNotification(const QByteArray& payload);

public slots:

    void dataProviderEventHandler(const LenovoLegionDaemon::DataProvider::SubsystemEvent& event);

private:

    std::map<quint8,DataProvider*>       m_dataProviders;
};

};
