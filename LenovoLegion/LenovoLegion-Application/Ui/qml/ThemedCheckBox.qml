// SPDX-License-Identifier: GPL-2.0
// Check box of the theme (QCheckBox, QCheckBox::indicator, :hover, :checked of the stylesheet)
import QtQuick
import QtQuick.Controls

CheckBox {
    id: control

    // Text color, e.g. changed value not applied yet
    property color textColor: enabled ? theme.text : theme.disabledText

    indicator: Rectangle {
        implicitWidth: 20
        implicitHeight: 20

        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        radius: theme.styled ? 6 : 2
        color: !control.enabled ? theme.inputDisabledBackground
             : control.checked ? theme.accent : theme.base
        border.width: theme.styled ? 2 : 1
        border.color: !control.enabled ? theme.inputDisabledBorder
                    : control.checked ? (control.hovered ? theme.accentPressed : theme.accentHover)
                    : control.hovered ? theme.accentHover
                    : (theme.styled ? theme.accent : theme.groupBorder)

        Text {
            anchors.centerIn: parent
            visible: control.checkState !== Qt.Unchecked
            text: control.checkState === Qt.PartiallyChecked ? "–" : "✓"
            font.bold: true
            font.pixelSize: 14
            color: control.enabled ? theme.accentText : theme.disabledText
        }
    }

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        text: control.text
        font: control.font
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
    }
}
