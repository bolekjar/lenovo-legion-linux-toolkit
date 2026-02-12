// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderOther.h"
#include "SysFSDriverLegionGameZone.h"


#include "../LenovoLegion-PrepareBuild/Other.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderOther::SysFsDataProviderOther(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}


QByteArray SysFsDataProviderOther::serializeAndGetData() const
{
    legion::messages::OtherSettings     otherSettingsMsg;
    QByteArray                          byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFSDriverLegionGameZone::GameZone gameZone(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));

        // DisableTouchPad (disable_tp)
        // Note: disable_tp current_value = 1 means touchpad is DISABLED
        otherSettingsMsg.mutable_touch_pad()->set_current(getData(gameZone.m_disableTP.m_current_value).toUShort() == 1);
        otherSettingsMsg.mutable_touch_pad()->set_supported(getData(gameZone.m_disableTP.m_supported).toUShort() == 1);

        // DisableWinKey (disable_win_key)
        // Note: disable_win_key current_value = 1 means win key is DISABLED
        otherSettingsMsg.mutable_win_key()->set_current(getData(gameZone.m_disableWinKey.m_current_value).toUShort() == 1);
        otherSettingsMsg.mutable_win_key()->set_supported(getData(gameZone.m_disableWinKey.m_supported).toUShort() == 1);

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            otherSettingsMsg.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(otherSettingsMsg.ByteSizeLong());
    if(!otherSettingsMsg.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderOther::deserializeAndSetData(const QByteArray &data)
{
    SysFSDriverLegionGameZone::GameZone gameZone(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionGameZone::DRIVER_NAME));
    legion::messages::OtherSettings     otherSettingsMsg;

    LOG_T(__PRETTY_FUNCTION__);

    if(!otherSettingsMsg.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    // Set DisableTouchPad if provided
    if(otherSettingsMsg.has_touch_pad() && otherSettingsMsg.touch_pad().has_current()) {
        setData(gameZone.m_disableTP.m_current_value, otherSettingsMsg.touch_pad().current());
        LOG_D(QString("Setting disable touchpad to: ").append(QString::number(otherSettingsMsg.touch_pad().current())));
    }

    // Set DisableWinKey if provided
    if(otherSettingsMsg.has_win_key() && otherSettingsMsg.win_key().has_current()) {
        setData(gameZone.m_disableWinKey.m_current_value, otherSettingsMsg.win_key().current());
        LOG_D(QString("Setting disable win key to: ").append(QString::number(otherSettingsMsg.win_key().current())));
    }

    return {};
}

}
