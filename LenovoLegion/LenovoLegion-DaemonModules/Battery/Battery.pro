#-------------------------------------------------
#
# Daemon module Battery - battery
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleBattery

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderBattery.h

SOURCES += \
    ModuleBattery.cpp \
    SysFsDataProviderBattery.cpp
