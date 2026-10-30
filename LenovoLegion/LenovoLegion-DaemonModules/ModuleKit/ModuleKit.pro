#-------------------------------------------------
#
# ModuleKit - common code of daemon modules (static library linked into every module .so)
#
#  - SysFsDriver, SysFsDriverManager, DataProvider, SysFsDataProvider with the same interface
#    as in the daemon, without QObject and BJLibs, connected to IModuleHost,
#  - ProviderModule (IModule made of one provider and its drivers),
#  - sysfs drivers shared by several modules (Drivers/).
#
# Nothing is shared between modules at runtime, every module has its own copy (see ModulesDesign.md).
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleKit

CONFIG          += staticlib hide_symbols c++20 link_pkgconfig

QT              = core

PKGCONFIG       += protobuf

DESTDIR         = $${DESTINATION_LIB_PATH}

# Only the SDK and the messages, not the daemon or BJLibs
INCLUDEPATH     =  \
    $$PWD \
    $$PWD/../../LenovoLegion-DaemonSDK \
    $$PWD/../../LenovoLegion-PrepareBuild

HEADERS += \
    KitException.h \
    KitLog.h \
    KitMessage.h \
    ModuleEntry.h \
    SysFsDriver.h \
    SysFsDriverManager.h \
    DataProvider.h \
    SysFsDataProvider.h \
    ProviderModule.h \
    Drivers/SysFsDriverLegion.h \
    Drivers/SysFsDriverCPU.h \
    Drivers/SysFsDriverCPUAtom.h \
    Drivers/SysFsDriverCPUCore.h \
    Drivers/SysFsDriverCPUInfo.h \
    Drivers/SysFsDriverCPUIntelPState.h \
    Drivers/SysFsDriverCPUXList.h \
    Drivers/SysFsDriverIntelPowercapRapl.h \
    Drivers/SysFsDriverLegionEvents.h \
    Drivers/SysFsDriverLegionFanMode.h \
    Drivers/SysFsDriverLegionGameZone.h \
    Drivers/SysFsDriverLegionHWMon.h \
    Drivers/SysFsDriverLegionMachineInformation.h \
    Drivers/SysFsDriverLegionOther.h \
    Drivers/SysFsDriverPowerSuplyBattery0.h

SOURCES += \
    KitLog.cpp \
    SysFsDriver.cpp \
    SysFsDriverManager.cpp \
    DataProvider.cpp \
    SysFsDataProvider.cpp \
    ProviderModule.cpp \
    Drivers/SysFsDriverCPU.cpp \
    Drivers/SysFsDriverCPUAtom.cpp \
    Drivers/SysFsDriverCPUCore.cpp \
    Drivers/SysFsDriverCPUInfo.cpp \
    Drivers/SysFsDriverCPUIntelPState.cpp \
    Drivers/SysFsDriverCPUXList.cpp \
    Drivers/SysFsDriverIntelPowercapRapl.cpp \
    Drivers/SysFsDriverLegionEvents.cpp \
    Drivers/SysFsDriverLegionFanMode.cpp \
    Drivers/SysFsDriverLegionGameZone.cpp \
    Drivers/SysFsDriverLegionHWMon.cpp \
    Drivers/SysFsDriverLegionMachineInformation.cpp \
    Drivers/SysFsDriverLegionOther.cpp \
    Drivers/SysFsDriverPowerSuplyBattery0.cpp
