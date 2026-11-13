#-------------------------------------------------
#
# Daemon module API (SDK), header only draft, nothing is built yet
# See LenovoLegion-Daemon/ModulesDesign.md
#
#-------------------------------------------------
TEMPLATE        = aux

QT += core

CONFIG += c++20

HEADERS += \
    ModuleTypes.h \
    IModuleHost.h \
    IModule.h \
    ModuleApi.h \
    SysFs.h

# RGB key names, shared by the module RGBController and the GUI (Ui/KeyboardLayout.cpp)
HEADERS += \
    RGBControllerKeyNames.h
