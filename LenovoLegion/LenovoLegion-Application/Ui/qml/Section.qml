// SPDX-License-Identifier: GPL-2.0
// Group of fields of one data source, error of the last request and optional Apply/Cancel
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

GroupBox {
    id: section

    property DataSource source
    property bool       applyBar: false
    property var        presets: []      // buttons of the ApplyBar, see ApplyBar.presets
    property string     presetLabel: "Preset"
    property bool       checkable: false // check box in the title bar (checkable QGroupBox), e.g. opens a detail window
    property bool       checked: false
    property url        titleIcon        // icon before the title (Lenovo Legion: PowerProfilesPanel)

    // Colors of a single group (thread cells: QGroupBox::title { color }, QGroupBox { background-color } of ThreadControl),
    // undefined = theme
    property var        titleColor: undefined
    property var        backgroundColor: undefined

    default property alias content: fields.data

    // SettingsPage with the change bar (Cancel/Apply of the whole page): the group shows only its presets
    property Item       settingsPage: null
    readonly property bool pageApply: settingsPage !== null && settingsPage.changeBar

    // A group of a page in columns has the height of its row (the bottom edges of the groups next to each other match)
    // and the same width as the other columns (not by the width of its content)
    readonly property bool inPageColumns: settingsPage !== null && settingsPage.columns > 1 && parent === settingsPage.grid

    Layout.fillHeight: inPageColumns
    Layout.preferredWidth: inPageColumns ? 1 : -1
    Layout.horizontalStretchFactor: inPageColumns ? 1 : -1

    // The content uses the whole height of a stretched group (power profiles panel, fan curve), otherwise it stays at the top
    property bool       stretchContent: false

    // Host of the notifications (HomePage): errors are shown there, otherwise in the group
    property Item       notificationHost: null

    // Data source not available (daemon not connected, kernel driver / module not loaded): the fields and the
    // presets are hidden, the group shows why (showUnavailable: false keeps the content, the group handles it itself)
    property bool       showUnavailable: true
    readonly property bool unavailable: showUnavailable && source !== null && source.dataType.length > 0 && !source.available

    readonly property string unavailableReason: {
        if (source === null)
            return ""

        if (typeof appState !== "undefined" && appState !== null && !appState.connected)
            return "Not connected to the daemon"

        // Daemon module of the data not installed, not loaded or failed
        if (source.moduleError.length > 0)
            return source.moduleError

        if (source.platform === "lenovo")
            return "Requires the Lenovo Legion kernel driver (lenovo_legion)"

        // Error of the daemon without the name of the module ("IntelMSR: kernel module ... is not loaded")
        const error = source.lastError.replace(/^[A-Za-z]+:\s*/, "")
        return error.length > 0 ? error.charAt(0).toUpperCase() + error.slice(1) : "The daemon provides no data for this group"
    }

    Component.onCompleted: {
        for (let item = parent; item !== null; item = item.parent) {
            if (settingsPage === null && item.isSettingsPage === true)
                settingsPage = item

            if (item.isNotificationHost === true) {
                notificationHost = item
                break
            }
        }
    }

    // Error of the data source shown for errorTimeout ms (every failure shows it again, also the same text)
    property int    errorTimeout: 10000
    property string shownError: ""

    Connections {
        target: section.source
        ignoreUnknownSignals: true

        function onFailed(message) {
            if (section.notificationHost !== null) {
                section.notificationHost.notify(section.title, message)
                return
            }

            section.shownError = message
            errorTimer.restart()
        }
    }

    Timer {
        id: errorTimer

        interval: section.errorTimeout
        onTriggered: section.shownError = ""
    }

    Layout.fillWidth: true

    // Palette of the theme also for groups outside of a SettingsPage (panels next to the tabs): the controls
    // (combo box text, ...) do not take the default palette of the system (white text on a white theme)
    palette.window:            theme.window
    palette.windowText:        theme.text
    palette.text:              theme.text
    palette.base:              theme.base
    palette.alternateBase:     theme.groupBackground
    // Fusion paints switches, combo boxes and spin buttons with the button color, push buttons are ThemedButton
    palette.button:            theme.base
    palette.buttonText:        theme.text
    palette.highlight:         theme.accent
    palette.highlightedText:   theme.accentText
    palette.mid:               theme.groupBorder
    palette.dark:              theme.groupBorder
    palette.disabled.text:       theme.disabledText
    palette.disabled.windowText: theme.disabledText
    palette.disabled.buttonText: theme.disabledText
    palette.disabled.button:     theme.disabledBackground


    // QGroupBox { margin-top: 18px; padding: 8px }: the frame starts 18 px under the top, the title tab over it
    readonly property bool hasTitle: title.length > 0
    readonly property int  frameTop: hasTitle ? 18 : 0

    padding: 8
    topInset: frameTop
    topPadding: (hasTitle ? titleBar.height : 0) + padding

    // Frame of the theme (QGroupBox: border, radius 12 px, vertical gradient of the group background to the window)
    background: Rectangle {
        border.color: theme.groupBorder
        border.width: theme.styled ? 2 : 1
        radius: theme.radius
        color: section.backgroundColor !== undefined ? section.backgroundColor : theme.groupBackground
        gradient: section.backgroundColor !== undefined ? null : groupGradient

        Gradient {
            id: groupGradient

            GradientStop { position: 0.0; color: theme.groupBackground }
            GradientStop { position: 1.0; color: theme.styled ? theme.window : theme.groupBackground }
        }
    }

    // Title tab of the theme (QGroupBox::title: top left, width of the text, padding 6 px 16 px,
    // top corners rounded 10 px, horizontal gradient)
    label: Item {
        id: titleBar

        visible: section.hasTitle
        x: 0
        y: 0
        implicitWidth: titleRow.implicitWidth + 32      // the group is at least as wide as its title
        implicitHeight: titleRow.implicitHeight + 12
        width: implicitWidth
        height: implicitHeight
        clip: true

        // Rounded only at the top: taller rounded rectangle clipped at the bottom
        Rectangle {
            width: parent.width
            height: parent.height + radius
            radius: theme.styled ? 10 : 2
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: theme.titleBackground }
                GradientStop { position: 1.0; color: theme.titleBackgroundEnd }
            }
        }

        Row {
            id: titleRow

            x: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            // Check box of a checkable group (QGroupBox::indicator of the stylesheet)
            Rectangle {
                id: indicator

                visible: section.checkable
                anchors.verticalCenter: parent.verticalCenter
                width: 16
                height: 16
                radius: theme.styled ? 6 : 2
                border.width: theme.styled ? 2 : 1
                border.color: section.checked ? theme.accentHover : (theme.styled ? theme.accent : theme.groupBorder)
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: section.checked ? theme.accent : theme.window }
                    GradientStop { position: 1; color: section.checked ? theme.accentHover : theme.window }
                }
            }

            Image {
                anchors.verticalCenter: parent.verticalCenter
                visible: section.titleIcon.toString().length > 0
                source: section.titleIcon
                width: 16
                height: 16
                sourceSize: Qt.size(32, 32)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Label {
                id: titleText

                anchors.verticalCenter: parent.verticalCenter
                text: section.title
                color: section.titleColor !== undefined ? section.titleColor : theme.titleText
                font.bold: true
                font.letterSpacing: 0.5
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: section.checkable
            cursorShape: Qt.PointingHandCursor
            onClicked: section.checked = !section.checked
        }
    }

    ColumnLayout {
        width: parent.width
        // Whole height of a stretched group (PowerProfilesPanel), the same as the content otherwise
        height: parent.height

        ColumnLayout {
            id: fields

            Layout.fillWidth: true
            Layout.fillHeight: section.stretchContent
            visible: !section.unavailable
        }

        // Not available: icon, "Not available" and the reason
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            visible: section.unavailable
            spacing: 12

            Rectangle {
                Layout.alignment: Qt.AlignTop
                implicitWidth: 32
                implicitHeight: 32
                radius: 16
                color: "transparent"
                border.width: 1.5
                border.color: theme.disabledText

                TintedIcon {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "qrc:/ui/icons/plug.svg"
                    color: theme.disabledText
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: "Not available"
                    font.bold: true
                    color: theme.text
                }

                Label {
                    Layout.fillWidth: true
                    text: section.unavailableReason
                    color: theme.disabledText
                    wrapMode: Text.Wrap
                }
            }
        }

        // Extra height of a group higher than its content (next to a higher group): under the content, the presets
        // and the error at the bottom
        Item {
            Layout.fillHeight: !section.stretchContent
        }

        // Error of the last request, shown for a while (errorTimeout), then hidden.
        // A disabled page without data shows the reason once (SettingsPage), not the error of every section.
        Label {
            Layout.fillWidth: true

            visible: section.shownError.length > 0 && section.source !== null && !section.unavailable && (section.enabled || section.source.available)
            text: section.shownError
            color: "red"
            wrapMode: Text.Wrap
        }

        ApplyBar {
            Layout.fillWidth: true

            visible: section.applyBar && !section.unavailable && (!section.pageApply || section.presets.length > 0)
            source: section.source
            presets: section.presets
            presetLabel: section.presetLabel
            showApply: !section.pageApply
        }
    }
}
