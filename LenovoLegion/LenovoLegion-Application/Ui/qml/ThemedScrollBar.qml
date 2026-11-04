// SPDX-License-Identifier: GPL-2.0
// Scroll bar of the theme (QScrollBar, ::handle, :hover, :pressed of the stylesheet)
import QtQuick
import QtQuick.Controls

ScrollBar {
    id: control

    padding: 2
    minimumSize: horizontal ? 30 / Math.max(width,1) : 30 / Math.max(height,1)

    contentItem: Rectangle {
        implicitWidth: control.horizontal ? 30 : 14
        implicitHeight: control.horizontal ? 14 : 30

        radius: 7
        color: control.pressed ? theme.accentPressed
             : control.hovered ? theme.accentHover : theme.accent
        opacity: control.policy === ScrollBar.AlwaysOn || control.size < 1.0 ? 1.0 : 0.0
    }

    background: Rectangle {
        implicitWidth: control.horizontal ? 100 : 18
        implicitHeight: control.horizontal ? 18 : 100

        radius: 9
        color: theme.trackBackground
        visible: control.size < 1.0
    }
}
