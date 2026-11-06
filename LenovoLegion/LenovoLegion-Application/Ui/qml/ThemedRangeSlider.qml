// SPDX-License-Identifier: GPL-2.0
// Range slider of the theme (two handles, the same look as ThemedSlider: QSlider::groove, ::sub-page, ::handle)
import QtQuick
import QtQuick.Controls

RangeSlider {
    id: control

    component ThemedHandle: Rectangle {
        required property var node

        implicitWidth: 20
        implicitHeight: 20
        radius: width / 2

        // Disabled handle: light gray in all stylesheets (QSlider::handle:disabled)
        color: control.enabled ? theme.sliderHandle : (theme.styled ? "#e0e0e0" : theme.disabledBackground)
        border.width: 2
        border.color: !control.enabled ? theme.inputDisabledBorder
                    : (node.hovered || node.pressed) ? theme.accentHover : theme.accent
    }

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        implicitWidth: 200
        implicitHeight: 6
        width: control.availableWidth
        height: implicitHeight

        radius: 3
        color: control.enabled ? theme.sliderGroove : theme.inputDisabledBackground
        border.width: 1
        border.color: control.enabled ? theme.sliderGroove : theme.inputDisabledBorder

        // Filled part between the handles
        Rectangle {
            x: control.first.visualPosition * parent.width
            width: (control.second.visualPosition - control.first.visualPosition) * parent.width
            height: parent.height

            radius: 3
            color: control.enabled ? theme.accent : (theme.styled ? theme.inputDisabledBorder : theme.disabledBackground)
            gradient: control.enabled && theme.styled ? fill : null

            Gradient {
                id: fill

                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: theme.accent }
                GradientStop { position: 1.0; color: theme.accentHover }
            }
        }
    }

    first.handle: ThemedHandle {
        node: control.first
        x: control.leftPadding + control.first.visualPosition * (control.availableWidth - width)
        y: control.topPadding + (control.availableHeight - height) / 2
    }

    second.handle: ThemedHandle {
        node: control.second
        x: control.leftPadding + control.second.visualPosition * (control.availableWidth - width)
        y: control.topPadding + (control.availableHeight - height) / 2
    }
}
