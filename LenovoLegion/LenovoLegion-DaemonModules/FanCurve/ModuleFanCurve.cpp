// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderFanCurve.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionFanMode.h>

#include <FanControl.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Fan curve (Legion WMI fan mode). Owns notification FAN_CURVE_CHANGED for events of the fan mode driver.
 */
class ModuleFanCurve : public ProviderModule
{
public:

    explicit ModuleFanCurve(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionFanMode>());

        setProvider(std::make_unique<SysFsDataProviderFanCurve>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 30, .customOnly = true };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverLegionFanMode::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED)
        {
            return notification(legion::messages::Notification::FAN_CURVE_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::FanCurve fanCurve;

        /*
         * Only saved points (nothing saved = empty message = nothing is applied)
         */
        if (storage.contains("point1"))  fanCurve.mutable_current_value()->set_point1(storage.value("point1").toInt());
        if (storage.contains("point2"))  fanCurve.mutable_current_value()->set_point2(storage.value("point2").toInt());
        if (storage.contains("point3"))  fanCurve.mutable_current_value()->set_point3(storage.value("point3").toInt());
        if (storage.contains("point4"))  fanCurve.mutable_current_value()->set_point4(storage.value("point4").toInt());
        if (storage.contains("point5"))  fanCurve.mutable_current_value()->set_point5(storage.value("point5").toInt());
        if (storage.contains("point6"))  fanCurve.mutable_current_value()->set_point6(storage.value("point6").toInt());
        if (storage.contains("point7"))  fanCurve.mutable_current_value()->set_point7(storage.value("point7").toInt());
        if (storage.contains("point8"))  fanCurve.mutable_current_value()->set_point8(storage.value("point8").toInt());
        if (storage.contains("point9"))  fanCurve.mutable_current_value()->set_point9(storage.value("point9").toInt());
        if (storage.contains("point10")) fanCurve.mutable_current_value()->set_point10(storage.value("point10").toInt());

        return serialize(fanCurve);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto  fanCurve = parse<legion::messages::FanCurve>(message);
        const auto& points   = fanCurve.current_value();

        storage.setValue("point1",  points.point1());
        storage.setValue("point2",  points.point2());
        storage.setValue("point3",  points.point3());
        storage.setValue("point4",  points.point4());
        storage.setValue("point5",  points.point5());
        storage.setValue("point6",  points.point6());
        storage.setValue("point7",  points.point7());
        storage.setValue("point8",  points.point8());
        storage.setValue("point9",  points.point9());
        storage.setValue("point10", points.point10());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleFanCurve,"FanCurve","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderFanCurve::dataType,"FanCurveData")
