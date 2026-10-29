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
