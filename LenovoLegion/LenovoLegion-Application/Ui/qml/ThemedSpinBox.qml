// SPDX-License-Identifier: GPL-2.0
// Spin box of the theme (QSpinBox, QSpinBox:hover/focus/disabled of the stylesheet)
import QtQuick
import QtQuick.Controls

SpinBox {
    id: spin

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 28

        radius: theme.inputRadius
        color: spin.enabled ? theme.base : theme.inputDisabledBackground
        border.width: theme.styled ? 2 : 1
        border.color: !spin.enabled ? theme.inputDisabledBorder
                    : (spin.hovered || spin.activeFocus) ? theme.accentHover
                    : (theme.styled ? theme.accent : theme.groupBorder)
    }
}
