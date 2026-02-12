TEMPLATE        = lib
TARGET          = __prepare-build_none__


system($${SYSTEM_PROTOC} -I=$${PROJECT_ROOT_PATH}/LenovoLegion-PrepareBuild/ --cpp_out=$${PROJECT_ROOT_PATH}/LenovoLegion-PrepareBuild $${PROJECT_ROOT_PATH}/LenovoLegion-PrepareBuild/HWMonitoring.proto)

QMAKE_POST_LINK += $$quote($${SYSTEM_MKDIR} -p $${DESTINATION_PATH}$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote($${SYSTEM_MKDIR} -p $${DESTINATION_DATA_PATH}$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote($${SYSTEM_MKDIR} -p $${DESTINATION_BIN_PATH}$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote($${SYSTEM_MKDIR} -p $${DESTINATION_LIB_PATH}$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote($${SYSTEM_MKDIR} -p $${DESTINATION_LOG_PATH}$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote($${SYSTEM_MKDIR} -p $${DESTINATION_MODULES_PATH}$$escape_expand(\\n\\t))

DISTFILES += \
    Battery.proto \
    CPUFrequency.proto \
    CPUOptions.proto \
    CPUTopology.proto \
    ComputerInfo.proto \
    CpuIntelMSR.proto \
    CpuPower.proto \
    FanControl.proto \
    GPUPower.proto \
    HWMonitoring.proto \
    Notification.proto \
    NvidiaNvml.proto \
    Other.proto \
    PowerProfile.proto \
    DaemonSettings.proto \
    RGBController.proto

for (PFILE, DISTFILES) {
    system($${SYSTEM_PROTOC} -I=$${PROJECT_ROOT_PATH}/LenovoLegion-PrepareBuild/ --cpp_out=$${PROJECT_ROOT_PATH}/LenovoLegion-PrepareBuild $${PROJECT_ROOT_PATH}/LenovoLegion-PrepareBuild/$${PFILE})
}
