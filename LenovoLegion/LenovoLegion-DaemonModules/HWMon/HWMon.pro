#-------------------------------------------------
#
# Daemon module HWMon - hardware monitoring
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleHWMon

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderHWMon.h

SOURCES += \
    ModuleHWMon.cpp \
    SysFsDataProviderHWMon.cpp
