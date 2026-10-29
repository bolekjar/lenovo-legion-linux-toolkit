#-------------------------------------------------
#
# Daemon module GPUPower - GPU power limits
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleGPUPower

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderGPUPower.h

SOURCES += \
    ModuleGPUPower.cpp \
    SysFsDataProviderGPUPower.cpp
