#-------------------------------------------------
#
# Daemon module FanCurve - fan curve
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleFanCurve

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderFanCurve.h

SOURCES += \
    ModuleFanCurve.cpp \
    SysFsDataProviderFanCurve.cpp
