#-------------------------------------------------
#
# Daemon module PCIeASPM - PCIe ASPM policy of the kernel
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModulePCIeASPM

include(../ModuleKit/ModuleKit.pri)

HEADERS += \
    SysFsDataProviderPCIeASPM.h

SOURCES += \
    ModulePCIeASPM.cpp \
    SysFsDataProviderPCIeASPM.cpp
