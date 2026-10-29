#-------------------------------------------------
#
# Daemon module OtherGpuSwitch - GPU switch
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleOtherGpuSwitch

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderOtherGpuSwitch.h

SOURCES += \
    ModuleOtherGpuSwitch.cpp \
    SysFsDataProviderOtherGpuSwitch.cpp
