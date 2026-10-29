#-------------------------------------------------
#
# Daemon module FanOption - fan options
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleFanOption

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderFanOption.h

SOURCES += \
    ModuleFanOption.cpp \
    SysFsDataProviderFanOption.cpp
