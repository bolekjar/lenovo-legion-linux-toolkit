// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderRGBController.h"
#include "RGBController.h"

#include "SysFsDriverLegionEvents.h"

#include "Core/LoggerHolder.h"

#include "../LenovoLegion-PrepareBuild/RGBController.pb.h"
#include "../LenovoLegion-PrepareBuild/Notification.pb.h"

#include <hidapi.h>

namespace LenovoLegionDaemon {

    DataProviderRGBController::DataProviderRGBController(QObject* parent)  :DataProvider(parent,dataType) {}

    DataProviderRGBController::~DataProviderRGBController()
    {}

    QByteArray DataProviderRGBController::serializeAndGetData(const QByteArray& data) const
    {
        legion::messages::RGBControllerResponse rgbController;
        QByteArray byteArray;

        LOG_T(__PRETTY_FUNCTION__);

        if(m_rgbController == nullptr)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + " - RGB Controller not available");
            rgbController.Clear();
        }
        else
        {
            legion::messages::RGBControllerRequest request;

            if(!request.ParseFromArray(data.data(), data.size()))
            {
                THROW_EXCEPTION(exception_T, DataProvider::ERROR_CODES::INVALID_DATA, "Parse of data message error !");
            }

            const legion::messages::RGBControllerRequest::RequestFlags requestFlags = static_cast<legion::messages::RGBControllerRequest::RequestFlags>(request.request_flags());


            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_BRITNESS)
            {
                m_rgbController->DeviceRefreshBrightness();
            }

            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_PROFILE           ||
               requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_LED_GROUP_EFFECTS
              )
            {
                m_rgbController->DeviceRefresh();
            }

            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_LOGO_STATUS)
            {
                m_rgbController->DeviceRefreshLogoState();
            }

            // Set device type
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_DEVICE_TYPE)
            {
                rgbController.set_device_type(static_cast<legion::messages::RGBController::DeviceType>(m_rgbController->GetDeviceType()));
            }

            // Set profiles
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_PROFILE)
            {
                rgbController.mutable_profile()->set_current(m_rgbController->GetProfiles().active);
                rgbController.mutable_profile()->set_min(m_rgbController->GetProfiles().min);
                rgbController.mutable_profile()->set_max(m_rgbController->GetProfiles().max);
            }

            // Set brightness
            if(requestFlags &  legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_BRITNESS)
            {
                rgbController.mutable_britness()->set_current(m_rgbController->GetBrightness().active);
                rgbController.mutable_britness()->set_min(m_rgbController->GetBrightness().min);
                rgbController.mutable_britness()->set_max(m_rgbController->GetBrightness().max);
            }

            // Set max effects
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_MAX_EFFECTS)
            {
                rgbController.set_max_effects(m_rgbController->GetMaxEffects());
            }

            // Serialize Effects
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_LED_GROUP_EFFECTS)
            {
                const auto& effects = m_rgbController->GetEffects();
                for(const auto& effect : effects)
                {
                    auto* pbEffect = rgbController.add_led_group_effects();

                    pbEffect->set_mode(effect.m_mode);
                    pbEffect->set_direction(effect.m_direction);
                    pbEffect->set_speed(effect.m_speed);
                    pbEffect->set_color_mode(effect.m_color_mode);

                    for (const auto& color : effect.m_colors)
                    {
                        pbEffect->add_colors(color);
                    }

                    for (const auto& led : effect.m_leds)
                    {
                        auto* ledToAdd = pbEffect->add_leds();

                        ledToAdd->set_value(led.value);
                        ledToAdd->set_name(led.name);
                    }
                }
            }

            // Serialize LEDs
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_LEDS)
            {
                const auto& leds = m_rgbController->GetLEDs();
                for(const auto& led : leds)
                {
                    auto* pbLed = rgbController.add_leds();

                    pbLed->set_name(led.name);
                    pbLed->set_value(led.value);
                }
            }

            // Serialize Zones
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_ZONES)
            {
                const auto& zones = m_rgbController->GetZones();
                for(const auto& zone : zones)
                {
                    auto* pbZone = rgbController.add_zones();

                    pbZone->set_name(zone.name);
                    pbZone->set_type(static_cast<legion::messages::RGBController::ZoneType>(zone.type));
                    pbZone->set_leds_count(zone.leds_count);
                    pbZone->set_leds_min(zone.leds_min);
                    pbZone->set_leds_max(zone.leds_max);
                    pbZone->set_flags(zone.flags);

                    if(zone.matrix_map.map.size() > 0)
                    {
                        auto* pbMatrixMap = pbZone->mutable_matrix_map();
                        pbMatrixMap->set_height(zone.matrix_map.height);
                        pbMatrixMap->set_width(zone.matrix_map.width);

                        for(const auto& val : zone.matrix_map.map)
                        {
                            pbMatrixMap->add_map(val);
                        }
                    }
                    pbZone->set_start_idx(zone.start_idx);
                }
            }

            // Serialize Modes
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_MODES)
            {
                const auto& modes = m_rgbController->GetModes();
                for(const auto& mode : modes)
                {
                    auto* pbMode = rgbController.add_modes();

                    pbMode->set_name(mode.name);
                    pbMode->set_value(mode.value);
                    pbMode->set_flags(mode.flags);
                    pbMode->set_speed_min(mode.speed_min);
                    pbMode->set_speed_max(mode.speed_max);
                    pbMode->set_colors_min(mode.colors_min);
                    pbMode->set_colors_max(mode.colors_max);
                    pbMode->set_speed(mode.speed);
                    pbMode->set_direction(mode.direction);
                    pbMode->set_color_mode(mode.color_mode);

                    // Serialize mode colors
                    for(const auto& color : mode.colors)
                    {
                        pbMode->add_colors(color);
                    }
                }
            }

            // Serialize Colors For All leds States
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_STATE_FOR_ALL_LEDS)
            {
                auto colors = m_rgbController->DeviceGetState();
                for(const auto& color : colors)
                {
                    rgbController.add_colors(color);
                }
            }

            // Serialize info
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_DEVICE_INFO)
            {
                rgbController.mutable_device_info()->set_name(m_rgbController->GetName());
                rgbController.mutable_device_info()->set_vendor(m_rgbController->GetVendor());
                rgbController.mutable_device_info()->set_description(m_rgbController->GetDescription());
                rgbController.mutable_device_info()->set_serial(m_rgbController->GetSerial());
                rgbController.mutable_device_info()->set_location(m_rgbController->GetLocation());
                rgbController.mutable_device_info()->set_vendor_id(m_rgbController->GetVendorID());
                rgbController.mutable_device_info()->set_product_id(m_rgbController->GetProductID());
            }

            // Serialize logo state
            if(requestFlags & legion::messages::RGBControllerRequest::RequestFlags::RGBControllerRequest_RequestFlags_REQUEST_LOGO_STATUS)
            {
                rgbController.mutable_logo_status()->set_has_logo(m_rgbController->HasLogo());
                rgbController.mutable_logo_status()->set_logo_on(m_rgbController->GetLogoState());
            }
        }

        byteArray.resize(rgbController.ByteSizeLong());
        if(!rgbController.SerializeToArray(byteArray.data(), byteArray.size()))
        {
            THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Serialize of data message error !");
        }

        return byteArray;
    }

    QByteArray DataProviderRGBController::deserializeAndSetData(const QByteArray& data)
    {
        legion::messages::RGBControllerSetRequest rgbController;

        LOG_T(__PRETTY_FUNCTION__);

        if(!rgbController.ParseFromArray(data.data(), data.size()))
        {
            THROW_EXCEPTION(exception_T, DataProvider::ERROR_CODES::INVALID_DATA, "Parse of data message error !");
        }

        if(m_rgbController == nullptr)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + " - RGB Controller not available");
            return {};
        }

        // Apply profile if changed
        if(rgbController.set_request_flags() & legion::messages::RGBControllerSetRequest::SetRequestFlags::RGBControllerSetRequest_SetRequestFlags_SET_REQUEST_PROFILE)
        {
            auto currentProfile = m_rgbController->GetProfiles().active;
            if(currentProfile != rgbController.profile().current())
            {
                LOG_D(QString("Changing profile from %1 to %2").arg(currentProfile).arg(rgbController.profile().current()));

                m_rgbController->SetProfile(rgbController.profile().current());
                m_rgbController->DeviceUpdateProfile();
            }
        }

        // Apply brightness if changed
        if(rgbController.set_request_flags() & legion::messages::RGBControllerSetRequest::SetRequestFlags::RGBControllerSetRequest_SetRequestFlags_SET_REQUEST_BRITNESS)
        {
            auto currentBrightness = m_rgbController->GetBrightness().active;
            if(currentBrightness != rgbController.britness().current())
            {
                LOG_D(QString("Changing brightness from %1 to %2").arg(currentBrightness).arg(rgbController.britness().current()));

                m_rgbController->SetBrightness(rgbController.britness().current());
                m_rgbController->DeviceUpdateBrightness();
            }
        }

        /*
         * Apply effects if provided
         */
        if(rgbController.set_request_flags() & legion::messages::RGBControllerSetRequest::SetRequestFlags::RGBControllerSetRequest_SetRequestFlags_SET_REQUEST_LED_GROUP_EFFECTS)
        {
            LOG_D("Applying LED Group Effects");

            m_rgbController->ClearEffects();

            for(int i = 0; i < rgbController.led_group_effects_size(); i++)
            {
                LenovoLegionDaemon::led_group_effect effect;

                const auto& pbEffect = rgbController.led_group_effects(i);

                effect.m_mode        = pbEffect.mode();
                effect.m_direction   = pbEffect.direction();
                effect.m_speed       = pbEffect.speed();
                effect.m_color_mode  = pbEffect.color_mode();

                for (int j = 0; j < pbEffect.colors_size(); j++)
                {
                    effect.m_colors.push_back(pbEffect.colors(j));
                }

                for (int j = 0; j < pbEffect.leds_size(); j++)
                {
                    const auto& pbLed = pbEffect.leds(j);
                    LenovoLegionDaemon::led led;
                    led.name  = pbLed.name();
                    led.value = pbLed.value();
                    effect.m_leds.push_back(led);
                }

                m_rgbController->AddEffect(effect);
            }

            m_rgbController->DeviceUpdateEfects();
        }

        /*
         * Apply reset effects to default if requested
         */
        if(rgbController.set_request_flags() & legion::messages::RGBControllerSetRequest::SetRequestFlags::RGBControllerSetRequest_SetRequestFlags_SET_REQUEST_RESET_EFECTS_TTO_DEF)
        {
            m_rgbController->DeviceResetEffectsToDefault();
        }

        /*
         * Apply logo state if provided
         */
        if(rgbController.set_request_flags() & legion::messages::RGBControllerSetRequest::SetRequestFlags::RGBControllerSetRequest_SetRequestFlags_SET_REQUEST_LOGO_STATUS)
        {
            if(m_rgbController->HasLogo())
            {
                LOG_D("Changing logo state to " + QString(rgbController.logo_status().logo_on() ? "ON" : "OFF"));

                m_rgbController->SetLogoState(rgbController.logo_status().logo_on());
                m_rgbController->DeviceUpdateLogoState();
            }
        }

        return {};
    }

    void DataProviderRGBController::init()
    {
        LOG_T(__PRETTY_FUNCTION__);

        clean();

        /*-----------------------------------------------------------------------------*\
        | Loop through all available detectors.  If all required information matches,   |
        | run the detector                                                              |
        \*-----------------------------------------------------------------------------*/
        for (auto &detector : hidDeviceDetectorsBlocks)
        {
            hid_device_info * hid_devices = hid_enumerate(detector.m_vid, 0);

            auto cleanup = qScopeGuard([hid_devices] { if(hid_devices != nullptr) hid_free_enumeration(hid_devices); });

            for (hid_device_info * current_hid_device = hid_devices; current_hid_device != nullptr; current_hid_device = current_hid_device->next)
            {
                LOG_T(QString::asprintf("Checking HID Device: VID=%X, PID=%X, Interface=%d, Usage_Page=%X, Path=%s",
                                        current_hid_device->vendor_id,
                                        current_hid_device->product_id,
                                        current_hid_device->interface_number,
                                        current_hid_device->usage_page,
                                        current_hid_device->path));

                try {
                    if (detector.compare(*current_hid_device)) {

                        if(m_rgbController != nullptr)
                        {
                            LOG_W("RGB Controller already detected, skipping further detection attempts.");
                            break;
                        }

                        m_rgbController.reset(detector.m_function(*current_hid_device,detector.m_name));

                        LOG_T(QString("RGB Controller Detected: ").append(detector.m_name.c_str()));
                    }
                } catch (bj::framework::exception::Exception& ex)
                {
                    /*
                     * Problem is that hid_enumerate itself is returning the same device twice in its linked list. This is a known issue with hidapi on Linux when:
                     *   1. A device has been unplugged/replugged
                     *   2. There's a kernel/udev issue creating duplicate entries
                     *   3. The hidapi library has a bug
                     */
                    LOG_E(QString("Error during RGB Controller detection (").append(detector.m_name.c_str()).append("): ").append(ex.what()));
                    break;
                }
            }
        }
    }

    void DataProviderRGBController::clean()
    {
        m_rgbController.reset();
    }

    void DataProviderRGBController::kernelEventHandler(const SysFsDriver::SubsystemEvent &event)
    {
        LOG_T(__PRETTY_FUNCTION__);

        if(event.m_driverName == SysFsDriverLegionEvents::DRIVER_NAME)
        {
            if(event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED)
            {
                if(static_cast<SysFsDriverLegionEvents::LegionVmiEventType>(QString(event.m_DriverSpecificEventType.data()).toInt()) == SysFsDriverLegionEvents::LegionVmiEventType::LENOVO_WMI_EVENT_UTILITY)
                {
                    if(m_rgbController != nullptr)
                    {
                        if(static_cast<SysFsDriverLegionEvents::LegionVmiEventType>(QString(event.m_DriverSpecificEventType.data()).toInt()) == SysFsDriverLegionEvents::LegionVmiEventType::LENOVO_WMI_EVENT_UTILITY)
                        {
                            LOG_D("DataProviderRGBController: kernelEventHandler - LENOVO_WMI_EVENT_UTILITY received, refreshing RGB Controller data");

                            switch(static_cast<legion::messages::Notification_SpecialKey>(QString(event.m_DriverSpecificEventValue.data()).toUInt()))
                            {
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMBACKLIGHTOFF:
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMBACKLIGHT1:
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMBACKLIGHT2:
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMBACKLIGHT3:
                                    m_rgbController->DeviceRefreshBrightness();
                                    break;
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMPRESET1:
                                    m_rgbController->DeviceRefresh(1);
                                    break;
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMPRESET2:
                                    m_rgbController->DeviceRefresh(2);
                                    break;
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMPRESET3:
                                    m_rgbController->DeviceRefresh(3);
                                    break;
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMPRESET4:
                                    m_rgbController->DeviceRefresh(4);
                                    break;
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMPRESET5:
                                    m_rgbController->DeviceRefresh(5);
                                    break;
                                case legion::messages::Notification_SpecialKey::Notification_SpecialKey_SPECTRUMPRESET6:
                                    m_rgbController->DeviceRefresh(6);
                                    break;
                                default:
                                    break;
                            }

                        }
                    }
                }
            }
        }
    }

    void DataProviderRGBController::registerControler(std::string name, HIDDeviceDetectorFunction det, uint16_t vid, uint16_t pid, uint16_t pidMask)
    {
        hidDeviceDetectorsBlocks.push_back({
            .m_name         = name,
            .m_vid          = vid,
            .m_pid          = pid,
            .m_pidMask      = pidMask,
            .m_function     = det
        });
    }

    std::vector<DataProviderRGBController::HIDDeviceDetectorBlock> DataProviderRGBController::hidDeviceDetectorsBlocks{};


    bool DataProviderRGBController::HIDDeviceDetectorBlock::compare(const hid_device_info& info)
    {
        return ((m_vid == info.vendor_id) && (m_pid == (info.product_id & m_pidMask)));
    }

}

