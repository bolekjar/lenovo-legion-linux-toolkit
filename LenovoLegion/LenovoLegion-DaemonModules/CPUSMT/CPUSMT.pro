#-------------------------------------------------
#
# Daemon module CPUSMT - CPU SMT control
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleCPUSMT

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderCPUSMT.h

SOURCES += \
    ModuleCPUSMT.cpp \
    SysFsDataProviderCPUSMT.cpp
