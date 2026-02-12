#include "DataProviderNvidiaNvml.h"
#include "Core/LoggerHolder.h"

#include "../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h"

namespace LenovoLegionDaemon {


DataProviderNvidiaNvml::DataProviderNvidiaNvml(QObject* parent)  : DataProvider(parent,dataType),
    m_maxGraphicsClock(0),
    m_maxSmClock(0),
    m_maxMemClock(0),
    m_device(nullptr)
{

}

DataProviderNvidiaNvml::~DataProviderNvidiaNvml()
{
    cleanUp();
}

QByteArray DataProviderNvidiaNvml::serializeAndGetData() const
{
    legion::messages::NvidiaNvml gpuData;
    QByteArray                   byteArray;


    LOG_T(__PRETTY_FUNCTION__);

    /*
     * Static data
     */
    if(m_device != nullptr)
    {
        nvmlReturn_t result;
        unsigned int graphicsClock, smClock, memClock;
        int gpuOffset, memOffset;


        LOG_T("Filling NVML GPU data");

        /*
         * Name
         */
        gpuData.set_name(m_GPUName.toStdString());


        /*
         * Min Max values
         */
        gpuData.mutable_hardware_monitor()->mutable_memory_clock()->set_max_value(m_maxMemClock);
        gpuData.mutable_hardware_monitor()->mutable_memory_clock()->set_min_value(0);

        gpuData.mutable_hardware_monitor()->mutable_sm_clock()->set_max_value(m_maxSmClock);
        gpuData.mutable_hardware_monitor()->mutable_sm_clock()->set_min_value(0);

        gpuData.mutable_hardware_monitor()->mutable_gpu_clock()->set_max_value(m_maxGraphicsClock);
        gpuData.mutable_hardware_monitor()->mutable_gpu_clock()->set_min_value(0);

        gpuData.mutable_hardware_monitor()->mutable_gpu_utilization()->set_max_value(100);
        gpuData.mutable_hardware_monitor()->mutable_gpu_utilization()->set_min_value(0);

        gpuData.mutable_hardware_monitor()->mutable_memory_utilization()->set_max_value(100);
        gpuData.mutable_hardware_monitor()->mutable_memory_utilization()->set_min_value(0);

        /*
         * Specific data
         */
        gpuData.mutable_hardware_monitor()->mutable_temperature()->set_shutdown(m_shutdownTempThreshold);
        gpuData.mutable_hardware_monitor()->mutable_temperature()->set_slowdown(m_slowdownTempThreshold);


        gpuData.mutable_hardware_monitor()->mutable_power()->set_min_value(m_powerLimitMin);
        gpuData.mutable_hardware_monitor()->mutable_power()->set_max_value(m_powerLimitMax);


        gpuData.mutable_hardware_monitor()->mutable_pcie()->set_generation_max(m_pciGenerationMax);
        gpuData.mutable_hardware_monitor()->mutable_pcie()->set_width_max(m_pciWidthMax);


        gpuData.mutable_gpu_offset()->set_min(m_minGpuOffset);
        gpuData.mutable_gpu_offset()->set_max(m_maxGpuOffset);
        gpuData.mutable_memory_offset()->set_min(m_minMemOffset);
        gpuData.mutable_memory_offset()->set_max(m_maxMemOffset);



        // Get current clock speeds
        result = nvmlDeviceGetClockInfo(m_device, NVML_CLOCK_GRAPHICS, &graphicsClock);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_gpu_clock()->set_value(graphicsClock);
        }

        result = nvmlDeviceGetClockInfo(m_device, NVML_CLOCK_SM, &smClock);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_sm_clock()->set_value(smClock);
        }

        result = nvmlDeviceGetClockInfo(m_device, NVML_CLOCK_MEM, &memClock);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_memory_clock()->set_value(memClock);
        }


        // Get utilization (GPU and memory)
        nvmlUtilization_t utilization;
        result = nvmlDeviceGetUtilizationRates(m_device, &utilization);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_gpu_utilization()->set_value(utilization.gpu);
            gpuData.mutable_hardware_monitor()->mutable_memory_utilization()->set_value(utilization.memory);
        }

        // Get temperature
        nvmlTemperature_t tempInfo;
        tempInfo.version = nvmlTemperature_v1;
        tempInfo.sensorType = NVML_TEMPERATURE_GPU;
        result = nvmlDeviceGetTemperatureV(m_device, &tempInfo);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_temperature()->set_value(tempInfo.temperature);
        }

        // Get memory info
        nvmlMemory_t memory;
        result = nvmlDeviceGetMemoryInfo(m_device, &memory);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_memory_use()->set_used(memory.used);
            gpuData.mutable_hardware_monitor()->mutable_memory_use()->set_free(memory.free);
            gpuData.mutable_hardware_monitor()->mutable_memory_use()->set_total(memory.total);
        }

        // Get current power draw
        unsigned int power_draw;
        result = nvmlDeviceGetPowerUsage(m_device, &power_draw);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_power()->set_value(power_draw);
        }

        // Get total energy consumption
        unsigned long long total_energy;
        result = nvmlDeviceGetTotalEnergyConsumption(m_device, &total_energy);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_power()->set_total(total_energy);
        }

        unsigned int default_power_limit;
        result = nvmlDeviceGetPowerManagementDefaultLimit(m_device, &default_power_limit);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_power()->set_default_value(default_power_limit);
        }

        // Get enforced power limit (actual limit being enforced)
        unsigned int enforced_power_limit;
        result = nvmlDeviceGetEnforcedPowerLimit(m_device, &enforced_power_limit);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_power()->set_enforced_value(enforced_power_limit);
        }

        // Get PCIe info
        unsigned int curr_link_gen, curr_link_width;
        result = nvmlDeviceGetCurrPcieLinkGeneration(m_device, &curr_link_gen);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_pcie()->set_generation(curr_link_gen);
        }

        result = nvmlDeviceGetCurrPcieLinkWidth(m_device, &curr_link_width);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_pcie()->set_width(curr_link_width);
        }

        // Get PCIe throughput
        unsigned int tx_throughput, rx_throughput;
        result = nvmlDeviceGetPcieThroughput(m_device, NVML_PCIE_UTIL_TX_BYTES, &tx_throughput);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_pcie()->set_tx_bytes(tx_throughput);
        }

        result = nvmlDeviceGetPcieThroughput(m_device, NVML_PCIE_UTIL_RX_BYTES, &rx_throughput);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_hardware_monitor()->mutable_pcie()->set_rx_bytes(rx_throughput);
        }

        // Get GPU clock offset
        result = nvmlDeviceGetGpcClkVfOffset(m_device, &gpuOffset);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_gpu_offset()->set_value(gpuOffset);
        }

        // Get Memory clock offset
        result = nvmlDeviceGetMemClkVfOffset(m_device, &memOffset);
        if (NVML_SUCCESS == result) {
            gpuData.mutable_memory_offset()->set_value(memOffset);
        }
    }
    else
    {
        LOG_T("No NVIDIA GPU detected, skipping data collection.");
    }


    byteArray.resize(gpuData.ByteSizeLong());
    if(!gpuData.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray DataProviderNvidiaNvml::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::NvidiaNvml       nvmlData;

    LOG_T(__PRETTY_FUNCTION__);


    if(!nvmlData.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    /*
      * Apply GPU offset
      */
    if(m_device == nullptr)
    {
        LOG_W("No NVIDIA GPU device available to set data.");
        return {};
    }

    if(nvmlData.has_gpu_offset())
    {
        nvmlReturn_t result;
        nvmlClockOffset_t gpuClkOffset;
        gpuClkOffset.version = nvmlClockOffset_v1;
        gpuClkOffset.type = NVML_CLOCK_GRAPHICS;
        gpuClkOffset.pstate = NVML_PSTATE_0;
        gpuClkOffset.clockOffsetMHz = nvmlData.gpu_offset().value();

        result = nvmlDeviceSetClockOffsets(m_device, &gpuClkOffset);
        if (NVML_SUCCESS != result) {
            LOG_D(QString("Failed to set GPU clock offset: %1").arg(nvmlErrorString(result)));
        }
    }

    /*
      * Apply Memory offset
      */
    if(nvmlData.has_memory_offset())
    {
        nvmlReturn_t result;
        nvmlClockOffset_t memClkOffset;
        memClkOffset.version = nvmlClockOffset_v1;
        memClkOffset.type = NVML_CLOCK_MEM;
        memClkOffset.pstate = NVML_PSTATE_0;
        memClkOffset.clockOffsetMHz = nvmlData.memory_offset().value();

        result = nvmlDeviceSetClockOffsets(m_device, &memClkOffset);
        if (NVML_SUCCESS != result) {
            LOG_D(QString("Failed to set Memory clock offset: %1").arg(nvmlErrorString(result)));
        }
    }

    return {};
}

void DataProviderNvidiaNvml::init()
{
    try {
        nvmlReturn_t result;
        unsigned int device_count, i;


        // Initialize NVML library
        result = nvmlInit();
        if (NVML_SUCCESS != result) {
            THROW_EXCEPTION(exception_T,ERROR_NVML_INIT_FAILED, "NVML initialization failed");
        }

        // Get number of GPU devices
        result = nvmlDeviceGetCount(&device_count);
        if (NVML_SUCCESS != result) {
            nvmlShutdown();
            THROW_EXCEPTION(exception_T,ERROR_NVML_DEVICE_COUNT_FAILED, "Failed to get NVML device count");
        }


        if(device_count == 0){
            nvmlShutdown();
            THROW_EXCEPTION(exception_T,ERROR_NVML_DEVICE_COUNT_FAILED, "Unusual NVML device count");
        }

        LOG_D(QString("NVIDIA NVML initialized with %1 device(s)").arg(device_count));

        for (i = 0; i < device_count; i++) {
            char name[NVML_DEVICE_NAME_BUFFER_SIZE];

            // Get device handle
            result = nvmlDeviceGetHandleByIndex(i, &m_device);
            if (NVML_SUCCESS != result) {
                m_device = nullptr;
                LOG_W(QString("Failed to get handle for device %1: %2").arg(i).arg(nvmlErrorString(result)));
                continue;
            }

            // Get device name
            result = nvmlDeviceGetName(m_device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
            if (NVML_SUCCESS != result) {
                m_device = nullptr;
                LOG_W(QString("Failed to get name for device %1: %2").arg(i).arg(nvmlErrorString(result)));
                continue;
            }

            // Get max clocks
            result = nvmlDeviceGetMaxClockInfo(m_device, NVML_CLOCK_GRAPHICS, &m_maxGraphicsClock);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get max graphics clock for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            result = nvmlDeviceGetMaxClockInfo(m_device, NVML_CLOCK_SM, &m_maxSmClock);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get max SM clock for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            result = nvmlDeviceGetMaxClockInfo(m_device, NVML_CLOCK_MEM, &m_maxMemClock);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get max memory clock for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            // Get temperature thresholds
            result = nvmlDeviceGetTemperatureThreshold(m_device, NVML_TEMPERATURE_THRESHOLD_SHUTDOWN, &m_shutdownTempThreshold);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get shutdown temperature for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }
            result = nvmlDeviceGetTemperatureThreshold(m_device, NVML_TEMPERATURE_THRESHOLD_SLOWDOWN, &m_slowdownTempThreshold);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get slowdown temperature for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            result = nvmlDeviceGetPowerManagementLimitConstraints(m_device, &m_powerLimitMin, &m_powerLimitMax);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get power limit constraints for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            result = nvmlDeviceGetMaxPcieLinkGeneration(m_device, &m_pciGenerationMax);
            if (NVML_SUCCESS != result){
                LOG_W(QString("Failed to get PCIe generation max for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            result = nvmlDeviceGetMaxPcieLinkWidth(m_device, &m_pciWidthMax);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get PCIe width max for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            // Get GPU clock offset
            result = nvmlDeviceGetGpcClkMinMaxVfOffset(m_device, &m_minGpuOffset, &m_maxGpuOffset);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get GPU clock offset for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            // Get Memory clock offset
            result = nvmlDeviceGetMemClkMinMaxVfOffset(m_device, &m_minMemOffset, &m_maxMemOffset);
            if (NVML_SUCCESS != result) {
                LOG_W(QString("Failed to get Memory clock offset for device %1: %2").arg(i).arg(nvmlErrorString(result)));
            }

            m_GPUName = QString(name);
            break;
        }

        if(m_device == nullptr)
        {
            THROW_EXCEPTION(exception_T,ERROR_NVML_DEVICE_COUNT_FAILED, "No NVIDIA GPU found");
        }

    } catch(exception_T& ex)
    {
        cleanUp();
        LOG_W(QString("DataProviderNvidiaNvml initialization failed: %1").arg(ex.what()));
    }
}

void DataProviderNvidiaNvml::clean()
{
    cleanUp();
}

void DataProviderNvidiaNvml::cleanUp()
{
    if(m_device != nullptr)
    {
        if(nvmlShutdown() != NVML_SUCCESS){
            LOG_W("NVML shutdown failed");
        }
        else {
            m_device = nullptr;
            LOG_T("NVML shutdown successful");
        }
    }
}

}
