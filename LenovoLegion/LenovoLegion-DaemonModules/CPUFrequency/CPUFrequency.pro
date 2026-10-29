#-------------------------------------------------
#
# Daemon module CPUFrequency - CPU frequency limits
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUFrequency

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUFrequency.h

SOURCES += \
    ModuleCPUFrequency.cpp \
    SysFsDataProviderCPUFrequency.cpp
