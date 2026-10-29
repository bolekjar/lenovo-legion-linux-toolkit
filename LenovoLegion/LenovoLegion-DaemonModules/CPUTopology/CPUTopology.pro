#-------------------------------------------------
#
# Daemon module CPUTopology - CPU topology
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUTopology

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUTopology.h

SOURCES += \
    ModuleCPUTopology.cpp \
    SysFsDataProviderCPUTopology.cpp
