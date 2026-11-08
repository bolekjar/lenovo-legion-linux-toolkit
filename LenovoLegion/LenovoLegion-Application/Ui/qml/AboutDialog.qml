// SPDX-License-Identifier: GPL-2.0
// About the application (replaces AboutWindow): logo, name, version, description, license, link to the project
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: dialog

    readonly property string projectUrl: "https://github.com/bolekjar/lenovo-legion-linux-toolkit"

    parent: Overlay.overlay
    x: parent !== null ? Math.round((parent.width - width) / 2) : 0
    y: parent !== null ? Math.round((parent.height - height) / 2) : 0
    width: Math.min(480, parent !== null ? parent.width - 40 : 480)
    padding: 24
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    Overlay.modal: Rectangle {
        color: Qt.rgba(0, 0, 0, 0.45)
    }

    background: Rectangle {
        radius: theme.radius
        color: theme.groupBackground
        border.width: theme.styled ? 2 : 1
        border.color: theme.accent
        clip: true

        // Accent light from the top
        Rectangle {
            anchors.fill: parent
            anchors.margins: parent.border.width
            radius: parent.radius
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.22) }
                GradientStop { position: 0.45; color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.0) }
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        LegionLogo {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 84
            Layout.preferredHeight: 84
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: "Lenovo Legion Linux Toolkit"
            font.bold: true
            font.pixelSize: theme.fontPixelSize + 9
            color: theme.text
        }

        // Version
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: versionLabel.implicitWidth + 24
            implicitHeight: versionLabel.implicitHeight + 8
            radius: height / 2
            color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.14)
            border.width: 1
            border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.55)

            Label {
                id: versionLabel

                anchors.centerIn: parent
                text: "Version " + appState.version
                font.bold: true
                color: theme.text
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 4
            text: "Unofficial control application for Lenovo Legion notebooks on Linux: power profiles, power limits, "
                + "fan curve, CPU and GPU settings, voltage offsets and the RGB keyboard. Tested on Legion Pro 7 Gen 9 and Gen 10."
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: theme.text
        }

        Label {
            Layout.fillWidth: true
            text: "This project is not affiliated with Lenovo in any way."
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: theme.disabledText
        }

        Label {
            Layout.fillWidth: true
            text: "© Jaroslav Bolek · GPL-2.0 · This program comes with absolutely no warranty."
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: theme.disabledText
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 8
            spacing: 8

            ThemedButton {
                text: "GitHub"
                onClicked: Qt.openUrlExternally(dialog.projectUrl)
            }

            Item {
                Layout.fillWidth: true
            }

            ThemedButton {
                text: "Close"
                onClicked: dialog.close()
            }
        }
    }
}
