#-------------------------------------------------
#
# Daemon module GPURuntimePM - runtime power management of the discrete NVIDIA GPU
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleGPURuntimePM

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderGPURuntimePM.h

SOURCES += \
    ModuleGPURuntimePM.cpp \
    SysFsDataProviderGPURuntimePM.cpp
