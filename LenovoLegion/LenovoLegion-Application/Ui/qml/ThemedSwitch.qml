// SPDX-License-Identifier: GPL-2.0
// Switch of the theme (QCheckBox::indicator, :hover, :checked of the stylesheet)
import QtQuick
import QtQuick.Controls

Switch {
    id: control

    indicator: Rectangle {
        implicitWidth: 42
        implicitHeight: 22

        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        radius: theme.styled ? 6 : height / 2
        color: !control.enabled ? theme.inputDisabledBackground
             : control.checked ? theme.accent : theme.base
        border.width: theme.styled ? 2 : 1
        border.color: !control.enabled ? theme.inputDisabledBorder
                    : (control.checked || control.hovered || control.visualFocus) ? theme.accentHover
                    : (theme.styled ? theme.accent : theme.groupBorder)

        // Handle
        Rectangle {
            x: control.checked ? parent.width - width - 4 : 4
            y: 4

            width: parent.height - 8
            height: width
            radius: theme.styled ? 3 : width / 2
            color: !control.enabled ? theme.disabledText
                 : control.checked ? theme.accentText
                 : (theme.styled ? theme.accent : theme.text)

            Behavior on x {
                NumberAnimation { duration: 100 }
            }
        }
    }

    // No text, the content has the size of the indicator, so the whole indicator is inside the control and reacts to the mouse
    contentItem: Item {
        implicitWidth: control.indicator.implicitWidth
        implicitHeight: control.indicator.implicitHeight
    }
}
