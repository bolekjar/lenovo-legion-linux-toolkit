// SPDX-License-Identifier: GPL-2.0
// Radio button of the theme (QRadioButton::indicator, :checked of the stylesheet)
import QtQuick
import QtQuick.Controls

RadioButton {
    id: control

    // Text color, e.g. the color of a power profile ((legion.ui.value_color))
    property color textColor: theme.text

    indicator: Rectangle {
        implicitWidth: 20
        implicitHeight: 20

        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        radius: width / 2
        color: control.enabled ? theme.base : theme.inputDisabledBackground
        border.width: theme.styled ? 2 : 1
        border.color: !control.enabled ? theme.inputDisabledBorder
                    : (control.checked || control.hovered) ? theme.accentHover
                    : (theme.styled ? theme.accent : theme.groupBorder)

        // Checked: accent dot with the lighter center
        Rectangle {
            anchors.centerIn: parent
            visible: control.checked
            width: parent.width - 8
            height: width
            radius: width / 2

            gradient: Gradient {
                GradientStop { position: 0.0; color: control.enabled ? theme.accentHover : theme.disabledText }
                GradientStop { position: 1.0; color: control.enabled ? theme.accent : theme.disabledText }
            }
        }
    }

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        text: control.text
        font: control.font
        color: control.enabled ? control.textColor : theme.disabledText
        verticalAlignment: Text.AlignVCenter
    }
}
