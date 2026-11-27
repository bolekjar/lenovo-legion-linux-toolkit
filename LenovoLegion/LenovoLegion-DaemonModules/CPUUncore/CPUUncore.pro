#-------------------------------------------------
#
# Daemon module CPUUncore - Intel uncore frequency
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUUncore

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUUncore.h

SOURCES += \
    ModuleCPUUncore.cpp \
    SysFsDataProviderCPUUncore.cpp
