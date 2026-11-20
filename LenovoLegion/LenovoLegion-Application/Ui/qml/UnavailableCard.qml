// SPDX-License-Identifier: GPL-2.0
// Card of a content which is not available (daemon not connected, page without data, e.g. no supported keyboard):
// icon in a circle, title, reason and optional buttons (default property), centered in its parent
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card

    property url    icon: "qrc:/ui/icons/plug.svg"
    property string title: "Not available"
    property string text: ""
    // Color of the icon and its circle: an error (red) or a neutral state (the disabled text of the theme)
    property color  iconColor: theme.disabledText

    default property alias buttons: buttonsRow.data

    anchors.centerIn: parent
    width: Math.min(560, parent.width - 40)
    implicitHeight: cardColumn.implicitHeight + 48
    radius: theme.radius
    color: theme.groupBackground
    border.width: theme.styled ? 2 : 1
    border.color: theme.groupBorder

    ColumnLayout {
        id: cardColumn

        anchors.fill: parent
        anchors.margins: 24
        spacing: 12

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: 56
            implicitHeight: 56
            radius: 28
            color: Qt.rgba(card.iconColor.r, card.iconColor.g, card.iconColor.b, 0.14)
            border.width: 2
            border.color: card.iconColor

            TintedIcon {
                anchors.centerIn: parent
                width: 26
                height: 26
                source: card.icon
                color: card.iconColor
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: card.title
            font.bold: true
            font.pixelSize: theme.fontPixelSize + 6
            color: theme.text
        }

        Label {
            Layout.fillWidth: true
            visible: text.length > 0
            text: card.text
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: theme.disabledText
        }

        RowLayout {
            id: buttonsRow

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 6
            visible: children.length > 0
            spacing: 8
        }
    }
}
