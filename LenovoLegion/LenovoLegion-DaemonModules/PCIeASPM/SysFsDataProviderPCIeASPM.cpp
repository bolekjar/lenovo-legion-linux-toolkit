// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderPCIeASPM.h"

#include <Drivers/SysFsDriverPcieAspm.h>

#include <KitLog.h>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderPCIeASPM::SysFsDataProviderPCIeASPM(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

legion::messages::PcieAspm SysFsDataProviderPCIeASPM::read() const
{
    legion::messages::PcieAspm aspm;

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverPcieAspm::DRIVER_NAME))
    {
        return aspm;
    }

    const SysFsDriverPcieAspm::Aspm driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverPcieAspm::DRIVER_NAME));

    /*
     * "[default] performance powersave powersupersave": the current one in brackets
     */
    for(QString policy : getData(driver.m_policy).split(' ',Qt::SkipEmptyParts))
    {
        policy = policy.trimmed();

        if(policy.startsWith('[') && policy.endsWith(']'))
        {
            policy = policy.mid(1,policy.size() - 2);
            aspm.set_policy(policy.toStdString());
        }

        aspm.add_available_policies(policy.toStdString());
    }

    return aspm;
}

QByteArray SysFsDataProviderPCIeASPM::serializeAndGetData()
{
    LOG_T(__PRETTY_FUNCTION__);

    return serializeMessage(read());
}

QByteArray SysFsDataProviderPCIeASPM::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::PcieAspm aspm = parseMessage<legion::messages::PcieAspm>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(aspm,Origin::REQUEST);

    if(!aspm.has_policy())
    {
        return {};
    }

    const SysFsDriverPcieAspm::Aspm driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverPcieAspm::DRIVER_NAME));

    /*
     * The kernel refuses it when the BIOS does not allow ASPM (FADT) or pcie_aspm=off (the error is sent to the client)
     */
    setData(driver.m_policy,std::string_view(aspm.policy()));

    return {};
}

QByteArray SysFsDataProviderPCIeASPM::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::PcieAspm>(data,[this,origin](legion::messages::PcieAspm& aspm) { settable(aspm,origin); });
}

void SysFsDataProviderPCIeASPM::settable(legion::messages::PcieAspm &aspm, Origin origin) const
{
    aspm.clear_available_policies();

    if(!aspm.has_policy())
    {
        return;
    }

    const legion::messages::PcieAspm current = read();
    const QString                    policy  = QString::fromStdString(std::string(aspm.policy()));
    QStringList                      available;

    for(const auto& item : current.available_policies())
    {
        available.append(QString::fromStdString(std::string(item)));
    }

    if(!available.contains(policy))
    {
        skipField(origin,QString("PCIe ASPM policy=").append(policy),QString("available: ").append(available.join(',')));
        aspm.clear_policy();
    }
}

}
