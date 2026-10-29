// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ModuleIntelMSR.h"

#include <SysFs.h>

#include <CpuIntelMSR.pb.h>
#include <MessageRegistry.pb.h>
#include <Notification.pb.h>

#include <array>

namespace LenovoLegionDaemon::Modules::IntelMSR {

using namespace LenovoLegionDaemon::Module;
using legion::messages::CpuIntelMSR;

namespace {

/*
 * Device and kernel module
 */
constexpr const char* SYSFS_PATH        = "/sys/class/legion-intel-msr/intel-msr-0/";
constexpr const char* SUBSYSTEM         = "legion-intel-msr";
constexpr const char* DRIVER            = "legion-intel-msr";
constexpr const char* KERNEL_MODULE     = "lenovo_legion";

constexpr const char* MODULE_SUBSYSTEM  = "module";

/*
 * Voltage planes, in the order they are written (the same as before the module)
 */
struct Plane
{
    const char* m_name;                                                         // prefix of sysfs attributes and settings keys
    bool                                (CpuIntelMSR::*m_has)()      const;
    const CpuIntelMSR::VoltageSettings& (CpuIntelMSR::*m_settings)() const;
    CpuIntelMSR::VoltageSettings*       (CpuIntelMSR::*m_mutable)();
};

const std::array<Plane,5> PLANES {{
    { "analogio", &CpuIntelMSR::has_analogio, &CpuIntelMSR::analogio, &CpuIntelMSR::mutable_analogio },
    { "cache",    &CpuIntelMSR::has_cache,    &CpuIntelMSR::cache,    &CpuIntelMSR::mutable_cache    },
    { "cpu",      &CpuIntelMSR::has_cpu,      &CpuIntelMSR::cpu,      &CpuIntelMSR::mutable_cpu      },
    { "gpu",      &CpuIntelMSR::has_gpu,      &CpuIntelMSR::gpu,      &CpuIntelMSR::mutable_gpu      },
    { "uncore",   &CpuIntelMSR::has_uncore,   &CpuIntelMSR::uncore,   &CpuIntelMSR::mutable_uncore   },
}};

QString offsetKey(const Plane& plane)
{
    return QString(plane.m_name).append("_offset");
}

/*
 * Offset read back from MSR is not exact (1/1024 V units), e.g. -49804 uV for -50 mV.
 * Round away from zero to whole mV (value in uV) before saving.
 */
qint32 roundToMillivolts(qint32 offsetUv)
{
    return ((offsetUv > 0 ? offsetUv + 999 : offsetUv - 999) / 1000) * 1000;
}

template<class Message>
QByteArray serialize(const Message& message)
{
    QByteArray bytes;

    bytes.resize(static_cast<qsizetype>(message.ByteSizeLong()));

    if(!message.SerializeToArray(bytes.data(),static_cast<int>(bytes.size())))
    {
        throw Error("IntelMSR: serialize of data message error !");
    }

    return bytes;
}


class ModuleIntelMSR : public IModule
{
public:

    ModuleIntelMSR(IModuleHost& host,const std::filesystem::path& sysfsPath) :
        m_host(host),
        m_sysfsPath(sysfsPath)
    {}

    void init() override
    {
        m_host.addKernelEventFilter({ SUBSYSTEM,        { "EVENT_TYPE", "EVENT_VALUE" } });
        m_host.addKernelEventFilter({ MODULE_SUBSYSTEM, {} });

        detect();
    }

    void clean() override
    {
        m_available = false;
    }

    bool isAvailable() const override
    {
        return m_available;
    }

    QByteArray get(const QByteArray&) override
    {
        return serialize(readState());
    }

    QByteArray set(const QByteArray& request) override
    {
        if(request.isEmpty())
        {
            m_host.logger().debug("IntelMSR: empty set request, skipping");
            return {};
        }

        CpuIntelMSR message;

        if(!message.ParseFromArray(request.constData(),static_cast<int>(request.size())))
        {
            throw Error("IntelMSR: parse of data message error !");
        }

        apply(message);

        return {};
    }

    void onKernelEvent(const KernelEvent& event) override
    {
        if(event.m_subSystem == MODULE_SUBSYSTEM)
        {
            if(event.m_sysName == KERNEL_MODULE)
            {
                if(event.m_action == "add")
                {
                    detect();
                }

                if(event.m_action == "remove")
                {
                    m_host.logger().debug("IntelMSR: kernel module removed");
                    m_available = false;
                }
            }

            return;
        }

        if(event.m_subSystem == SUBSYSTEM && event.m_driver == DRIVER && event.m_action == "change")
        {
            legion::messages::Notification notification;

            notification.set_action(legion::messages::Notification::CPU_OFFSET_CHANGED);

            m_host.notify(serialize(notification));
        }
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 90, .customOnly = false };
    }

    void loadSettings(const ISettingsStorage& storage) override
    {
        CpuIntelMSR message;

        for(const auto& plane : PLANES)
        {
            if(storage.contains(offsetKey(plane)))
            {
                (message.*plane.m_mutable)()->set_offset(storage.value(offsetKey(plane)).toInt());
            }
        }

        if(message.ByteSizeLong() == 0)
        {
            m_host.logger().debug("IntelMSR: no saved settings, skipping");
            return;
        }

        apply(message);
    }

    void saveSettings(ISettingsStorage& storage) override
    {
        /*
         * Device not available -> nothing is saved, saved settings stay unchanged
         */
        const CpuIntelMSR message = readState();

        for(const auto& plane : PLANES)
        {
            if((message.*plane.m_has)() && (message.*plane.m_settings)().has_offset())
            {
                storage.setValue(offsetKey(plane),roundToMillivolts((message.*plane.m_settings)().offset()));
            }
        }
    }

private:

    std::filesystem::path attribute(const Plane& plane,const char* suffix) const
    {
        return m_sysfsPath / (std::string(plane.m_name) + suffix);
    }

    void detect()
    {
        m_available = SysFs::exists(m_sysfsPath);

        m_host.logger().debug(QString("IntelMSR: device ").append(QString::fromStdString(m_sysfsPath.string())).append(m_available ? " found" : " not found"));
    }

    /*
     * Current state, empty message if the device is not available
     */
    CpuIntelMSR readState() const
    {
        CpuIntelMSR message;

        if(!m_available)
        {
            m_host.logger().debug("IntelMSR: device not available");
            return message;
        }

        for(const auto& plane : PLANES)
        {
            CpuIntelMSR::VoltageSettings* settings = (message.*plane.m_mutable)();

            settings->set_max_overvolt(SysFs::read(attribute(plane,"_max_overvolt")).toUInt());
            settings->set_max_undervolt(SysFs::read(attribute(plane,"_max_undervolt")).toUInt());
            settings->set_offset(SysFs::read(attribute(plane,"_offset")).toInt());
            settings->set_supported(SysFs::read(attribute(plane,"_offset_ctrl_supported")).toUShort() == 1);
        }

        return message;
    }

    /*
     * Write offsets present in the message. Failed write is only logged, the client re-reads the values.
     */
    void apply(const CpuIntelMSR& message)
    {
        if(!m_available)
        {
            m_host.logger().error("IntelMSR: device not available, request is ignored !");
            return;
        }

        /*
         * Own kernel events are not delivered to the client
         */
        KernelEventBlocker blocker(m_host);

        for(const auto& plane : PLANES)
        {
            if(!(message.*plane.m_has)() || !(message.*plane.m_settings)().has_offset())
            {
                continue;
            }

            QString error;

            if(!SysFs::write(attribute(plane,"_offset"),(message.*plane.m_settings)().offset(),&error))
            {
                m_host.logger().error(QString("IntelMSR: unable to set ").append(plane.m_name).append(" offset ").append(QString::number((message.*plane.m_settings)().offset())).append(" : ").append(error));
            }
        }
    }

private:

    IModuleHost&                m_host;
    const std::filesystem::path m_sysfsPath;
    bool                        m_available = false;
};


const Info INFO {
    .abiVersion       = ABI_VERSION,
    .name             = "IntelMSR",
    .version          = "1.0",
    .legacyDataType   = legion::messages::DataType::CPU_INTEL_MSR,
    .settingsGroup    = "IntelMSRData",
    .requiredServices = nullptr
};

}


const Info* info()
{
    return &INFO;
}

IModule* create(IModuleHost& host)
{
    return create(host,SYSFS_PATH);
}

IModule* create(IModuleHost& host,const std::filesystem::path& sysfsPath)
{
    return new ModuleIntelMSR(host,sysfsPath);
}

void destroy(IModule* module)
{
    delete module;
}

}


/*
 * Exported entry points of the module (.so), everything else is hidden
 */
LEGION_MODULE_DECLARE_ENTRY_POINTS

const LenovoLegionDaemon::Module::Info* legion_module_info()
{
    return LenovoLegionDaemon::Modules::IntelMSR::info();
}

LenovoLegionDaemon::Module::IModule* legion_module_create(LenovoLegionDaemon::Module::IModuleHost& host)
{
    return LenovoLegionDaemon::Modules::IntelMSR::create(host);
}

void legion_module_destroy(LenovoLegionDaemon::Module::IModule* module)
{
    LenovoLegionDaemon::Modules::IntelMSR::destroy(module);
}
