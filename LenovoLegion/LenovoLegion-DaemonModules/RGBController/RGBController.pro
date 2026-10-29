#-------------------------------------------------
#
# Daemon module RGBController - RGB keyboard (HID)
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleRGBController

include(../ModuleKit/ModuleKit.pri)

# hidapi, only this module depends on it
PKGCONFIG       += hidapi-hidraw

HEADERS += \
    DataProviderRGBController.h \
    RGBControlers/LenovoRGBControllerC197.h \
    RGBControlers/LenovoRGBControllerC9xx.h \
    RGBControlers/LenovoRGBController.h \
    RGBControlers/LenovoUSBControllerC9xx.h \
    RGBControlers/LenovoUSBController.h \
    RGBControllerDetector.h \
    RGBController.h \
    RGBControllerInterface.h \
    RGBControllerKeyNames.h \
    StringUtils.h

SOURCES += \
    DataProviderRGBController.cpp \
    ModuleRGBController.cpp \
    RGBControlers/LenovoRGBControllerC197.cpp \
    RGBControlers/LenovoRGBControllerC9xx.cpp \
    RGBControlers/LenovoRGBController.cpp \
    RGBControlers/LenovoUSBControllerC9xx.cpp \
    RGBControlers/LenovoUSBController.cpp \
    RGBControlers/LenovoUSBControllerDetect.cpp \
    RGBController.cpp \
    StringUtils.cpp
