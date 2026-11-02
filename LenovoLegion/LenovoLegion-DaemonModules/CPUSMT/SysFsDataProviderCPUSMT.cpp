// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUSMT.h"
#include <Drivers/SysFsDriverCPU.h>
#include <Drivers/SysFsDriverCPUXList.h>

#include <CPUOptions.pb.h>


#include <KitLog.h>
#include <QCoreApplication>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderCPUSMT::SysFsDataProviderCPUSMT(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}



QByteArray SysFsDataProviderCPUSMT::serializeAndGetData()
{
    legion::messages::CPUSMT       cpuSmt;
    QByteArray                     byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverCPU::CPU::Smt smtControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPU::DRIVER_NAME));

        cpuSmt.set_control(getData(smtControl.m_control.value()).toStdString());
        cpuSmt.set_active(getData(smtControl.m_active.value()).toUShort() == 1);

        for(const auto& control : settableControls(QString::fromStdString(std::string(cpuSmt.control()))))
        {
            cpuSmt.add_settable_controls(control.toStdString());
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuSmt.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(cpuSmt.ByteSizeLong());
    if(!cpuSmt.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUSMT::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverCPU::CPU::Smt smtControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPU::DRIVER_NAME));
    legion::messages::CPUSMT cpuSmt;

    m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUXList::DRIVER_NAME,true);
    auto cleanup =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUXList::DRIVER_NAME,false); });
    LOG_T(__PRETTY_FUNCTION__);

    if(!cpuSmt.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    settable(cpuSmt,Origin::REQUEST);

    if(cpuSmt.has_control()) {
        /*
         * "on" means SMT active. Writing "on" when the control is already "on" does nothing, so sibling threads
         * switched off one by one (CPUOptions) would stay off: switch off and on, kernel switches all siblings on.
         */
        if(cpuSmt.control() == "on" && getData(smtControl.m_control.value()) == "on" && getData(smtControl.m_active.value()).toUShort() == 0)
        {
            LOG_D("SMT control is on but no sibling thread is active, switching SMT off and on");
            setData(smtControl.m_control.value(),std::string_view("off"));
        }

        setData(smtControl.m_control.value(),cpuSmt.control());
    }

    m_sysFsDriverManager->processAllUdevEvents(100);

    m_sysFsDriverManager->refreshDriver(SysFsDriverCPUXList::DRIVER_NAME);
    return {};
}

QByteArray SysFsDataProviderCPUSMT::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::CPUSMT>(data,[this,origin](legion::messages::CPUSMT& smt) { settable(smt,origin); });
}

QStringList SysFsDataProviderCPUSMT::settableControls(const QString &current)
{
    /*
     * Kernel control values: on, off, forceoff, notsupported, notimplemented.
     * Only on/off can be set and only when the current state is on/off
     * (forceoff can not be reverted until reboot, it is never set by the daemon).
     */
    const QStringList switchable {"on","off"};

    return switchable.contains(current) ? switchable : QStringList();
}

void SysFsDataProviderCPUSMT::settable(legion::messages::CPUSMT &smt, Origin origin) const
{
    /*
     * Current state "on" without any active sibling thread (all siblings switched off by CPUOptions) is the real state "off".
     * Saved "on" would switch the siblings on again after CPUOptions is applied (see deserializeAndSetData).
     */
    const bool inactive = origin == Origin::STATE && smt.has_active() && !smt.active();

    smt.clear_active();
    smt.clear_settable_controls();

    if(!smt.has_control())
    {
        return;
    }

    const QString requested = QString::fromStdString(std::string(smt.control())).trimmed();
    const QString current   = getData(SysFsDriverCPU::CPU::Smt(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPU::DRIVER_NAME)).m_control.value());

    const QStringList settable = settableControls(current);

    if(settable.isEmpty())
    {
        skipField(origin,QString("SMT control=").append(requested),QString("current state is ").append(current));
        smt.clear_control();
    }
    else if(!settable.contains(requested))
    {
        skipField(origin,QString("SMT control=").append(requested),QString("settable: ").append(settable.join(',')));
        smt.clear_control();
    }
    else
    {
        smt.set_control((inactive && requested == "on") ? std::string("off") : requested.toStdString());
    }
}


}
