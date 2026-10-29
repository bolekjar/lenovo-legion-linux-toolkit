#-------------------------------------------------
#
# Daemon module CPUInfo - CPU information
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUInfo

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUInfo.h

SOURCES += \
    ModuleCPUInfo.cpp \
    SysFsDataProviderCPUInfo.cpp
