#include "SysFsDataProviderCPUInfo.h"


#include "Core/LoggerHolder.h"
#include "SysFsDriverCPUInfo.h"

#include "../LenovoLegion-PrepareBuild/ComputerInfo.pb.h"

#include <cpuid.h>

namespace LenovoLegionDaemon {

SysFsDataProviderCPUInfo::SysFsDataProviderCPUInfo(SysFsDriverManager *sysFsDriverManager, QObject *parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}

QByteArray SysFsDataProviderCPUInfo::serializeAndGetData() const
{
    legion::messages::CPUInfo       cpuInfoMessage;
    QByteArray                      byteArray;


    LOG_T(__PRETTY_FUNCTION__);

    try {
        // Read CPU info from sysfs
        SysFsDriverCPUInfo::CPUInfo   cpuInfo(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUInfo::DRIVER_NAME),m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUInfo::DRIVER_NAME));

        std::string vendor, family, model;

        // Fill CPU info message
        cpuInfoMessage.set_raw_modalias(getData(cpuInfo.m_modalias).toStdString());
        parseModalias(cpuInfoMessage.raw_modalias().data(), vendor, family, model);


        cpuInfoMessage.set_vendor_code(vendor);
        cpuInfoMessage.set_family(QString(family.c_str()).toInt(nullptr,16));
        cpuInfoMessage.set_model(QString(model.c_str()).toInt(nullptr,16));
        cpuInfoMessage.set_cpu_model(getCpuBrandString());
        cpuInfoMessage.set_cpu_generation(getCpuName(QString(family.c_str()).toInt(nullptr,16), QString(model.c_str()).toInt(nullptr,16)));
        cpuInfoMessage.set_architecture(getArchitecture(cpuInfoMessage.raw_modalias().data()));

        for (const auto& cache : cpuInfo.m_caches) {
            legion::messages::CPUInfo::CacheInfo* cacheInfo = cpuInfoMessage.add_caches();
            cacheInfo->set_level(getData(cache.m_level).toUInt());
            cacheInfo->set_type(getData(cache.m_type).toStdString());
            cacheInfo->set_size(getData(cache.m_size).toStdString());
        }

        cpuInfoMessage.set_physical_package_id(getData(cpuInfo.m_package_id).toInt());
    }
    catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuInfoMessage.Clear();
        }
        else
        {
            throw;
        }
    }





    byteArray.resize(cpuInfoMessage.ByteSizeLong());
    if(!cpuInfoMessage.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUInfo::deserializeAndSetData(const QByteArray &)
{
    return {};
}


std::string SysFsDataProviderCPUInfo::getCpuName(int family, int model) const {
    // Intel Family 6 models (most common)
    if (family == 6) {
        static const std::map<int, std::string> intel_models = {
            // Arrow Lake (15th gen)
            {0xC5, "Intel Core Ultra 15th Gen (Arrow Lake)"},
            {0xC6, "Intel Core Ultra 15th Gen (Arrow Lake)"},
            // Raptor Lake (13th/14th gen)
            {0xB7, "Intel Core 13th/14th Gen (Raptor Lake/Raptor Lake Refresh)"},
            {0xBA, "Intel Core 13th Gen (Raptor Lake)"},
            {0xBF, "Intel Core 13th Gen (Raptor Lake)"},
            // Alder Lake (12th gen)
            {0x97, "Intel Core 12th Gen (Alder Lake)"},
            {0x9A, "Intel Core 12th Gen (Alder Lake)"},
            // Tiger Lake (11th gen)
            {0x8C, "Intel Core 11th Gen (Tiger Lake)"},
            {0x8D, "Intel Core 11th Gen (Tiger Lake)"},
            // Ice Lake (10th gen)
            {0x7D, "Intel Core 10th Gen (Ice Lake)"},
            {0x7E, "Intel Core 10th Gen (Ice Lake)"},
            // Comet Lake (10th gen)
            {0xA5, "Intel Core 10th Gen (Comet Lake)"},
            {0xA6, "Intel Core 10th Gen (Comet Lake)"},
            // Coffee Lake (8th/9th gen)
            {0x9E, "Intel Core 8th/9th Gen (Coffee Lake)"},
            {0x9C, "Intel Core 8th/9th Gen (Coffee Lake)"},
            // Kaby Lake (7th gen)
            {0x8E, "Intel Core 7th Gen (Kaby Lake)"},
            {0x9E, "Intel Core 7th Gen (Kaby Lake)"},
            // Skylake (6th gen)
            {0x4E, "Intel Core 6th Gen (Skylake)"},
            {0x5E, "Intel Core 6th Gen (Skylake)"},
            // Broadwell (5th gen)
            {0x47, "Intel Core 5th Gen (Broadwell)"},
            {0x3D, "Intel Core 5th Gen (Broadwell)"},
            // Haswell (4th gen)
            {0x3C, "Intel Core 4th Gen (Haswell)"},
            {0x45, "Intel Core 4th Gen (Haswell)"},
            {0x46, "Intel Core 4th Gen (Haswell)"},
            // Ivy Bridge (3rd gen)
            {0x3A, "Intel Core 3rd Gen (Ivy Bridge)"},
            {0x3E, "Intel Core 3rd Gen (Ivy Bridge)"},
            // Sandy Bridge (2nd gen)
            {0x2A, "Intel Core 2nd Gen (Sandy Bridge)"},
            {0x2D, "Intel Core 2nd Gen (Sandy Bridge)"},
            };

        auto it = intel_models.find(model);
        if (it != intel_models.end()) {
            return it->second;
        }
        return "Intel Family 6 Model " + std::to_string(model);
    }

    // AMD Family identifiers
    if (family == 0x17) {
        return "AMD Zen/Zen+/Zen2 (Family 17h)";
    } else if (family == 0x19) {
        return "AMD Zen 3/Zen 4 (Family 19h)";
    } else if (family == 0x1A) {
        return "AMD Zen 5 (Family 1Ah)";
    } else if (family == 0x15) {
        return "AMD Bulldozer/Piledriver/Steamroller/Excavator (Family 15h)";
    }

    return "Unknown CPU (Family " + std::to_string(family) + ", Model " + std::to_string(model) + ")";
}




void SysFsDataProviderCPUInfo::parseModalias(const std::string& modalias, std::string& vendor,std::string& family, std::string& model) const {
    // Format: cpu:type:x86,ven0000fam0006mod00B7:feature:...
    size_t ven_pos = modalias.find("ven");
    size_t fam_pos = modalias.find("fam");
    size_t mod_pos = modalias.find("mod");

    if (ven_pos != std::string::npos && fam_pos != std::string::npos) {
        vendor = modalias.substr(ven_pos + 3, 4);
        family = modalias.substr(fam_pos + 3, 4);
        model = modalias.substr(mod_pos + 3, 4);
    }
}


std::string SysFsDataProviderCPUInfo::getArchitecture(const std::string& modalias) const {
    if (modalias.find("type:x86") != std::string::npos) {
        return "x86_64";
    } else if (modalias.find("type:arm") != std::string::npos) {
        return "ARM";
    }
    return "Unknown";
}

std::string SysFsDataProviderCPUInfo::getCpuBrandString() const {
    // Check if extended CPUID is available
    unsigned int eax, ebx, ecx, edx;
    if (!__get_cpuid(0x80000000, &eax, &ebx, &ecx, &edx)) {
        return "";
    }

    // Need at least 0x80000004 for brand string
    if (eax < 0x80000004) {
        return "";
    }

    // Brand string is in CPUID leaves 0x80000002, 0x80000003, 0x80000004
    char brand[48];
    unsigned int *brand_uint = reinterpret_cast<unsigned int*>(brand);

    __get_cpuid(0x80000002, &brand_uint[0], &brand_uint[1], &brand_uint[2], &brand_uint[3]);
    __get_cpuid(0x80000003, &brand_uint[4], &brand_uint[5], &brand_uint[6], &brand_uint[7]);
    __get_cpuid(0x80000004, &brand_uint[8], &brand_uint[9], &brand_uint[10], &brand_uint[11]);

    brand[47] = '\0';

    // Trim leading spaces
    char* start = brand;
    while (*start == ' ') start++;

    return std::string(start);
}

}
