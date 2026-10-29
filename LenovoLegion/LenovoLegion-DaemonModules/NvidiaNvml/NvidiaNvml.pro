#-------------------------------------------------
#
# Daemon module NvidiaNvml - NVIDIA GPU (NVML)
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleNvidiaNvml

include(../ModuleKit/ModuleKit.pri)

# NVML (libnvidia-ml), only this module depends on it
INCLUDEPATH     += $${CUDA_PATH}/include
LIBS            += -L$${CUDA_PATH}/lib64 -lnvidia-ml

HEADERS += \
    DataProviderNvidiaNvml.h

SOURCES += \
    DataProviderNvidiaNvml.cpp \
    ModuleNvidiaNvml.cpp
