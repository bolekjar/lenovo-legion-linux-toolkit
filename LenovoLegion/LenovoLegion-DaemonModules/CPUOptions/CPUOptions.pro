#-------------------------------------------------
#
# Daemon module CPUOptions - CPU options (online, governor)
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUOptions

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUOptions.h

SOURCES += \
    ModuleCPUOptions.cpp \
    SysFsDataProviderCPUOptions.cpp
