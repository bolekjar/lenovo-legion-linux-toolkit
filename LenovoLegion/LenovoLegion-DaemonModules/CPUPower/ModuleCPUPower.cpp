// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUPower.h"
#include "RaplPowerLimits.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverIntelPowercapRapl.h>
#include <Drivers/SysFsDriverLegionEvents.h>
#include <Drivers/SysFsDriverLegionGameZone.h>
#include <Drivers/SysFsDriverLegionOther.h>

#include <KitLog.h>

#include <QScopeGuard>

#include <CpuPower.pb.h>

#include <set>

namespace LenovoLegionDaemon::Kit {

/*
 * CPU power limits (Legion WMI other). Owns notification POWER_CONTROL_CHANGED
 * for the attributes it writes (CPU limits, APU PPT, GPU total on AC).
 * Writes the CPU power limits of the thermal mode to the RAPL of the kernel (RaplPowerLimits).
 */
class ModuleCPUPower : public ProviderModule
{
public:

    explicit ModuleCPUPower(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionOther>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());
        drivers().addDriver(std::make_unique<SysFsDriverIntelPowercapRapl>());

        setProvider(std::make_unique<SysFsDataProviderCPUPower>(&drivers()));
    }

    void init() override
    {
        ProviderModule::init();

        applyRaplPowerLimits();
    }

    void onKernelModule(const QString& name,bool loaded) override
    {
        if(loaded && name == SysFsDriverLegionOther::MODULE_NAME)
        {
            applyRaplPowerLimits();
        }
    }

    /*
     * Limits set by a client or restored from the settings: in the custom mode the RAPL limits follow them at once
     * (also when a part of the request failed, the other limits may have been written)
     */
    QByteArray set(const QByteArray& request) override
    {
        const auto apply = qScopeGuard([this] { applyRaplPowerLimits(-1,true); });

        return ProviderModule::set(request);
    }

    void loadSettings(const Module::ISettingsStorage& storage) override
    {
        const auto apply = qScopeGuard([this] { applyRaplPowerLimits(-1,true); });

        ProviderModule::loadSettings(storage);
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 40, .customOnly = true };
    }

protected:

    /*
     * Thermal mode changed (Fn+Q or set by a client): RAPL limits of the new mode
     */
    void onDriverEvent(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverLegionEvents::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
           eventType(event) == SysFsDriverLegionEvents::LegionVmiEventType::LEGION_WMI_EVENT_THERMAL_MODE)
        {
            bool      ok   = false;
            const int mode = event.m_DriverSpecificEventValue.toInt(&ok);

            if(ok)
            {
                applyRaplPowerLimits(mode);
            }
        }
    }

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        static const std::set<int> OWNED {
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_STP, SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_LTP,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_PP,  SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_TMP,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_CLP, SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_PL1_TAU,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_APU_PPT, SysFsDriverLegionOther::LENOVO_WMI_OTHER_GPU_TAC
        };

        if(event.m_driverName == SysFsDriverLegionOther::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED && OWNED.contains(eventType(event)))
        {
            return notification(legion::messages::Notification::POWER_CONTROL_CHANGED);
        }

        return {};
    }

    /*
     * Thermal mode -1: the current one (smart fan mode of the Legion WMI game zone).
     * customOnly: only in the custom mode (the limits of the other modes do not depend on the set values).
     */
    void applyRaplPowerLimits(int thermalMode = -1,bool customOnly = false)
    {
        try {
            if(!drivers().isLoaded(SysFsDriverLegionOther::DRIVER_NAME))
            {
                return;
            }

            if(thermalMode < 0)
            {
                if(!drivers().isLoaded(SysFsDriverLegionGameZone::DRIVER_NAME))
                {
                    return;
                }

                const SysFsDriverLegionGameZone::GameZone::SmartFan smartFan(drivers().getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));

                thermalMode = SysFsDataProvider::getData(smartFan.m_current_value).toInt();
            }

            if(customOnly && thermalMode != RaplPowerLimits::THERMAL_MODE_CUSTOM)
            {
                return;
            }

            m_raplPowerLimits.apply(thermalMode,parse<legion::messages::CPUPower>(provider().serializeAndGetData()));
        }
        catch(const std::exception& ex)
        {
            LOG_W(QString("RAPL power limits of the thermal mode not written: ").append(ex.what()));
        }
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUPower cpuPower;

        if (storage.contains("cpu_stp_limit"))            cpuPower.mutable_cpu_stp_limit()->set_current_value(storage.value("cpu_stp_limit").toUInt());
        if (storage.contains("cpu_ltp_limit"))            cpuPower.mutable_cpu_ltp_limit()->set_current_value(storage.value("cpu_ltp_limit").toUInt());
        if (storage.contains("cpu_clp_limit"))            cpuPower.mutable_cpu_clp_limit()->set_current_value(storage.value("cpu_clp_limit").toUInt());
        if (storage.contains("cpu_tmp_limit"))            cpuPower.mutable_cpu_tmp_limit()->set_current_value(storage.value("cpu_tmp_limit").toUInt());
        if (storage.contains("cpu_pl1_tau"))              cpuPower.mutable_cpu_pl1_tau()->set_current_value(storage.value("cpu_pl1_tau").toUInt());
        if (storage.contains("gpu_total_onac"))           cpuPower.mutable_gpu_total_onac()->set_current_value(storage.value("gpu_total_onac").toUInt());
        if (storage.contains("gpu_to_cpu_dynamic_boost")) cpuPower.mutable_gpu_to_cpu_dynamic_boost()->set_current_value(storage.value("gpu_to_cpu_dynamic_boost").toUInt());

        return serialize(cpuPower);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto cpuPower = parse<legion::messages::CPUPower>(message);

        if (cpuPower.has_cpu_stp_limit())            storage.setValue("cpu_stp_limit", cpuPower.cpu_stp_limit().current_value());
        if (cpuPower.has_cpu_ltp_limit())            storage.setValue("cpu_ltp_limit", cpuPower.cpu_ltp_limit().current_value());
        if (cpuPower.has_cpu_clp_limit())            storage.setValue("cpu_clp_limit", cpuPower.cpu_clp_limit().current_value());
        if (cpuPower.has_cpu_tmp_limit())            storage.setValue("cpu_tmp_limit", cpuPower.cpu_tmp_limit().current_value());
        if (cpuPower.has_cpu_pl1_tau())              storage.setValue("cpu_pl1_tau", cpuPower.cpu_pl1_tau().current_value());
        if (cpuPower.has_gpu_total_onac())           storage.setValue("gpu_total_onac", cpuPower.gpu_total_onac().current_value());
        if (cpuPower.has_gpu_to_cpu_dynamic_boost()) storage.setValue("gpu_to_cpu_dynamic_boost", cpuPower.gpu_to_cpu_dynamic_boost().current_value());
    }

private:

    RaplPowerLimits m_raplPowerLimits { drivers() };
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUPower,"CPUPower","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUPower::dataType,"CPUPowerData")
