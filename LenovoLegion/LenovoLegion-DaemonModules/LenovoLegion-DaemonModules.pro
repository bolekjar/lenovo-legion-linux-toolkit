#-------------------------------------------------
#
# Daemon modules (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE = subdirs

# Common code of modules (static library)
SUBDIRS += \
    ModuleKit

# Modules (.so), every module except IntelMSR is built with ModuleKit
MODULES = \
    MachineInformation \
    CPUInfo \
    Battery \
    HWMon \
    CPUPower \
    GPUPower \
    FanOption \
    FanCurve \
    PowerProfile \
    Other \
    OtherGpuSwitch \
    CPUTopology \
    CPUOptions \
    CPUSMT \
    CPUFrequency \
    CPUPState \
    NvidiaNvml \
    RGBController

SUBDIRS += \
    IntelMSR \
    $${MODULES}

for(module, MODULES) {
    $${module}.depends = ModuleKit
}
