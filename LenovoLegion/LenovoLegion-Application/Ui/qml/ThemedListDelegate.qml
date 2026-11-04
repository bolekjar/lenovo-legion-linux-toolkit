// SPDX-License-Identifier: GPL-2.0
// Item of a list of the theme (QListWidget::item, :hover, :selected of the stylesheet),
// the list itself: ThemedListBackground
import QtQuick
import QtQuick.Controls

ItemDelegate {
    id: control

    property bool selected: false

    implicitHeight: 36
    leftPadding: 12
    rightPadding: 12

    contentItem: Text {
        text: control.text
        font.pixelSize: theme.fontPixelSize
        color: control.selected ? theme.listSelectedText
             : control.hovered ? theme.listSelectedText : theme.listText
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: 8
        color: control.selected ? theme.listItemSelected
             : control.hovered ? theme.listItemHover : theme.listItemBackground
        border.width: 1
        border.color: (control.selected || control.hovered) && theme.styled ? theme.accent : theme.listItemBorder
    }
}
