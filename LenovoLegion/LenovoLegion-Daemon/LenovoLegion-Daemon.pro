TEMPLATE = app
TARGET = $${APPLICATION_NAME2}

QT       += core network

CONFIG += c++20 cmdline link_pkgconfig

PKGCONFIG += protobuf hidapi-hidraw

DESTDIR = $${DESTINATION_BIN_PATH}

# Main Framework sources
SOURCES +=  \
        Application.cpp \
        DaemonSettingsManager.cpp \
        DataProvider.cpp \
        DataProviderManager.cpp \
        ProtocolParser.cpp \
        ProtocolProcessor.cpp \
        ProtocolProcessorBase.cpp \
        ProtocolProcessorNotifier.cpp \
        RGBController.cpp \
        SysFsDataProvider.cpp \
        SysFsDriver.cpp \
        SysFsDriverManager.cpp \
        Settings.cpp \
        StringUtils.cpp \
        main.cpp

# SysFsDriver sources
SOURCES += \
        SysFsDrivers/SysFsDriverLegionFanMode.cpp \
        SysFsDrivers/SysFsDriverLegionGameZone.cpp \
        SysFsDrivers/SysFsDriverLegionHWMon.cpp \
        SysFsDrivers/SysFsDriverLegionIntelMSR.cpp \
        SysFsDrivers/SysFsDriverCPUAtom.cpp \
        SysFsDrivers/SysFsDriverCPUCore.cpp \
        SysFsDrivers/SysFsDriverCPUInfo.cpp \
        SysFsDrivers/SysFsDriverCPUXList.cpp \
        SysFsDrivers/SysFsDriverIntelPowercapRapl.cpp \
        SysFsDrivers/SysFsDriverLegionEvents.cpp \
        SysFsDrivers/SysFsDriverLegionMachineInformation.cpp \
        SysFsDrivers/SysFsDriverLegionOther.cpp \
        SysFsDrivers/SysFsDriverACPIPlatformProfile.cpp \
        SysFsDrivers/SysFsDriverPowerSuplyBattery0.cpp \
        SysFsDrivers/SysFsDriverCPU.cpp \
        SysFsDrivers/SysFsDriverCPUIntelPState.cpp

# SysFS data provider sources
SOURCES += \
        SysFsDataProviders/SysFsDataProviderBattery.cpp \
        SysFsDataProviders/SysFsDataProviderCPUFrequency.cpp \
        SysFsDataProviders/SysFsDataProviderCPUInfo.cpp \
        SysFsDataProviders/SysFsDataProviderCPUOptions.cpp \
        SysFsDataProviders/SysFsDataProviderCPUPState.cpp \
        SysFsDataProviders/SysFsDataProviderCPUPower.cpp \
        SysFsDataProviders/SysFsDataProviderCPUSMT.cpp \
        SysFsDataProviders/SysFsDataProviderCPUTopology.cpp \
        SysFsDataProviders/SysFsDataProviderFanCurve.cpp \
        SysFsDataProviders/SysFsDataProviderFanOption.cpp \
        SysFsDataProviders/SysFsDataProviderGPUPower.cpp \
        SysFsDataProviders/SysFsDataProviderHWMon.cpp \
        SysFsDataProviders/SysFsDataProviderIntelMSR.cpp \
        SysFsDataProviders/SysFsDataProviderMachineInformation.cpp \
        SysFsDataProviders/SysFsDataProviderOther.cpp \
        SysFsDataProviders/SysFsDataProviderOtherGpuSwitch.cpp \
        SysFsDataProviders/SysFsDataProviderPowerProfile.cpp

#Data provider sources
SOURCES += \
        DataProviders/DataProviderRGBController.cpp \
        DataProviders/DataProviderNvidiaNvml.cpp \
        DataProviders/DataProviderDaemonSettings.cpp

# RGB controller sources
SOURCES += \
        RGBControlers/LenovoRGBController.cpp \
        RGBControlers/LenovoUSBController.cpp \
        RGBControlers/LenovoUSBControllerDetect.cpp \
        RGBControlers/LenovoRGBControllerC197.cpp \
        RGBControlers/LenovoRGBControllerC9xx.cpp \
        RGBControlers/LenovoUSBControllerC9xx.cpp




#Main Framework headers
HEADERS += \
        Application.h \
        DaemonSettingsManager.h \
        DataProvider.h \
        DataProviderManager.h \
        Message.h \
        ProtocolParser.h \
        ProtocolProcessor.h \
        ProtocolProcessorBase.h \
        ProtocolProcessorNotifier.h \
        Settings.h \
        SysFsDataProvider.h \
        SysFsDriver.h \
        SysFsDriverManager.h \
        RGBControllerInterface.h \
        RGBController.h \
        RGBControllerKeyNames.h \
        StringUtils.h \
        RGBControllerDetector.h

# SysFsDriver  headers
HEADERS += \
        SysFsDrivers/SysFsDriverLegionFanMode.h \
        SysFsDrivers/SysFsDriverLegionGameZone.h \
        SysFsDrivers/SysFsDriverLegionHWMon.h \
        SysFsDrivers/SysFsDriverLegionIntelMSR.h \
        SysFsDrivers/SysFsDriverCPUAtom.h \
        SysFsDrivers/SysFsDriverCPUCore.h \
        SysFsDrivers/SysFsDriverCPUInfo.h \
        SysFsDrivers/SysFsDriverCPUXList.h \
        SysFsDrivers/SysFsDriverIntelPowercapRapl.h \
        SysFsDrivers/SysFsDriverLegionEvents.h \
        SysFsDrivers/SysFsDriverLegionMachineInformation.h \
        SysFsDrivers/SysFsDriverLegionOther.h \
        SysFsDrivers/SysFsDriverACPIPlatformProfile.h \
        SysFsDrivers/SysFsDriverPowerSuplyBattery0.h \
        SysFsDrivers/SysFsDriverCPU.h \
        SysFsDrivers/SysFsDriverCPUIntelPState.h \
        SysFsDrivers/SysFsDriverLegion.h


# SysFS data provider headers
HEADERS += \
        SysFsDataProviders/SysFsDataProviderBattery.h \
        SysFsDataProviders/SysFsDataProviderCPUFrequency.h \
        SysFsDataProviders/SysFsDataProviderCPUInfo.h \
        SysFsDataProviders/SysFsDataProviderCPUOptions.h \
        SysFsDataProviders/SysFsDataProviderCPUPState.h \
        SysFsDataProviders/SysFsDataProviderCPUPower.h \
        SysFsDataProviders/SysFsDataProviderCPUSMT.h \
        SysFsDataProviders/SysFsDataProviderCPUTopology.h \
        SysFsDataProviders/SysFsDataProviderFanCurve.h \
        SysFsDataProviders/SysFsDataProviderFanOption.h \
        SysFsDataProviders/SysFsDataProviderGPUPower.h \
        SysFsDataProviders/SysFsDataProviderHWMon.h \
        SysFsDataProviders/SysFsDataProviderIntelMSR.h \
        SysFsDataProviders/SysFsDataProviderMachineInformation.h \
        SysFsDataProviders/SysFsDataProviderOther.h \
        SysFsDataProviders/SysFsDataProviderOtherGpuSwitch.h \
        SysFsDataProviders/SysFsDataProviderPowerProfile.h

# Data provider headers
HEADERS += \
        DataProviders/DataProviderRGBController.h \
        DataProviders/DataProviderNvidiaNvml.h \
        DataProviders/DataProviderDaemonSettings.h


# RGB controller headers
HEADERS += \
        RGBControlers/LenovoRGBController.h \
        RGBControlers/LenovoUSBController.h \
        RGBControlers/LenovoRGBControllerC197.h \
        RGBControlers/LenovoRGBControllerC9xx.h \
        RGBControlers/LenovoUSBControllerC9xx.h

HEADERS += \
        ../LenovoLegion-PrepareBuild/HWMonitoring.pb.h \
        ../LenovoLegion-PrepareBuild/CPUTopology.pb.h  \
        ../LenovoLegion-PrepareBuild/PowerProfile.pb.h \
        ../LenovoLegion-PrepareBuild/Battery.pb.h      \
        ../LenovoLegion-PrepareBuild/CpuPower.pb.h     \
        ../LenovoLegion-PrepareBuild/GPUPower.pb.h     \
        ../LenovoLegion-PrepareBuild/FanControl.pb.h   \
        ../LenovoLegion-PrepareBuild/CPUFrequency.pb.h \
        ../LenovoLegion-PrepareBuild/CPUOptions.pb.h   \
        ../LenovoLegion-PrepareBuild/Notification.pb.h \
        ../LenovoLegion-PrepareBuild/CpuIntelMSR.pb.h  \
        ../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h   \
        ../LenovoLegion-PrepareBuild/ComputerInfo.pb.h \
        ../LenovoLegion-PrepareBuild/DaemonSettings.pb.h \
        ../LenovoLegion-PrepareBuild/Other.pb.h \
        ../LenovoLegion-PrepareBuild/RGBController.pb.h \
        ../LenovoLegion-PrepareBuild/MessageRegistry.pb.h

SOURCES += \
        ../LenovoLegion-PrepareBuild/HWMonitoring.pb.cc \
        ../LenovoLegion-PrepareBuild/CPUTopology.pb.cc  \
        ../LenovoLegion-PrepareBuild/PowerProfile.pb.cc \
        ../LenovoLegion-PrepareBuild/Battery.pb.cc      \
        ../LenovoLegion-PrepareBuild/CpuPower.pb.cc     \
        ../LenovoLegion-PrepareBuild/GPUPower.pb.cc     \
        ../LenovoLegion-PrepareBuild/FanControl.pb.cc   \
        ../LenovoLegion-PrepareBuild/CPUFrequency.pb.cc \
        ../LenovoLegion-PrepareBuild/CPUOptions.pb.cc   \
        ../LenovoLegion-PrepareBuild/Notification.pb.cc \
        ../LenovoLegion-PrepareBuild/CpuIntelMSR.pb.cc  \
        ../LenovoLegion-PrepareBuild/NvidiaNvml.pb.cc   \
        ../LenovoLegion-PrepareBuild/ComputerInfo.pb.cc \
        ../LenovoLegion-PrepareBuild/DaemonSettings.pb.cc \
        ../LenovoLegion-PrepareBuild/Other.pb.cc \
        ../LenovoLegion-PrepareBuild/RGBController.pb.cc \
        ../LenovoLegion-PrepareBuild/MessageRegistry.pb.cc


INCLUDEPATH += $${CUDA_PATH}/include
LIBS += -L$${CUDA_PATH}/lib64 -l$${PROJECT_LIBS_NAME} -ludev -lnvidia-ml

DISTFILES +=  \
    lenovo-legion-daemon.service \
    lenovo-legion.sysusers \
    lenovo-legion-setup.sh \
    ModulesDesign.md
