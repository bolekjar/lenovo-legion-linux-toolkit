#-------------------------------------------------
#
# Daemon module MachineInformation - machine information (DMI), notifications of the Legion kernel module
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleMachineInformation

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderMachineInformation.h

SOURCES += \
    ModuleMachineInformation.cpp \
    SysFsDataProviderMachineInformation.cpp
