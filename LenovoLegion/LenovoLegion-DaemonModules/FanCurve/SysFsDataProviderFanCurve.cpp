// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderFanCurve.h"
#include <Drivers/SysFsDriverLegionFanMode.h>

#include <FanControl.pb.h>


#include <KitLog.h>

#include <QStringList>

#include <utility>


namespace LenovoLegionDaemon::Kit {

SysFsDataProviderFanCurve::SysFsDataProviderFanCurve(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}


QByteArray SysFsDataProviderFanCurve::serializeAndGetData()
{
    legion::messages::FanCurve         fanCurveMsg;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFsDriverLegionFanMode::FanMode::FanCurve fanCurve(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionFanMode::DRIVER_NAME));

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
    SysFsDriverLegionFanMode::FanMode::FanCurve fanCurve(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionFanMode::DRIVER_NAME));
    legion::messages::FanCurve         fanCurveMsg;

    LOG_T(__PRETTY_FUNCTION__);

    if(!fanCurveMsg.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    settable(fanCurveMsg,Origin::REQUEST);

    m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionFanMode::DRIVER_NAME,true);
    auto cleanup =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverLegionFanMode::DRIVER_NAME,false); });

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

    m_sysFsDriverManager->processAllUdevEvents(100);

    return {};
}

QByteArray SysFsDataProviderFanCurve::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::FanCurve>(data,[origin](legion::messages::FanCurve& fanCurve) { settable(fanCurve,origin); });
}

void SysFsDataProviderFanCurve::settable(legion::messages::FanCurve &fanCurve, Origin origin)
{
    /*
     * Kernel accepts only the whole curve, 10 points with level 0..10 (fan curve is set only in the custom thermal mode, kernel returns -EBUSY otherwise)
     */
    static constexpr quint32 MAX_LEVEL = 10;

    fanCurve.clear_cpu_default();
    fanCurve.clear_gpu_default();
    fanCurve.clear_cpusen_default();
    fanCurve.clear_sys_default();

    if(!fanCurve.has_current_value())
    {
        return;
    }

    const auto& points = fanCurve.current_value();

    const std::pair<bool,quint32> values[] = {
        {points.has_point1(), points.point1()}, {points.has_point2(), points.point2()}, {points.has_point3(), points.point3()},
        {points.has_point4(), points.point4()}, {points.has_point5(), points.point5()}, {points.has_point6(), points.point6()},
        {points.has_point7(), points.point7()}, {points.has_point8(), points.point8()}, {points.has_point9(), points.point9()},
        {points.has_point10(),points.point10()}
    };

    QStringList curve;

    bool complete = true;
    bool inRange  = true;

    for(const auto& [has,value] : values)
    {
        complete = complete && has;
        inRange  = inRange && value <= MAX_LEVEL;
        curve.append(has ? QString::number(value) : QString("-"));
    }

    if(!complete)
    {
        skipField(origin,QString("fan curve=").append(curve.join(',')),"all 10 points are required");
        fanCurve.clear_current_value();
    }
    else if(!inRange)
    {
        skipField(origin,QString("fan curve=").append(curve.join(',')),"level range <0,10>");
        fanCurve.clear_current_value();
    }
}


}
