// SPDX-License-Identifier: GPL-2.0
// Text field of the theme (QLineEdit, QLineEdit:focus of the stylesheet)
import QtQuick
import QtQuick.Controls

TextField {
    id: control

    leftPadding: 12
    rightPadding: 12

    color: !enabled ? theme.disabledText : (theme.styled ? theme.accent : theme.text)
    selectionColor: theme.accent
    selectedTextColor: theme.accentText
    placeholderTextColor: theme.disabledText

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 34

        radius: theme.inputRadius
        color: control.enabled ? theme.base : theme.inputDisabledBackground
        border.width: theme.styled ? 2 : 1
        border.color: !control.enabled ? theme.inputDisabledBorder
                    : (control.activeFocus || control.hovered) ? theme.accentHover
                    : (theme.styled ? theme.accent : theme.groupBorder)
    }
}
