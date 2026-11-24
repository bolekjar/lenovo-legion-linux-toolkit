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

#include <QMap>
#include <QStringList>

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


std::optional<SysFsDriverLegionOther::Other> SysFsDataProviderOther::other() const
{
    if(!m_sysFsDriverManager->isLoaded(SysFsDriverLegionOther::DRIVER_NAME))
    {
        return std::nullopt;
    }

    return SysFsDriverLegionOther::Other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));
}

/*
 * Fields of the features of the other mode in the message
 */
static const std::vector<std::pair<legion::messages::OtherSettings::EnableDisableValue* (legion::messages::OtherSettings::*)(),
                                   const SysFsDriverLegionOther::Other::StandardAttributesCapData00 SysFsDriverLegionOther::Other::*>> OTHER_FIELDS = {
    { &legion::messages::OtherSettings::mutable_instant_boot_ac,     &SysFsDriverLegionOther::Other::m_instant_boot_ac                 },
    { &legion::messages::OtherSettings::mutable_instant_boot_usb_pd, &SysFsDriverLegionOther::Other::m_instant_boot_usb_power_delivery },
    { &legion::messages::OtherSettings::mutable_flip_to_start,       &SysFsDriverLegionOther::Other::m_flip_to_start                   },
    { &legion::messages::OtherSettings::mutable_over_drive,          &SysFsDriverLegionOther::Other::m_over_drive                      },
};

/*
 * A feature of the other mode is offered when the firmware marks it supported and writable (capability bits
 * valid = 1, set = 4 of the capability data 00, e.g. 7)
 */
static bool otherFeatureWritable(const QString& supported)
{
    const uint capability = supported.toUInt();

    return (capability & 0x1) && (capability & 0x4);
}

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

        // Legion WMI other mode: instant boot, flip to start, overdrive
        if(const auto features = other())
        {
            for(const auto& [field,member] : OTHER_FIELDS)
            {
                const auto& feature = (*features).*member;

                if(!feature.m_supported.empty())
                {
                    auto* value = (otherSettingsMsg.*field)();

                    value->set_supported(otherFeatureWritable(getData(feature.m_supported)));

                    if(value->supported())
                    {
                        value->set_current(getData(feature.m_current_value).toUShort() == 1);
                    }
                }
            }
        }

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

    // Legion WMI other mode (settable() removed the fields without the driver or support), own uevents blocked
    if(const auto features = other())
    {
        m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionOther::DRIVER_NAME,true);
        auto cleanupOther =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionOther::DRIVER_NAME,false); });

        for(const auto& [field,member] : OTHER_FIELDS)
        {
            const auto* value = (otherSettingsMsg.*field)();

            if(value->has_current())
            {
                const auto& feature = (*features).*member;

                setData(feature.m_current_value, value->current());
                LOG_D(QString("Setting other mode feature ").append(feature.m_current_value.parent_path().filename().c_str())
                      .append(" to: ").append(QString::number(value->current())));
            }
        }

        m_sysFsDriverManager->processAllUdevEvents(100);
    }

    // ACPI device VPC2004 (settable() removed the fields without the driver or support), own uevents blocked
    if(const auto features = vpc())
    {
        m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionVpc::DRIVER_NAME,true);
        auto cleanupVpc =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionVpc::DRIVER_NAME,false); });

        /*
         * Labels of the features for the client (the same as on the Device page)
         */
        static const QMap<QString,QString> LABELS = {
            { "conservation_mode", "Battery conservation mode" },
            { "rapid_charge",      "Rapid charge"              },
            { "usb_charging",      "USB charging when off"     },
            { "fn_lock",           "Fn lock"                   },
            { "camera_power",      "Camera power"              },
        };

        QStringList ignored;

        for(const auto& [field,member] : VPC_FIELDS)
        {
            const auto* value = (otherSettingsMsg.*field)();

            if(value->has_current())
            {
                const auto&   feature = (*features).*member;
                const QString name    = feature.m_current_value.parent_path().filename().c_str();

                setData(feature.m_current_value, value->current());
                LOG_D(QString("Setting VPC feature ").append(name).append(" to: ").append(QString::number(value->current())));

                /*
                 * The kernel verifies the write: a write ignored by the firmware (e.g. camera power reported by the
                 * firmware but not controlled by it) marks the feature as not supported
                 */
                if(getData(feature.m_supported).toUShort() != 1)
                {
                    ignored.append(LABELS.value(name,name));
                }
            }
        }

        m_sysFsDriverManager->processAllUdevEvents(100);

        /*
         * The client gets the error (notification in the GUI), it reads the data again and does not show the option
         */
        if(!ignored.isEmpty())
        {
            THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,
                            ignored.join(", ").append(ignored.size() == 1 ? " is" : " are")
                            .append(" not supported by the firmware of this notebook (the setting was ignored), the option is removed").toStdString());
        }
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
     * Legion WMI other mode: without the driver or a writable feature the field is skipped
     */
    if(settings.has_instant_boot_ac() || settings.has_instant_boot_usb_pd() || settings.has_flip_to_start() || settings.has_over_drive())
    {
        const auto features = other();

        auto checkOther = [&](legion::messages::OtherSettings::EnableDisableValue* value,
                              const SysFsDriverLegionOther::Other::StandardAttributesCapData00 SysFsDriverLegionOther::Other::* member,
                              const char* field) {
            value->clear_supported();

            if(!value->has_current())
            {
                return false;
            }

            if(!features.has_value() || ((*features).*member).m_supported.empty() ||
               !otherFeatureWritable(getData(((*features).*member).m_supported)))
            {
                skipField(origin,QString(field).append('=').append(QString::number(value->current())),"not supported");
                return false;
            }

            return true;
        };

        if(settings.has_instant_boot_ac() &&
           !checkOther(settings.mutable_instant_boot_ac(),&SysFsDriverLegionOther::Other::m_instant_boot_ac,"instant boot (charger)"))
        {
            settings.clear_instant_boot_ac();
        }

        if(settings.has_instant_boot_usb_pd() &&
           !checkOther(settings.mutable_instant_boot_usb_pd(),&SysFsDriverLegionOther::Other::m_instant_boot_usb_power_delivery,"instant boot (USB-C)"))
        {
            settings.clear_instant_boot_usb_pd();
        }

        if(settings.has_flip_to_start() &&
           !checkOther(settings.mutable_flip_to_start(),&SysFsDriverLegionOther::Other::m_flip_to_start,"flip to start"))
        {
            settings.clear_flip_to_start();
        }

        if(settings.has_over_drive() &&
           !checkOther(settings.mutable_over_drive(),&SysFsDriverLegionOther::Other::m_over_drive,"display overdrive"))
        {
            settings.clear_over_drive();
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
