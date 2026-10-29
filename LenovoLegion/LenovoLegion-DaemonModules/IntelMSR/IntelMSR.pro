#-------------------------------------------------
#
# Daemon module IntelMSR - CPU voltage offsets
# Dynamic module (.so) loaded by the daemon from modules/ (see LenovoLegion-Daemon/ModulesDesign.md)
#
#-------------------------------------------------
TEMPLATE        = lib
TARGET          = DaemonModuleIntelMSR

# plugin: plain .so without version symlinks, hide_symbols: only the entry points are exported
CONFIG          += plugin hide_symbols c++20 link_pkgconfig

QT              = core

PKGCONFIG       += protobuf

DESTDIR         = $${DESTINATION_MODULES_PATH}

# Module may use only the SDK and the messages, not the daemon or BJLibs
# (.qmake.conf adds BJLibs and LenovoLegion-Daemon to INCLUDEPATH of all projects)
INCLUDEPATH     =  \
    $$PWD \
    $$PWD/../../LenovoLegion-DaemonSDK \
    $$PWD/../../LenovoLegion-PrepareBuild

# Shared protocol messages (must not be linked into the module, see LenovoLegion-Messages.pro),
# found by rpath in the directory of the daemon (modules/..)
LIBS            += -L$${DESTINATION_BIN_PATH} -lLenovoLegionMessages
PRE_TARGETDEPS  += $${DESTINATION_BIN_PATH}/libLenovoLegionMessages.so
QMAKE_LFLAGS    += '-Wl,-rpath,\'\$$ORIGIN/..\''

# All symbols must be resolved at link time (symbols of the daemon are not available to modules),
# only really used libraries are recorded as dependencies
QMAKE_LFLAGS    += -Wl,--no-undefined -Wl,--as-needed

HEADERS += \
    ModuleIntelMSR.h

SOURCES += \
    ModuleIntelMSR.cpp
