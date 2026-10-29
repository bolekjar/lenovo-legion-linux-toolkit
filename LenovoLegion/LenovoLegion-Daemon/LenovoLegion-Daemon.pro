TEMPLATE = app
TARGET = $${APPLICATION_NAME2}

QT       += core network

CONFIG += c++20 cmdline link_pkgconfig

PKGCONFIG += protobuf

DESTDIR = $${DESTINATION_BIN_PATH}

# Main Framework sources
SOURCES +=  \
        Application.cpp \
        DaemonSettingsManager.cpp \
        DataProvider.cpp \
        DataProviderManager.cpp \
        ProtocolParser.cpp \
        ProtocolProcessor.cpp \
        ProtocolProcessorBase.cpp \
        ProtocolProcessorNotifier.cpp \
        SysFsDriverManager.cpp \
        Settings.cpp \
        main.cpp

# Data providers of the framework
SOURCES += \
        DataProviders/DataProviderDaemonSettings.cpp

# Daemon modules framework sources
SOURCES +=  \
        Modules/ModuleDataProviderAdapter.cpp \
        Modules/ModuleHost.cpp \
        Modules/ModuleLoader.cpp \
        Modules/ModuleSettingsStorage.cpp

#Main Framework headers
HEADERS += \
        Application.h \
        DaemonSettingsManager.h \
        DataProvider.h \
        DataProviderManager.h \
        Message.h \
        ProtocolParser.h \
        ProtocolProcessor.h \
        ProtocolProcessorBase.h \
        ProtocolProcessorNotifier.h \
        Settings.h \
        SysFsDriverManager.h

# Data providers of the framework
HEADERS += \
        DataProviders/DataProviderDaemonSettings.h

HEADERS +=  \
        Modules/ModuleDataProviderAdapter.h \
        Modules/ModuleHost.h \
        Modules/ModuleLoader.h \
        Modules/ModuleSettingsStorage.h

# RGB data model shared with the GUI (the RGB module has its own copy)
HEADERS += \
        RGBControllerInterface.h \
        RGBControllerKeyNames.h

# Daemon module API (SDK) and messages used by modules,
# hardware is handled by modules (.so) loaded at runtime from modules/ (LenovoLegion-DaemonModules)
INCLUDEPATH += $$PWD/../LenovoLegion-DaemonSDK \
               $$PWD/../LenovoLegion-PrepareBuild

# Protocol messages, shared library (must be linked only once per process, see LenovoLegion-Messages.pro)
include(../LenovoLegion-Messages/LenovoLegion-Messages.pri)

LIBS += -l$${PROJECT_LIBS_NAME} -ludev

DISTFILES +=  \
    lenovo-legion-daemon.service \
    lenovo-legion.sysusers \
    lenovo-legion-setup.sh \
    ModulesDesign.md
