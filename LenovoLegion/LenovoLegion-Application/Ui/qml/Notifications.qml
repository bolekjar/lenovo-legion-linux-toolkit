// SPDX-License-Identifier: GPL-2.0
// Notifications at the bottom right over the page: errors (failed requests of the groups, default) and information
// (kind "info", e.g. "Copied"): icon, title, message, hidden after timeout (errors) / infoTimeout ms or by the close
// button; the same message again only restarts its time
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: host

    property int timeout: 10000
    property int infoTimeout: 3000

    function notify(title, message, kind) {
        const info = kind === "info"

        for (let i = 0; i < messages.count; ++i) {
            if (messages.get(i).title === title && messages.get(i).message === message) {
                messages.setProperty(i, "stamp", Date.now())
                return
            }
        }

        messages.append({ title: title, message: message, info: info, stamp: Date.now() })
    }

    ListModel {
        id: messages
    }

    // Removes the expired notifications
    Timer {
        interval: 250
        repeat: true
        running: messages.count > 0
        onTriggered: {
            for (let i = messages.count - 1; i >= 0; --i)
                if (Date.now() - messages.get(i).stamp >= (messages.get(i).info ? host.infoTimeout : host.timeout))
                    messages.remove(i)
        }
    }

    Column {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 8

        Repeater {
            model: messages

            delegate: Rectangle {
                required property int    index
                required property string title
                required property string message
                required property bool   info

                width: Math.min(420, host.width)
                height: content.implicitHeight + 20

                radius: theme.radius
                color: theme.groupBackground
                border.width: 2
                border.color: info ? theme.accentHover : "#d9534f"

                RowLayout {
                    id: content

                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Image {
                        Layout.alignment: Qt.AlignTop
                        source: info ? "qrc:/images/icons/info.png" : "qrc:/images/icons/warning.png"
                        sourceSize: Qt.size(48, 48)
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        fillMode: Image.PreserveAspectFit
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            Layout.fillWidth: true
                            visible: title.length > 0
                            text: title
                            font.bold: true
                            font.pixelSize: theme.fontPixelSize
                            color: theme.text
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: message
                            font.pixelSize: theme.fontPixelSize
                            color: theme.text
                            wrapMode: Text.Wrap
                        }
                    }

                    ToolButton {
                        Layout.alignment: Qt.AlignTop
                        text: "✕"
                        font.pixelSize: theme.fontPixelSize
                        palette.buttonText: theme.text
                        onClicked: messages.remove(index)
                    }
                }
            }
        }
    }
}
