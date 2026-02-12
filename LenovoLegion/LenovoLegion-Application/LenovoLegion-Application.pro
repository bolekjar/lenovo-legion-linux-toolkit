TEMPLATE = app
TARGET = $${APPLICATION_NAME}

QT       += core gui widgets network charts

CONFIG += c++20 link_pkgconfig
PKGCONFIG += protobuf

DESTDIR = $${DESTINATION_BIN_PATH}

SOURCES +=               \
        AboutWindow.cpp \
        Application.cpp  \
        BateryStatus.cpp \
        CPUControl.cpp \
        CPUFrequency.cpp \
        CPUFrequencyControl.cpp \
        DataProvider.cpp \
        DataProviderManager.cpp \
        FanControl.cpp \
        GPUDetails.cpp \
        HWMonitor.cpp \
        HWMonitoring.cpp \
        MainWindow.cpp   \
        OffsetsControl.cpp \
        OtherControl.cpp \
        PowerControl.cpp \
        PowerProfileControl.cpp \
        ProtocolProcessor.cpp \
        ProtocolProcessorBase.cpp \
        ProtocolProcessorNotifier.cpp \
        RGBController.cpp \
        RGBKeyboardDevice.cpp \
        Settings.cpp \
        TaskList.cpp \
        ThreadControl.cpp \
        ThreadFrequency.cpp \
        ThreadFrequencyControl.cpp \
        ToolBarHomeWidget.cpp \
        ToolBarHelpWidget.cpp \
        ToolBarInformationWidget.cpp \
        ToolBarKeyboardWidget.cpp \
        ToolBarProfilesWidget.cpp \
        ToolBarSettingsWidget.cpp \
        ToolBarWidget.cpp \
        Utils.cpp \
        QTooltipedSlider.cpp \
        ColorWheel.cpp \
        DeviceView.cpp \
        Swatches.cpp \
        main.cpp

HEADERS +=          \
        AboutWindow.h \
        Application.h \
        BateryStatus.h \
        CPUControl.h \
        CPUFrequency.h \
        CPUFrequencyControl.h \
        DataProvider.h \
        DataProviderManager.h \
        FanControl.h \
        GPUDetails.h \
        HWMonitor.h \
        HWMonitoring.h \
        MainWindow.h  \
        OffsetsControl.h \
        OtherControl.h \
        PowerControl.h \
        PowerProfileControl.h \
        ProtocolProcessor.h \
        ProtocolProcessorBase.h \
        ProtocolProcessorNotifier.h \
        RGBController.h \
        RGBKeyboardDevice.h \
        Settings.h \
        TaskList.h \
        ThreadControl.h \
        ThreadFrequency.h \
        ThreadFrequencyControl.h \
        ToolBarHomeWidget.h \
        ToolBarHelpWidget.h \
        ToolBarInformationWidget.h \
        ToolBarKeyboardWidget.h \
        ToolBarProfilesWidget.h \
        ToolBarSettingsWidget.h \
        ToolBarWidget.h \
        Utils.h \
        QTooltipedSlider.h \
        ColorWheel.h \
        DeviceView.h \
        WidgetMessage.h \
        Swatches.h

HEADERS += \
        ../LenovoLegion-Daemon/ProtocolParser.h \
        ../LenovoLegion-PrepareBuild/HWMonitoring.pb.h \
        ../LenovoLegion-PrepareBuild/CPUTopology.pb.h \
        ../LenovoLegion-PrepareBuild/PowerProfile.pb.h \
        ../LenovoLegion-PrepareBuild/Battery.pb.h \
        ../LenovoLegion-PrepareBuild/CpuPower.pb.h \
        ../LenovoLegion-PrepareBuild/GPUPower.pb.h \
        ../LenovoLegion-PrepareBuild/FanControl.pb.h  \
        ../LenovoLegion-PrepareBuild/CPUFrequency.pb.h \
        ../LenovoLegion-PrepareBuild/CPUOptions.pb.h  \
        ../LenovoLegion-PrepareBuild/Notification.pb.h \
        ../LenovoLegion-PrepareBuild/CpuIntelMSR.pb.h \
        ../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h \
        ../LenovoLegion-PrepareBuild/ComputerInfo.pb.h \
        ../LenovoLegion-PrepareBuild/DaemonSettings.pb.h \
        ../LenovoLegion-PrepareBuild/Other.pb.h \
        ../LenovoLegion-PrepareBuild/RGBController.pb.h

SOURCES += \
        ../LenovoLegion-Daemon/ProtocolParser.cpp \
        ../LenovoLegion-PrepareBuild/HWMonitoring.pb.cc \
        ../LenovoLegion-PrepareBuild/CPUTopology.pb.cc \
        ../LenovoLegion-PrepareBuild/PowerProfile.pb.cc \
        ../LenovoLegion-PrepareBuild/Battery.pb.cc \
        ../LenovoLegion-PrepareBuild/CpuPower.pb.cc \
        ../LenovoLegion-PrepareBuild/GPUPower.pb.cc \
        ../LenovoLegion-PrepareBuild/FanControl.pb.cc \
        ../LenovoLegion-PrepareBuild/CPUFrequency.pb.cc \
        ../LenovoLegion-PrepareBuild/CPUOptions.pb.cc \
        ../LenovoLegion-PrepareBuild/Notification.pb.cc \
        ../LenovoLegion-PrepareBuild/CpuIntelMSR.pb.cc \
        ../LenovoLegion-PrepareBuild/NvidiaNvml.pb.cc \
        ../LenovoLegion-PrepareBuild/ComputerInfo.pb.cc \
        ../LenovoLegion-PrepareBuild/DaemonSettings.pb.cc \
        ../LenovoLegion-PrepareBuild/Other.pb.cc \
        ../LenovoLegion-PrepareBuild/RGBController.pb.cc

FORMS +=           \
    AboutWindow.ui \
    BateryStatus.ui \
    CPUControl.ui \
    CPUFrequency.ui \
    CPUFrequencyControl.ui \
    FanControl.ui \
    GPUDetails.ui \
    HWMonitor.ui \
    HWMonitoring.ui \
    MainWindow.ui \
    OffsetsControl.ui \
    OtherControl.ui \
    PowerControl.ui \
    PowerProfileControl.ui \
    RGBKeyboardDevice.ui \
    ThreadControl.ui \
    ThreadFrequency.ui \
    ThreadFrequencyControl.ui \
    ToolBarHomeWidget.ui \
    ToolBarHelpWidget.ui \
    ToolBarInformationWidget.ui \
    ToolBarKeyboardWidget.ui \
    ToolBarProfilesWidget.ui \
    ToolBarSettingsWidget.ui

INCLUDEPATH += $${CUDA_PATH}/include
LIBS += -l$${PROJECT_LIBS_NAME}

RESOURCES += \
    Resources.qrc

OTHER_FILES += \
    LenovoLegion.desktop

QMAKE_POST_LINK += $$quote($${SYSTEM_CP} $${PROJECT_ROOT_PATH}/LenovoLegion-Application/LenovoLegionIco.png $${DESTINATION_PATH}$$escape_expand(\\n\\t))
