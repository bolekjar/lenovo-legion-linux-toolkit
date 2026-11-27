#-------------------------------------------------
#
# Daemon module CPUTcc - Intel TCC offset
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUTcc

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUTcc.h

SOURCES += \
    ModuleCPUTcc.cpp \
    SysFsDataProviderCPUTcc.cpp
