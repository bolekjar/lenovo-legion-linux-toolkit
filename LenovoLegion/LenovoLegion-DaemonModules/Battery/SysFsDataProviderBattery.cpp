// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderBattery.h"
#include <Drivers/SysFsDriverPowerSuplyBattery0.h>
#include <Drivers/SysFsDriverLegionGameZone.h>


#include <Battery.pb.h>


#include <KitLog.h>

#include <optional>


namespace LenovoLegionDaemon::Kit {

SysFsDataProviderBattery::SysFsDataProviderBattery(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}



QByteArray SysFsDataProviderBattery::serializeAndGetData()
{
    legion::messages::Battery          battery;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverPowerSuplyBattery0::PowerSuplyBattery0 batery0(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverPowerSuplyBattery0::DRIVER_NAME));

        battery.set_baterry_status(getData(batery0.m_powerSuplyBattery0).toStdString());
        battery.set_supported(true);

        /*
         * Optional values (not every battery reports all of them, a value which can not be read is not set)
         */
        auto number = [](const std::filesystem::path& path) -> std::optional<qulonglong> {
            if(path.empty())
            {
                return {};
            }

            const std::optional<QString> text = getOptionalData(path);
            bool ok = false;
            const qlonglong value = text.has_value() ? text->toLongLong(&ok) : 0;

            return ok ? std::optional<qulonglong>(static_cast<qulonglong>(value < 0 ? -value : value)) : std::nullopt;
        };
        auto text = [](const std::filesystem::path& path) -> std::optional<QString> {
            return path.empty() ? std::nullopt : getOptionalData(path);
        };

        if(const auto value = number(batery0.m_capacity))     battery.set_capacity(static_cast<quint32>(*value));
        if(const auto value = number(batery0.m_voltageNow))   battery.set_voltage_now(static_cast<quint32>(*value));
        if(const auto value = number(batery0.m_cycleCount))   battery.set_cycle_count(static_cast<quint32>(*value));
        if(const auto value = text(batery0.m_capacityLevel))  battery.set_capacity_level(value->toStdString());
        if(const auto value = text(batery0.m_technology))     battery.set_technology(value->toStdString());
        if(const auto value = text(batery0.m_manufacturer))   battery.set_manufacturer(value->toStdString());
        if(const auto value = text(batery0.m_modelName))      battery.set_model_name(value->toStdString());
        if(const auto value = number(batery0.m_acOnline))     battery.set_ac_online(*value != 0);

        /*
         * Energy (uWh) and power (uW); batteries reporting the charge (uAh) and the current (uA): with the voltage
         */
        const std::optional<qulonglong> voltageDesign = number(batery0.m_voltageMinDesign);
        const std::optional<qulonglong> voltageNow    = number(batery0.m_voltageNow);
        auto energy = [&](const std::filesystem::path& energyPath,const std::filesystem::path& chargePath) -> std::optional<qulonglong> {
            if(const auto value = number(energyPath))
            {
                return value;
            }

            const auto charge = number(chargePath);
            return charge.has_value() && voltageDesign.has_value() ? std::optional<qulonglong>(*charge * *voltageDesign / 1000000) : std::nullopt;
        };

        if(const auto value = energy(batery0.m_energyNow,batery0.m_chargeNow))               battery.set_energy_now(*value);
        if(const auto value = energy(batery0.m_energyFull,batery0.m_chargeFull))             battery.set_energy_full(*value);
        if(const auto value = energy(batery0.m_energyFullDesign,batery0.m_chargeFullDesign)) battery.set_energy_full_design(*value);

        if(const auto value = number(batery0.m_powerNow))
        {
            battery.set_power_now(static_cast<quint32>(*value));
        }
        else if(const auto current = number(batery0.m_currentNow); current.has_value() && voltageNow.has_value())
        {
            battery.set_power_now(static_cast<quint32>(*current * *voltageNow / 1000000));
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            battery.Clear();
        }
        else
        {
            throw;
        }
    }

    /*
     * Charge mode: Lenovo Legion driver (the battery is shown also without it)
     */
    if(battery.supported())
    {
        const SysFsDriverLegionGameZone::GameZone::Other other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));
        const std::optional<QString> chargeMode = other.get_power_charge_mode.empty() ? std::nullopt : getOptionalData(other.get_power_charge_mode);

        if(chargeMode.has_value())
        {
            battery.set_current_charge_mode_value(static_cast<legion::messages::Battery::PowerChargeMode>(chargeMode->toUShort()));
            battery.set_charge_mode_supported(true);
        }
        else
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Lenovo Legion driver not available, no charge mode");
        }
    }

    byteArray.resize(battery.ByteSizeLong());
    if(!battery.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderBattery::deserializeAndSetData(const QByteArray &)
{
    return {};
}


}
