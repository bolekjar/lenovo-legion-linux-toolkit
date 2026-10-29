#-------------------------------------------------
#
# Daemon module Other - other settings (touchpad, Win key)
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleOther

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderOther.h

SOURCES += \
    ModuleOther.cpp \
    SysFsDataProviderOther.cpp
