// SPDX-License-Identifier: GPL-2.0
// Window of the generic GUI (MainWindow, UiDesign.md): header with the logo (About) and the navigation of the pages,
// the current page, status bar with the daemon (Reconnect), the power profile and the battery. Pages which need the
// daemon are shown only when it is connected (created again after a reconnection, they read the data again); Settings
// and Help work also without it.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

// Control: palette and font of the theme for all pages
Control {
    id: shell

    padding: 0

    background: Rectangle {
        color: theme.window
    }

    // Pages of the navigation: main pages on the left, the others on the right
    readonly property var pages: [
        { name: "Home",        icon: "home",        url: "qrc:/ui/HomePage.qml",        daemon: true,  right: false },
        { name: "Profiles",    icon: "profiles",    url: "qrc:/ui/ProfilesPage.qml",    daemon: true,  right: false },
        { name: "Keyboard",    icon: "keyboard",    url: "qrc:/ui/KeyboardPage.qml",    daemon: true,  right: false },
        { name: "Settings",    icon: "settings",    url: "qrc:/ui/PreferencesPage.qml", daemon: false, right: true },
        { name: "Information", icon: "information", url: "qrc:/ui/InformationPage.qml", daemon: true,  right: true },
        { name: "Help",        icon: "help",        url: "qrc:/ui/HelpPage.qml",        daemon: false, right: true }
    ]

    property int currentPage: 0

    readonly property var  current: pages[currentPage]
    readonly property bool pageShown: !current.daemon || appState.connected

    // Theme of the controls (the same as SettingsPage)
    palette.window:          theme.window
    palette.windowText:      theme.text
    palette.text:            theme.text
    palette.base:            theme.base
    palette.button:          theme.base
    palette.buttonText:      theme.text
    palette.highlight:       theme.accent
    palette.highlightedText: theme.accentText
    font.pixelSize:          theme.fontPixelSize

    Connections {
        target: appState

        function onAboutRequested() {
            about.open()
        }

        // Camera shutter (Legion WMI utility event): the firmware reports only its changes, its state can not be read
        function onDaemonNotification(notification) {
            if (notification.action !== "SPECIAL_KEY_PRESSED")
                return

            if (notification.special_key === "CAMERAON")
                hardwareNotifications.notify("Camera", "Camera shutter is open", "info")
            else if (notification.special_key === "CAMERAOFF")
                hardwareNotifications.notify("Camera", "Camera shutter is closed", "info")
        }
    }

    // Button of the navigation: icon and name, the current page with the light bar of the accent (like the view
    // switcher of the keyboard)
    component NavButton: AbstractButton {
        id: navButton

        property string iconName
        property bool   current: false

        implicitHeight: 40
        implicitWidth: navRow.implicitWidth + 28
        hoverEnabled: true

        background: Rectangle {
            radius: theme.radius
            color: navButton.current ? Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.16)
                 : navButton.hovered ? theme.itemHoverBackground : "transparent"
            border.width: navButton.current ? 1 : 0
            border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.5)

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 3
                width: parent.width - 2 * parent.radius
                height: 3
                radius: 1.5
                visible: navButton.current
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: theme.accent }
                    GradientStop { position: 1.0; color: theme.accentHover }
                }
            }
        }

        contentItem: Item {
            implicitWidth: navRow.implicitWidth
            implicitHeight: navRow.implicitHeight

            Row {
                id: navRow

                anchors.centerIn: parent
                spacing: 8

                TintedIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 18
                    height: 18
                    source: "qrc:/ui/icons/" + navButton.iconName + ".svg"
                    color: navButton.current ? theme.accentHover : navButton.hovered ? theme.itemHoverText : theme.text
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: navButton.text
                    font.bold: navButton.current
                    color: navButton.hovered && !navButton.current ? theme.itemHoverText : theme.text
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header: logo (About), name, navigation
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 60

            gradient: Gradient {
                GradientStop { position: 0.0; color: theme.groupBackground }
                GradientStop { position: 1.0; color: theme.window }
            }

            // Light line of the accent at the bottom
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 2
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: theme.accent }
                    GradientStop { position: 0.5; color: theme.accentHover }
                    GradientStop { position: 1.0; color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.0) }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 10
                anchors.bottomMargin: 2
                spacing: 6

                // Logo and name: About
                AbstractButton {
                    id: brand

                    Layout.rightMargin: 18
                    implicitHeight: 48
                    implicitWidth: brandRow.implicitWidth + 12
                    hoverEnabled: true
                    onClicked: about.open()

                    background: Rectangle {
                        radius: theme.radius
                        color: brand.hovered ? theme.itemHoverBackground : "transparent"
                    }

                    contentItem: Item {
                        implicitWidth: brandRow.implicitWidth
                        implicitHeight: brandRow.implicitHeight

                        Row {
                            id: brandRow

                            anchors.verticalCenter: parent.verticalCenter
                            x: 6
                            spacing: 10

                            LegionLogo {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 36
                                height: 36
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 0

                                Label {
                                    text: "Lenovo Legion"
                                    font.bold: true
                                    font.pixelSize: theme.fontPixelSize + 3
                                    color: brand.hovered ? theme.itemHoverText : theme.text
                                }

                                Label {
                                    text: "Linux Toolkit"
                                    color: brand.hovered ? theme.itemHoverText : theme.disabledText
                                }
                            }
                        }
                    }

                    ThemedToolTip {
                        visible: brand.hovered
                        delay: 600
                        text: "About Lenovo Legion Linux Toolkit"
                    }
                }

                Repeater {
                    model: shell.pages.filter(page => !page.right)

                    delegate: NavButton {
                        required property var modelData

                        text: modelData.name
                        iconName: modelData.icon
                        current: shell.current.name === modelData.name
                        onClicked: shell.currentPage = shell.pages.findIndex(page => page.name === modelData.name)
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Repeater {
                    model: shell.pages.filter(page => page.right)

                    delegate: NavButton {
                        required property var modelData

                        text: modelData.name
                        iconName: modelData.icon
                        current: shell.current.name === modelData.name
                        onClicked: shell.currentPage = shell.pages.findIndex(page => page.name === modelData.name)
                    }
                }

                // Separator and Exit
                Rectangle {
                    Layout.leftMargin: 4
                    Layout.rightMargin: 4
                    implicitWidth: 1
                    implicitHeight: 26
                    color: theme.groupBorder
                }

                NavButton {
                    id: exitButton

                    text: "Exit"
                    iconName: "exit"
                    onClicked: appState.quit()
                }
            }
        }

        // Current page
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Loader {
                id: pageLoader

                anchors.fill: parent
                source: shell.pageShown ? shell.current.url : ""
            }

            // Information about the hardware (camera shutter, onDaemonNotification) over every page
            Notifications {
                id: hardwareNotifications

                z: 100
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 24
                anchors.bottomMargin: 80
                width: 420
                height: parent.height / 2
            }

            // Page which needs the daemon without the connection
            Item {
                anchors.fill: parent
                visible: !shell.pageShown

                Rectangle {
                    anchors.centerIn: parent
                    width: Math.min(560, parent.width - 40)
                    implicitHeight: offlineColumn.implicitHeight + 48
                    radius: theme.radius
                    color: theme.groupBackground
                    border.width: theme.styled ? 2 : 1
                    border.color: theme.groupBorder

                    ColumnLayout {
                        id: offlineColumn

                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 12

                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            implicitWidth: 56
                            implicitHeight: 56
                            radius: 28
                            color: Qt.rgba(0.9, 0.33, 0.29, 0.14)
                            border.width: 2
                            border.color: "#e5534b"

                            TintedIcon {
                                anchors.centerIn: parent
                                width: 26
                                height: 26
                                source: "qrc:/ui/icons/plug.svg"
                                color: "#e5534b"
                            }
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Not connected to the daemon"
                            font.bold: true
                            font.pixelSize: theme.fontPixelSize + 6
                            color: theme.text
                        }

                        Label {
                            Layout.fillWidth: true
                            text: "The page " + shell.current.name + " needs LenovoLegion-Daemon. Check that the service runs "
                                + "(systemctl status lenovo-legion-daemon.service) and that your user is in the group legion."
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                            color: theme.disabledText
                        }

                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 6
                            spacing: 8

                            ThemedButton {
                                text: "Reconnect"
                                onClicked: appState.reconnect()
                            }

                            ThemedButton {
                                text: "Open Help"
                                onClicked: shell.currentPage = shell.pages.findIndex(page => page.name === "Help")
                            }
                        }
                    }
                }
            }
        }

        // Status bar: daemon, power profile, battery, version
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 34
            color: theme.groupBackground

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: theme.groupBorder
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 16

                // Connection to the daemon
                Row {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 8

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 10
                        height: 10
                        radius: 5
                        color: appState.connected ? "#3fb950" : "#e5534b"
                    }

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: appState.connected ? "Daemon connected" : "Daemon disconnected"
                        color: theme.text
                    }

                    Label {
                        id: reconnectLink

                        anchors.verticalCenter: parent.verticalCenter
                        visible: !appState.connected
                        text: "Reconnect"
                        font.bold: true
                        font.underline: reconnectHover.hovered
                        color: theme.accentHover

                        HoverHandler {
                            id: reconnectHover

                            cursorShape: Qt.PointingHandCursor
                        }

                        TapHandler {
                            onTapped: appState.reconnect()
                        }
                    }
                }

                // Power profile (Lenovo Legion driver)
                Row {
                    id: profileState

                    Layout.alignment: Qt.AlignVCenter
                    visible: appState.connected && powerProfile.available && active !== null
                    spacing: 8

                    DataSource {
                        id: powerProfile

                        dataType: "POWER_PROFILE"
                    }

                    readonly property FieldModel profile: powerProfile.field("current_value", powerProfile.revision)
                    readonly property var active: {
                        if (profile === null)
                            return null

                        for (const choice of profile.choices)
                            if (profile.value === choice.value)
                                return choice

                        return null
                    }

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 1
                        height: 16
                        color: theme.groupBorder
                    }

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 10
                        height: 10
                        radius: 3
                        color: profileState.active !== null && profileState.active.color !== undefined ? profileState.active.color : theme.accent
                    }

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Power profile: " + (profileState.active !== null ? profileState.active.text : "")
                        color: theme.text
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                BatteryIndicator {
                    Layout.alignment: Qt.AlignVCenter
                    visible: appState.connected
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter
                    text: "v" + appState.version
                    color: theme.disabledText
                }
            }
        }
    }

    AboutDialog {
        id: about
    }
}
