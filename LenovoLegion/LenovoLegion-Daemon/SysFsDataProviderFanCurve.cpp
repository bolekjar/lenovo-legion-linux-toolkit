// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderFanCurve.h"
#include "SysFSDriverLegionFanMode.h"

#include "../LenovoLegion-PrepareBuild/FanControl.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderFanCurve::SysFsDataProviderFanCurve(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}


QByteArray SysFsDataProviderFanCurve::serializeAndGetData() const
{
    legion::messages::FanCurve         fanCurveMsg;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFSDriverLegionFanMode::FanMode::FanCurve fanCurve(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionFanMode::DRIVER_NAME));

        auto setValuesSteps = [](const std::filesystem::path& path,std::function<void (legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values)> setter,auto map)
        {

            if(!getData(path).trimmed().isEmpty())
            {
                auto modeValue = getData(path).split(',');

                for (unsigned int i = 0; i < modeValue.size(); ++i) {
                    auto range = modeValue.at(i).split('=');

                    if(range.size() == 2)
                    {
                        setter((*map)[range.at(0).toUInt()],range.at(1).split("|"));
                    }
                    else
                    {
                        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid FanCurve data !").c_str());
                    }
                }
            }
        };



        auto fanCurveData = getData(fanCurve.m_current_value).split(',');

        if(static_cast<size_t>(fanCurveData.size()) != 10)
        {
            THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid fan curve data !").c_str());
        }



        fanCurveMsg.mutable_current_value()->set_point1(fanCurveData.at(0).toInt());
        fanCurveMsg.mutable_current_value()->set_point2(fanCurveData.at(1).toInt());
        fanCurveMsg.mutable_current_value()->set_point3(fanCurveData.at(2).toInt());
        fanCurveMsg.mutable_current_value()->set_point4(fanCurveData.at(3).toInt());
        fanCurveMsg.mutable_current_value()->set_point5(fanCurveData.at(4).toInt());
        fanCurveMsg.mutable_current_value()->set_point6(fanCurveData.at(5).toInt());
        fanCurveMsg.mutable_current_value()->set_point7(fanCurveData.at(6).toInt());
        fanCurveMsg.mutable_current_value()->set_point8(fanCurveData.at(7).toInt());
        fanCurveMsg.mutable_current_value()->set_point9(fanCurveData.at(8).toInt());
        fanCurveMsg.mutable_current_value()->set_point10(fanCurveData.at(9).toInt());


        /*
         * Default values
         */
        setValuesSteps(fanCurve.m_cpu_fan_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
            for (const auto& val : values) {
                defaultValues.add_fan(val.toUInt());
            }
        },fanCurveMsg.mutable_cpu_default());
        setValuesSteps(fanCurve.m_cpu_sensor_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
            for (const auto& val : values) {
                defaultValues.add_sensors(val.toUInt());
            }
        },fanCurveMsg.mutable_cpu_default());

        setValuesSteps(fanCurve.m_gpu_fan_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
            for (const auto& val : values) {
                defaultValues.add_fan(val.toUInt());
            }
        },fanCurveMsg.mutable_gpu_default());
        setValuesSteps(fanCurve.m_gpu_sensor_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
            for (const auto& val : values) {
                defaultValues.add_sensors(val.toUInt());
            }
        },fanCurveMsg.mutable_gpu_default());



        /*
         * Optionaly
         */
        if(!fanCurve.m_cpusen_fan_default.empty() && !fanCurve.m_cpusen_sensor_default.empty())
        {
            setValuesSteps(fanCurve.m_cpusen_fan_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
                for (const auto& val : values) {
                    defaultValues.add_fan(val.toUInt());
                }
            },fanCurveMsg.mutable_cpusen_default());
            setValuesSteps(fanCurve.m_cpusen_sensor_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
                for (const auto& val : values) {
                    defaultValues.add_sensors(val.toUInt());
                }
            },fanCurveMsg.mutable_cpusen_default());
        }

        if(!fanCurve.m_sys_fan_default.empty() && !fanCurve.m_sys_sensor_default.empty())
        {
            setValuesSteps(fanCurve.m_sys_fan_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
                for (const auto& val : values) {
                    defaultValues.add_fan(val.toUInt());
                }
            },fanCurveMsg.mutable_sys_default());
            setValuesSteps(fanCurve.m_sys_sensor_default,[&](legion::messages::FanCurve::Default &defaultValues,const QList<QString>& values){
                for (const auto& val : values) {
                    defaultValues.add_sensors(val.toUInt());
                }
            },fanCurveMsg.mutable_sys_default());

        }

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            fanCurveMsg.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(fanCurveMsg.ByteSizeLong());
    if(!fanCurveMsg.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderFanCurve::deserializeAndSetData(const QByteArray &data)
{
    SysFSDriverLegionFanMode::FanMode::FanCurve fanCurve(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionFanMode::DRIVER_NAME));
    legion::messages::FanCurve         fanCurveMsg;

    LOG_T(__PRETTY_FUNCTION__);

    if(!fanCurveMsg.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    if(fanCurveMsg.has_current_value()) {
        setData(fanCurve.m_current_value,std::vector<quint8>{
                static_cast<quint8>(fanCurveMsg.current_value().point1()),
                static_cast<quint8>(fanCurveMsg.current_value().point2()),
                static_cast<quint8>(fanCurveMsg.current_value().point3()),
                static_cast<quint8>(fanCurveMsg.current_value().point4()),
                static_cast<quint8>(fanCurveMsg.current_value().point5()),
                static_cast<quint8>(fanCurveMsg.current_value().point6()),
                static_cast<quint8>(fanCurveMsg.current_value().point7()),
                static_cast<quint8>(fanCurveMsg.current_value().point8()),
                static_cast<quint8>(fanCurveMsg.current_value().point9()),
                static_cast<quint8>(fanCurveMsg.current_value().point10())
                });
    }

    return {};
}


}
