TEMPLATE = app
TARGET = $${APPLICATION_NAME}

QT       += core gui widgets network qml quick quickwidgets quickcontrols2 concurrent

CONFIG += c++20 link_pkgconfig
PKGCONFIG += protobuf

DESTDIR = $${DESTINATION_BIN_PATH}

SOURCES +=               \
        Application.cpp  \
        DataProvider.cpp \
        DataProviderManager.cpp \
        MainWindow.cpp   \
        ProtocolProcessor.cpp \
        ProtocolProcessorBase.cpp \
        ProtocolProcessorNotifier.cpp \
        Settings.cpp \
        main.cpp

HEADERS +=          \
        Application.h \
        DataProvider.h \
        DataProviderManager.h \
        MainWindow.h  \
        ProtocolProcessor.h \
        ProtocolProcessorBase.h \
        ProtocolProcessorNotifier.h \
        Settings.h

HEADERS += \
        ../LenovoLegion-Daemon/ProtocolParser.h

SOURCES += \
        ../LenovoLegion-Daemon/ProtocolParser.cpp

# Protocol messages, shared library (the same as the daemon uses, see LenovoLegion-Messages.pro)
include(../LenovoLegion-Messages/LenovoLegion-Messages.pri)

INCLUDEPATH += $${CUDA_PATH}/include
LIBS += -l$${PROJECT_LIBS_NAME}

RESOURCES += \
    Resources.qrc

OTHER_FILES += \
    LenovoLegion.desktop

QMAKE_POST_LINK += $$quote($${SYSTEM_CP} $${PROJECT_ROOT_PATH}/LenovoLegion-Application/LenovoLegionIco.png $${DESTINATION_PATH}$$escape_expand(\\n\\t))

# Generic GUI (UiDesign.md): window, generic data model and state for QML
SOURCES += \
        Ui/AppSettings.cpp \
        Ui/AppState.cpp \
        Ui/DataSource.cpp \
        Ui/FieldModel.cpp \
        Ui/KeyboardLayout.cpp \
        Ui/PageSources.cpp \
        Ui/Reflection.cpp \
        Ui/Theme.cpp

HEADERS += \
        Ui/AppSettings.h \
        Ui/AppState.h \
        Ui/DataSource.h \
        Ui/FieldModel.h \
        Ui/KeyboardLayout.h \
        Ui/PageSources.h \
        Ui/Reflection.h \
        Ui/Theme.h

RESOURCES += \
        Ui/Ui.qrc

DISTFILES += \
        UiDesign.md
