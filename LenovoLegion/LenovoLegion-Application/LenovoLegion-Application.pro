TEMPLATE = app
TARGET = $${APPLICATION_NAME}

QT       += core gui widgets network qml quick quickwidgets quickcontrols2 concurrent

CONFIG += c++20 link_pkgconfig
PKGCONFIG += protobuf

DESTDIR = $${DESTINATION_BIN_PATH}

SOURCES +=               \
        AboutWindow.cpp \
        Application.cpp  \
        BateryStatus.cpp \
        DataProvider.cpp \
        DataProviderManager.cpp \
        MainWindow.cpp   \
        ProtocolProcessor.cpp \
        ProtocolProcessorBase.cpp \
        ProtocolProcessorNotifier.cpp \
        RGBController.cpp \
        RGBKeyboardDevice.cpp \
        Settings.cpp \
        TaskList.cpp \
        ToolBarHelpWidget.cpp \
        ToolBarInformationWidget.cpp \
        ToolBarKeyboardWidget.cpp \
        ToolBarProfilesWidget.cpp \
        ToolBarQmlPageWidget.cpp \
        ToolBarSettingsWidget.cpp \
        ToolBarWidget.cpp \
        Utils.cpp \
        QTooltipedSlider.cpp \
        ColorWheel.cpp \
        DeviceView.cpp \
        Swatches.cpp \
        main.cpp

HEADERS +=          \
        AboutWindow.h \
        Application.h \
        BateryStatus.h \
        DataProvider.h \
        DataProviderManager.h \
        MainWindow.h  \
        ProtocolProcessor.h \
        ProtocolProcessorBase.h \
        ProtocolProcessorNotifier.h \
        RGBController.h \
        RGBKeyboardDevice.h \
        Settings.h \
        TaskList.h \
        ToolBarHelpWidget.h \
        ToolBarInformationWidget.h \
        ToolBarKeyboardWidget.h \
        ToolBarProfilesWidget.h \
        ToolBarQmlPageWidget.h \
        ToolBarSettingsWidget.h \
        ToolBarWidget.h \
        Utils.h \
        QTooltipedSlider.h \
        ColorWheel.h \
        DeviceView.h \
        WidgetMessage.h \
        Swatches.h

HEADERS += \
        ../LenovoLegion-Daemon/ProtocolParser.h

SOURCES += \
        ../LenovoLegion-Daemon/ProtocolParser.cpp

# Protocol messages, shared library (the same as the daemon uses, see LenovoLegion-Messages.pro)
include(../LenovoLegion-Messages/LenovoLegion-Messages.pri)

FORMS +=           \
    AboutWindow.ui \
    BateryStatus.ui \
    MainWindow.ui \
    RGBKeyboardDevice.ui \
    ToolBarHelpWidget.ui \
    ToolBarInformationWidget.ui \
    ToolBarKeyboardWidget.ui \
    ToolBarProfilesWidget.ui \
    ToolBarSettingsWidget.ui

INCLUDEPATH += $${CUDA_PATH}/include
LIBS += -l$${PROJECT_LIBS_NAME}

RESOURCES += \
    Resources.qrc

OTHER_FILES += \
    LenovoLegion.desktop

QMAKE_POST_LINK += $$quote($${SYSTEM_CP} $${PROJECT_ROOT_PATH}/LenovoLegion-Application/LenovoLegionIco.png $${DESTINATION_PATH}$$escape_expand(\\n\\t))

# Generic GUI (UiDesign.md): generic data model for QML pages
SOURCES += \
        Ui/DataSource.cpp \
        Ui/FieldModel.cpp \
        Ui/KeyboardLayout.cpp \
        Ui/PageSources.cpp \
        Ui/Reflection.cpp \
        Ui/Theme.cpp

HEADERS += \
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
