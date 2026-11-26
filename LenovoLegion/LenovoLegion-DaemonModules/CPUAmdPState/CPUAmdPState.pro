#-------------------------------------------------
#
# Daemon module CPUAmdPState - AMD P-state (amd-pstate)
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUAmdPState

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUAmdPState.h

SOURCES += \
    ModuleCPUAmdPState.cpp \
    SysFsDataProviderCPUAmdPState.cpp
