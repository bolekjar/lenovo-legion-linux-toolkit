// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderOther.h"
#include <Drivers/SysFsDriverLegionGameZone.h>
#include <Drivers/SysFsDriverLegionEvents.h>
#include <Drivers/SysFsDriverLegionVpc.h>


#include <Other.pb.h>

#include <optional>


#include <KitLog.h>


namespace LenovoLegionDaemon::Kit {

SysFsDataProviderOther::SysFsDataProviderOther(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

std::optional<SysFsDriverLegionVpc::Vpc> SysFsDataProviderOther::vpc() const
{
    /*
     * Optional: the ACPI device VPC2004 is not on every model, the other settings work without it
     */
    if(!m_sysFsDriverManager->isLoaded(SysFsDriverLegionVpc::DRIVER_NAME))
    {
        return std::nullopt;
    }

    return SysFsDriverLegionVpc::Vpc(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionVpc::DRIVER_NAME));
}

/*
 * Fields of the VPC features in the message, by their names in the driver
 */
static const std::vector<std::pair<legion::messages::OtherSettings::EnableDisableValue* (legion::messages::OtherSettings::*)(),
                                   const SysFsDriverLegionVpc::Vpc::Feature SysFsDriverLegionVpc::Vpc::*>> VPC_FIELDS = {
    { &legion::messages::OtherSettings::mutable_conservation_mode, &SysFsDriverLegionVpc::Vpc::m_conservationMode },
    { &legion::messages::OtherSettings::mutable_usb_charging,      &SysFsDriverLegionVpc::Vpc::m_usbCharging      },
    { &legion::messages::OtherSettings::mutable_fn_lock,           &SysFsDriverLegionVpc::Vpc::m_fnLock           },
    { &legion::messages::OtherSettings::mutable_camera_power,      &SysFsDriverLegionVpc::Vpc::m_cameraPower      },
    { &legion::messages::OtherSettings::mutable_rapid_charge,      &SysFsDriverLegionVpc::Vpc::m_rapidCharge      },
};


QByteArray SysFsDataProviderOther::serializeAndGetData()
{
    legion::messages::OtherSettings     otherSettingsMsg;
    QByteArray                          byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFsDriverLegionGameZone::GameZone gameZone(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));

        // DisableTouchPad (disable_tp)
        // Note: disable_tp current_value = 1 means touchpad is DISABLED
        otherSettingsMsg.mutable_touch_pad()->set_current(getData(gameZone.m_disableTP.m_current_value).toUShort() == 1);
        otherSettingsMsg.mutable_touch_pad()->set_supported(getData(gameZone.m_disableTP.m_supported).toUShort() == 1);

        // DisableWinKey (disable_win_key)
        // Note: disable_win_key current_value = 1 means win key is DISABLED
        otherSettingsMsg.mutable_win_key()->set_current(getData(gameZone.m_disableWinKey.m_current_value).toUShort() == 1);
        otherSettingsMsg.mutable_win_key()->set_supported(getData(gameZone.m_disableWinKey.m_supported).toUShort() == 1);

        // ACPI device VPC2004: an unsupported feature has no current value (the read fails)
        if(const auto features = vpc())
        {
            for(const auto& [field,member] : VPC_FIELDS)
            {
                const auto& feature = (*features).*member;

                if(feature.exists())
                {
                    auto* value = (otherSettingsMsg.*field)();

                    value->set_supported(getData(feature.m_supported).toUShort() == 1);

                    if(value->supported())
                    {
                        value->set_current(getData(feature.m_current_value).toUShort() == 1);
                    }
                }
            }
        }

    } catch(SysFsDriver::exception_T& ex)
    {
        /*
         * Lenovo Legion kernel driver not loaded: an error, not an empty message. The GUI shows the data type as
         * not available (pages grayed, Help system check) and no empty settings are saved over the saved ones.
         */
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
        }

        throw;
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
    SysFsDriverLegionGameZone::GameZone gameZone(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));
    legion::messages::OtherSettings     otherSettingsMsg;

    LOG_T(__PRETTY_FUNCTION__);

    if(!otherSettingsMsg.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    settable(otherSettingsMsg,Origin::REQUEST);

    m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionEvents::DRIVER_NAME,true);
    auto cleanup =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionEvents::DRIVER_NAME,false); });

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

    // ACPI device VPC2004 (settable() removed the fields without the driver or support), own uevents blocked
    if(const auto features = vpc())
    {
        m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionVpc::DRIVER_NAME,true);
        auto cleanupVpc =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionVpc::DRIVER_NAME,false); });

        for(const auto& [field,member] : VPC_FIELDS)
        {
            const auto* value = (otherSettingsMsg.*field)();

            if(value->has_current())
            {
                setData(((*features).*member).m_current_value, value->current());
                LOG_D(QString("Setting VPC feature ").append(((*features).*member).m_current_value.parent_path().filename().c_str()).append(" to: ").append(QString::number(value->current())));
            }
        }

        m_sysFsDriverManager->processAllUdevEvents(100);
    }

    m_sysFsDriverManager->processAllUdevEvents(100);

    return {};
}

QByteArray SysFsDataProviderOther::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::OtherSettings>(data,[this,origin](legion::messages::OtherSettings& settings) { settable(settings,origin); });
}

void SysFsDataProviderOther::settable(legion::messages::OtherSettings &settings, Origin origin) const
{
    auto check = [&](bool has,legion::messages::OtherSettings::EnableDisableValue* value,const std::filesystem::path& supported,const char* field) {
        if(!has)
        {
            return true;
        }

        value->clear_supported();

        if(!value->has_current())
        {
            return false;
        }

        if(getData(supported).toUShort() != 1)
        {
            skipField(origin,QString(field).append('=').append(QString::number(value->current())),"not supported");
            return false;
        }

        return true;
    };

    if(settings.has_touch_pad() || settings.has_win_key())
    {
        SysFsDriverLegionGameZone::GameZone gameZone(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));

        if(!check(settings.has_touch_pad(),settings.mutable_touch_pad(),gameZone.m_disableTP.m_supported,"disable touchpad"))
        {
            settings.clear_touch_pad();
        }

        if(!check(settings.has_win_key(),settings.mutable_win_key(),gameZone.m_disableWinKey.m_supported,"disable win key"))
        {
            settings.clear_win_key();
        }
    }

    /*
     * ACPI device VPC2004: without the driver or the feature the field is skipped
     */
    if(!settings.has_conservation_mode() && !settings.has_usb_charging() && !settings.has_fn_lock() && !settings.has_camera_power() && !settings.has_rapid_charge())
    {
        return;
    }

    const auto features = vpc();

    auto checkVpc = [&](bool has,legion::messages::OtherSettings::EnableDisableValue* value,const SysFsDriverLegionVpc::Vpc::Feature SysFsDriverLegionVpc::Vpc::* member,const char* field) {
        if(!has)
        {
            return true;
        }

        if(!features.has_value() || !((*features).*member).exists())
        {
            value->clear_supported();

            if(value->has_current())
            {
                skipField(origin,QString(field).append('=').append(QString::number(value->current())),"no VPC driver");
            }

            return false;
        }

        return check(true,value,((*features).*member).m_supported,field);
    };

    if(!checkVpc(settings.has_conservation_mode(),settings.mutable_conservation_mode(),&SysFsDriverLegionVpc::Vpc::m_conservationMode,"conservation mode"))
    {
        settings.clear_conservation_mode();
    }

    if(!checkVpc(settings.has_usb_charging(),settings.mutable_usb_charging(),&SysFsDriverLegionVpc::Vpc::m_usbCharging,"USB charging"))
    {
        settings.clear_usb_charging();
    }

    if(!checkVpc(settings.has_fn_lock(),settings.mutable_fn_lock(),&SysFsDriverLegionVpc::Vpc::m_fnLock,"Fn lock"))
    {
        settings.clear_fn_lock();
    }

    if(!checkVpc(settings.has_camera_power(),settings.mutable_camera_power(),&SysFsDriverLegionVpc::Vpc::m_cameraPower,"camera power"))
    {
        settings.clear_camera_power();
    }

    if(!checkVpc(settings.has_rapid_charge(),settings.mutable_rapid_charge(),&SysFsDriverLegionVpc::Vpc::m_rapidCharge,"rapid charge"))
    {
        settings.clear_rapid_charge();
    }
}

}
