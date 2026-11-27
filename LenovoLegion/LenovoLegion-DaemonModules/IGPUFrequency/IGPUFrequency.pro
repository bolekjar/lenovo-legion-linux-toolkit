#-------------------------------------------------
#
# Daemon module IGPUFrequency - frequency of the Intel integrated GPU
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleIGPUFrequency

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderIGPUFrequency.h

SOURCES += \
    ModuleIGPUFrequency.cpp \
    SysFsDataProviderIGPUFrequency.cpp
