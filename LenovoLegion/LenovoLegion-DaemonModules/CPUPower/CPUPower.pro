#-------------------------------------------------
#
# Daemon module CPUPower - CPU power limits
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUPower

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUPower.h

SOURCES += \
    ModuleCPUPower.cpp \
    SysFsDataProviderCPUPower.cpp
