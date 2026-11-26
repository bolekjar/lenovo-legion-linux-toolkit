#-------------------------------------------------
#
# Daemon module RGB4Zone - 4-zone RGB keyboard (HID)
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleRGB4Zone

include(../ModuleKit/ModuleKit.pri)

PKGCONFIG       += hidapi-hidraw

HEADERS += \
    DataProviderRGB4Zone.h

SOURCES += \
    DataProviderRGB4Zone.cpp \
    ModuleRGB4Zone.cpp
