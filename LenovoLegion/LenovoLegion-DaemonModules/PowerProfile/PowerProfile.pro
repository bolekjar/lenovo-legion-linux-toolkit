#-------------------------------------------------
#
# Daemon module PowerProfile - power profile
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModulePowerProfile

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderPowerProfile.h

SOURCES += \
    ModulePowerProfile.cpp \
    SysFsDataProviderPowerProfile.cpp
