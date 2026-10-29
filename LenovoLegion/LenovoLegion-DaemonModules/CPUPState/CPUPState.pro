#-------------------------------------------------
#
# Daemon module CPUPState - Intel P-state
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUPState

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUPState.h

SOURCES += \
    ModuleCPUPState.cpp \
    SysFsDataProviderCPUPState.cpp
