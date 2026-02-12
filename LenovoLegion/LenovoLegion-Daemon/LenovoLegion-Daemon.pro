TEMPLATE = app
TARGET = $${APPLICATION_NAME2}

QT       += core network

CONFIG += c++20 cmdline link_pkgconfig

PKGCONFIG += protobuf hidapi-hidraw

DESTDIR = $${DESTINATION_BIN_PATH}

SOURCES +=  \
        Application.cpp \
        DaemonSettingsManager.cpp \
        DataProvider.cpp \
        DataProviderDaemonSettings.cpp \
        DataProviderManager.cpp \
        DataProviderNvidiaNvml.cpp \
        DataProviderRGBController.cpp \
        ProtocolParser.cpp \
        ProtocolProcessor.cpp \
        ProtocolProcessorBase.cpp \
        ProtocolProcessorNotifier.cpp \
        RGBControlers/LenovoRGBControllerC197.cpp \
        RGBControlers/LenovoRGBControllerC9xx.cpp \
        RGBControlers/LenovoUSBControllerC9xx.cpp \
        RGBController.cpp \
        SysFSDriverLegionFanMode.cpp \
        SysFSDriverLegionGameZone.cpp \
        SysFSDriverLegionHWMon.cpp \
        SysFSDriverLegionIntelMSR.cpp \
        SysFsDataProvider.cpp \
        SysFsDataProviderBattery.cpp \
        SysFsDataProviderCPUFrequency.cpp \
        SysFsDataProviderCPUInfo.cpp \
        SysFsDataProviderCPUOptions.cpp \
        SysFsDataProviderCPUPower.cpp \
        SysFsDataProviderCPUSMT.cpp \
        SysFsDataProviderCPUTopology.cpp \
        SysFsDataProviderFanCurve.cpp \
        SysFsDataProviderFanOption.cpp \
        SysFsDataProviderGPUPower.cpp \
        SysFsDataProviderHWMon.cpp \
        SysFsDataProviderIntelMSR.cpp \
        SysFsDataProviderMachineInformation.cpp \
        SysFsDataProviderOther.cpp \
        SysFsDataProviderOtherGpuSwitch.cpp \
        SysFsDataProviderPowerProfile.cpp \
        SysFsDriver.cpp \
        SysFsDriverACPIPlatformProfile.cpp \
        SysFsDriverCPU.cpp \
        SysFsDriverCPUAtom.cpp \
        SysFsDriverCPUCore.cpp \
        SysFsDriverCPUInfo.cpp \
        SysFsDriverCPUXList.cpp \
        SysFsDriverIntelPowercapRapl.cpp \
        SysFsDriverLegionEvents.cpp \
        SysFsDriverLegionMachineInformation.cpp \
        SysFsDriverLegionOther.cpp \
        SysFsDriverManager.cpp \
        SysFsDriverPowerSuplyBattery0.cpp \
        Settings.cpp \
        StringUtils.cpp \
        main.cpp

HEADERS += \
    Application.h \
    DaemonSettingsManager.h \
    DataProvider.h \
    DataProviderDaemonSettings.h \
    DataProviderManager.h \
    DataProviderNvidiaNvml.h \
    DataProviderRGBController.h \
    Message.h \
    ProtocolParser.h \
    ProtocolProcessor.h \
    ProtocolProcessorBase.h \
    ProtocolProcessorNotifier.h \
    RGBControlers/LenovoRGBControllerC197.h \
    RGBControlers/LenovoRGBControllerC9xx.h \
    RGBControlers/LenovoUSBControllerC9xx.h \
    SysFSDriverLegionFanMode.h \
    SysFSDriverLegionGameZone.h \
    Settings.h \
    SysFSDriverLegionHWMon.h \
    SysFSDriverLegionIntelMSR.h \
    SysFsDataProvider.h \
    SysFsDataProviderBattery.h \
    SysFsDataProviderCPUFrequency.h \
    SysFsDataProviderCPUInfo.h \
    SysFsDataProviderCPUOptions.h \
    SysFsDataProviderCPUPower.h \
    SysFsDataProviderCPUSMT.h \
    SysFsDataProviderCPUTopology.h \
    SysFsDataProviderFanCurve.h \
    SysFsDataProviderFanOption.h \
    SysFsDataProviderGPUPower.h \
    SysFsDataProviderHWMon.h \
    SysFsDataProviderIntelMSR.h \
    SysFsDataProviderMachineInformation.h \
    SysFsDataProviderOther.h \
    SysFsDataProviderOtherGpuSwitch.h \
    SysFsDataProviderPowerProfile.h \
    SysFsDriver.h \
    SysFsDriverACPIPlatformProfile.h \
    SysFsDriverCPU.h \
    SysFsDriverCPUAtom.h \
    SysFsDriverCPUCore.h \
    SysFsDriverCPUInfo.h \
    SysFsDriverCPUXList.h \
    SysFsDriverIntelPowercapRapl.h \
    SysFsDriverLegion.h \
    SysFsDriverLegionEvents.h \
    SysFsDriverLegionMachineInformation.h \
    SysFsDriverLegionOther.h \
    SysFsDriverManager.h \
    SysFsDriverPowerSuplyBattery0.h \
    RGBControllerInterface.h \
    RGBController.h \
    RGBControllerKeyNames.h \
    StringUtils.h \
    RGBControllerDetector.h


HEADERS += \
        RGBControlers/LenovoRGBController.h \
        RGBControlers/LenovoUSBController.h

SOURCES += \
        RGBControlers/LenovoRGBController.cpp \
        RGBControlers/LenovoUSBController.cpp \
        RGBControlers/LenovoUSBControllerDetect.cpp


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
        ../LenovoLegion-PrepareBuild/RGBController.pb.h

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
        ../LenovoLegion-PrepareBuild/RGBController.pb.cc


INCLUDEPATH += $${CUDA_PATH}/include
LIBS += -L$${CUDA_PATH}/lib64 -l$${PROJECT_LIBS_NAME} -ludev -lnvidia-ml

DISTFILES +=  \
    lenovo-legion-daemon.service
