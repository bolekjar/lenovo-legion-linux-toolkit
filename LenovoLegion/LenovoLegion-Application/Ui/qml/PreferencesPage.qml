// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (toolbar page "Settings"): appearance (theme cards with a preview), the application
// (appSettings: tray, logging; saved at once to LenovoLegionGui.ini) and the daemon (DAEMON_SETTINGS: apply / save the
// settings, logging; the daemon replaces its settings by the whole message, so all values are always sent), Save
// Current Configuration, Reconnect, Reset to Defaults. Questions in ConfirmDialog, results in Notifications.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    id: page

    title: ""
    changeBar: false
    columnWidth: 560

    DataSource {
        id: daemon

        dataType: "DAEMON_SETTINGS"
    }

    function daemonValue(path) {
        return daemon.available && daemon.value(path, daemon.revision) === true
    }

    // All settings of the daemon with the changes (the daemon replaces its settings by the message)
    function sendDaemon(changes, done) {
        const values = {
            "apply_settings_on_start": daemonValue("apply_settings_on_start"),
            "save_settings_on_exit":   daemonValue("save_settings_on_exit"),
            "debug_logging":           daemonValue("debug_logging"),
            "trace_logging":           daemonValue("trace_logging")
        }

        for (const key in changes)
            values[key] = changes[key]

        if (!daemon.sendCommand(values)) {
            notifications.notify("Daemon", daemon.lastError.length > 0 ? daemon.lastError : "The daemon did not accept the settings")
            daemon.reload()
            return
        }

        if (done !== undefined)
            notifications.notify("Daemon", done, "info")
    }

    // Colors of the theme previews (Ui/Theme), No Theme: the system palette
    SystemPalette {
        id: system

        colorGroup: SystemPalette.Active
    }

    readonly property var themes: [
        { value: 0, name: "No Theme", hint: "Colors of the system",
          window: system.window, group: system.base, text: system.windowText, accent: system.highlight, accentText: system.highlightedText },
        { value: 1, name: "White", hint: "Bright with blue accents",
          window: "#ffffff", group: "#fafafa", text: "#2d2d30", accent: "#007acc", accentText: "#ffffff" },
        { value: 2, name: "Gray", hint: "Classic gray with green accents",
          window: "#808080", group: "#787878", text: "#e0e0e0", accent: "#3c6844", accentText: "#e0e0e0" },
        { value: 3, name: "Dark", hint: "Dark with orange accents",
          window: "#2d2d30", group: "#3a3a3d", text: "#e0e0e0", accent: "#ff8800", accentText: "#ffffff" }
    ]

    // Row with a switch: title, description, the whole row toggles
    component SwitchRow: Rectangle {
        id: row

        property string title
        property string description
        property bool   checked
        signal toggled(bool checked)

        Layout.fillWidth: true
        implicitHeight: rowLayout.implicitHeight + 12
        radius: theme.inputRadius
        color: rowHover.hovered && row.enabled ? theme.itemHoverBackground : "transparent"
        opacity: enabled ? 1 : 0.5

        HoverHandler {
            id: rowHover
        }

        // The whole row toggles (over the switch: one toggle per click)
        MouseArea {
            anchors.fill: parent
            z: 1
            enabled: row.enabled
            cursorShape: Qt.PointingHandCursor
            onClicked: row.toggled(!row.checked)
        }

        RowLayout {
            id: rowLayout

            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 12

            // The texts wrap, they do not widen the row (the switch stays inside of the group)
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 0
                Layout.minimumWidth: 0
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: row.title
                    font.bold: true
                    color: rowHover.hovered && row.enabled ? theme.itemHoverText : theme.text
                    wrapMode: Text.Wrap
                }

                Label {
                    Layout.fillWidth: true
                    visible: text.length > 0
                    text: row.description
                    color: rowHover.hovered && row.enabled ? theme.itemHoverText : theme.disabledText
                    wrapMode: Text.Wrap
                }
            }

            ThemedSwitch {
                Layout.alignment: Qt.AlignVCenter
                checked: row.checked
            }
        }
    }

    // Title of the page
    Rectangle {
        id: hero

        Layout.fillWidth: true
        Layout.columnSpan: page.columns
        implicitHeight: heroLayout.implicitHeight + 36

        radius: theme.radius
        border.width: theme.styled ? 2 : 1
        border.color: theme.groupBorder
        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.groupBackground }
            GradientStop { position: 1.0; color: theme.styled ? theme.window : theme.groupBackground }
        }
        clip: true

        Rectangle {
            anchors.fill: parent
            anchors.margins: parent.border.width
            radius: parent.radius
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.22) }
                GradientStop { position: 0.6; color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.0) }
            }
        }

        Rectangle {
            x: parent.border.width
            y: parent.radius
            width: 4
            height: parent.height - 2 * parent.radius
            radius: 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: theme.accentHover }
                GradientStop { position: 1.0; color: theme.accent }
            }
        }

        RowLayout {
            id: heroLayout

            anchors.fill: parent
            anchors.margins: 18
            anchors.leftMargin: 28
            spacing: 20

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: "Settings"
                    font.pixelSize: theme.fontPixelSize + 12
                    font.bold: true
                    color: theme.text
                }

                Label {
                    Layout.fillWidth: true
                    text: "Appearance and behavior of this application and the configuration of the daemon. Changes are saved at once."
                    color: theme.disabledText
                    wrapMode: Text.Wrap
                }
            }

            // Connection to the daemon
            Rectangle {
                id: connectionState

                readonly property color stateColor: daemon.available ? "#3fb950" : "#e5534b"

                implicitWidth: stateRow.implicitWidth + 24
                implicitHeight: stateRow.implicitHeight + 14
                radius: height / 2
                color: Qt.rgba(stateColor.r, stateColor.g, stateColor.b, 0.14)
                border.width: 1
                border.color: stateColor

                Row {
                    id: stateRow

                    anchors.centerIn: parent
                    spacing: 8

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 10
                        height: 10
                        radius: 5
                        color: connectionState.stateColor
                    }

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: daemon.available ? "Daemon connected" : "Daemon not connected"
                        font.bold: true
                        color: theme.text
                    }
                }
            }
        }
    }

    // Theme cards: a small window in the colors of the theme
    Section {
        Layout.fillWidth: true
        Layout.columnSpan: page.columns
        title: "Appearance"

        GridLayout {
            Layout.fillWidth: true
            columns: Math.max(1, Math.min(4, Math.floor((width + columnSpacing) / (180 + columnSpacing))))
            columnSpacing: 12
            rowSpacing: 12

            Repeater {
                model: page.themes

                delegate: AbstractButton {
                    id: themeCard

                    required property var modelData

                    readonly property bool current: appSettings.theme === modelData.value

                    // The same width of all cards
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    padding: 10
                    hoverEnabled: true
                    onClicked: appSettings.theme = modelData.value

                    background: Rectangle {
                        radius: theme.radius
                        color: themeCard.hovered ? theme.itemHoverBackground : theme.window
                        border.width: themeCard.current ? 3 : 1
                        border.color: themeCard.current ? theme.accentHover : themeCard.hovered ? theme.accent : theme.groupBorder
                    }

                    contentItem: ColumnLayout {
                        id: cardColumn

                        spacing: 8

                        // Preview: window, title tab, group with lines of text, a button
                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 86
                            radius: 6
                            color: themeCard.modelData.window
                            border.width: 1
                            border.color: Qt.rgba(0, 0, 0, 0.25)
                            clip: true

                            Rectangle {
                                x: 10
                                y: 10
                                width: 54
                                height: 12
                                radius: 3
                                color: themeCard.modelData.accent
                            }

                            Rectangle {
                                x: 10
                                y: 22
                                width: parent.width - 20
                                height: parent.height - 32
                                radius: 4
                                color: themeCard.modelData.group
                                border.width: 1
                                border.color: Qt.rgba(0, 0, 0, 0.15)

                                Column {
                                    x: 8
                                    y: 9
                                    spacing: 6

                                    Repeater {
                                        model: [0.7, 0.5, 0.6]

                                        delegate: Rectangle {
                                            required property real modelData

                                            width: (themeCard.availableWidth - 40) * modelData
                                            height: 4
                                            radius: 2
                                            color: themeCard.modelData.text
                                            opacity: 0.6
                                        }
                                    }
                                }

                                Rectangle {
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.margins: 8
                                    width: 36
                                    height: 14
                                    radius: 4
                                    color: themeCard.modelData.accent
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 0

                                Label {
                                    text: themeCard.modelData.name
                                    font.bold: true
                                    color: themeCard.hovered ? theme.itemHoverText : theme.text
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: themeCard.modelData.hint
                                    color: themeCard.hovered ? theme.itemHoverText : theme.disabledText
                                    elide: Text.ElideRight
                                }
                            }

                            // Selected theme
                            Rectangle {
                                Layout.alignment: Qt.AlignTop
                                implicitWidth: 20
                                implicitHeight: 20
                                radius: 10
                                visible: themeCard.current
                                color: theme.accent

                                Text {
                                    anchors.centerIn: parent
                                    text: "✓"
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: theme.accentText
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Application and Daemon: the same width of the columns, not by the length of the texts
    Section {
        Layout.fillWidth: true
        Layout.preferredWidth: 0
        Layout.alignment: Qt.AlignTop
        title: "Application"

        SwitchRow {
            title: "Start minimized"
            description: "Start in the system tray without showing the window."
            checked: appSettings.startMinimized
            onToggled: value => appSettings.startMinimized = value
        }

        SwitchRow {
            title: "Minimize to tray on close"
            description: "Closing the window keeps the application running in the system tray."
            checked: appSettings.minimizeToTray
            onToggled: value => appSettings.minimizeToTray = value
        }

        SwitchRow {
            title: "Debug logging"
            description: "More messages in the log of the application."
            checked: appSettings.debugLogging
            onToggled: value => appSettings.debugLogging = value
        }

        SwitchRow {
            title: "Trace logging"
            description: "All messages in the log of the application (large logs, for bug reports)."
            checked: appSettings.traceLogging
            onToggled: value => appSettings.traceLogging = value
        }
    }

    Section {
        Layout.fillWidth: true
        Layout.preferredWidth: 0
        Layout.alignment: Qt.AlignTop
        title: "Daemon"
        source: daemon

        SwitchRow {
            enabled: daemon.available
            title: "Apply saved settings on start"
            description: "The daemon applies the saved hardware configuration when it starts (after a reboot)."
            checked: page.daemonValue("apply_settings_on_start")
            onToggled: value => page.sendDaemon({ "apply_settings_on_start": value })
        }

        SwitchRow {
            enabled: daemon.available
            title: "Save settings on exit"
            description: "The daemon saves the current hardware configuration when it stops."
            checked: page.daemonValue("save_settings_on_exit")
            onToggled: value => page.sendDaemon({ "save_settings_on_exit": value })
        }

        SwitchRow {
            enabled: daemon.available
            title: "Debug logging"
            description: "More messages in the log of the daemon (journalctl -u lenovo-legion-daemon.service)."
            checked: page.daemonValue("debug_logging")
            onToggled: value => page.sendDaemon({ "debug_logging": value })
        }

        SwitchRow {
            enabled: daemon.available
            title: "Trace logging"
            description: "All messages in the log of the daemon."
            checked: page.daemonValue("trace_logging")
            onToggled: value => page.sendDaemon({ "trace_logging": value })
        }

        // Buttons: next to each other, under each other in a narrow group
        Flow {
            Layout.fillWidth: true
            Layout.topMargin: 6
            spacing: 8

            ThemedButton {
                text: "Save Current Configuration"
                enabled: daemon.available
                onClicked: confirm.ask("Save Configuration",
                                       "Save the current hardware configuration (power profile, power limits, fan curve, CPU and GPU settings) to the configuration of the daemon?",
                                       "Save", false, () => page.sendDaemon({ "save_now": true }, "Current configuration saved"))
            }

            ThemedButton {
                text: "Reconnect"
                onClicked: {
                    notifications.notify("Daemon", "Connecting to the daemon again", "info")
                    appSettings.reconnectDaemon()
                }
            }
        }
    }

    Section {
        Layout.fillWidth: true
        Layout.columnSpan: page.columns
        title: "Reset"

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                Layout.fillWidth: true
                text: "Reset the settings of the application (theme, tray, logging) to their defaults. The configuration of the daemon is not changed."
                color: theme.disabledText
                wrapMode: Text.Wrap
            }

            ThemedButton {
                text: "Reset to Defaults"
                onClicked: confirm.ask("Reset to Defaults", "Reset all settings of the application to their default values?",
                                       "Reset", true, () => {
                    appSettings.resetToDefaults()
                    notifications.notify("Settings", "Settings of the application reset to defaults", "info")
                })
            }
        }
    }

    ConfirmDialog {
        id: confirm
    }

    // Bottom right over the page
    Notifications {
        id: notifications

        parent: page
        z: 100
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 24
        anchors.bottomMargin: 24
        width: 380
        height: parent.height / 2
    }
}
