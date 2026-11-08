// SPDX-License-Identifier: GPL-2.0
// Modal question of the theme (replaces QMessageBox Yes/No): title, text, Cancel and the confirm button (red for a
// destructive action). ask() opens it, the callback is called when the action is confirmed.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: dialog

    property string title
    property string text
    property string confirmText: "OK"
    property bool   destructive: false
    property var    callback: null

    function ask(title, text, confirmText, destructive, callback) {
        dialog.title = title
        dialog.text = text
        dialog.confirmText = confirmText
        dialog.destructive = destructive
        dialog.callback = callback
        open()
    }

    readonly property color destructiveColor: "#e5534b"

    parent: Overlay.overlay
    x: parent !== null ? Math.round((parent.width - width) / 2) : 0
    y: parent !== null ? Math.round((parent.height - height) / 2) : 0
    width: Math.min(460, parent !== null ? parent.width - 40 : 460)
    padding: 20
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
        border.color: dialog.destructive ? dialog.destructiveColor : theme.accent
    }

    contentItem: ColumnLayout {
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            Image {
                Layout.alignment: Qt.AlignTop
                source: dialog.destructive ? "qrc:/images/icons/warning.png" : "qrc:/images/icons/dialog.png"
                sourceSize: Qt.size(64, 64)
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                fillMode: Image.PreserveAspectFit
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    text: dialog.title
                    font.bold: true
                    font.pixelSize: theme.fontPixelSize + 3
                    color: theme.text
                    wrapMode: Text.Wrap
                }

                Label {
                    Layout.fillWidth: true
                    text: dialog.text
                    color: theme.text
                    wrapMode: Text.Wrap
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            ThemedButton {
                text: "Cancel"
                onClicked: dialog.close()
            }

            ThemedButton {
                id: confirmButton

                text: dialog.confirmText
                onClicked: {
                    const callback = dialog.callback
                    dialog.close()
                    if (callback !== null)
                        callback()
                }

                // Destructive action: red instead of the accent
                background: Rectangle {
                    implicitWidth: 90
                    implicitHeight: 32
                    radius: theme.radius
                    color: !dialog.destructive ? (confirmButton.down ? theme.accentPressed : theme.accent)
                         : confirmButton.down ? Qt.darker(dialog.destructiveColor, 1.2)
                         : confirmButton.hovered ? Qt.lighter(dialog.destructiveColor, 1.1) : dialog.destructiveColor
                    border.width: theme.styled ? 2 : 1
                    border.color: dialog.destructive ? dialog.destructiveColor : theme.accent
                }

                contentItem: Text {
                    text: confirmButton.text
                    font.bold: true
                    font.pixelSize: theme.fontPixelSize
                    color: dialog.destructive ? "white" : theme.accentText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    onClosed: callback = null
}
