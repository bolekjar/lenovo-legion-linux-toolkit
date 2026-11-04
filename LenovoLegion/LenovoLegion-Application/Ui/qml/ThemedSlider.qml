// SPDX-License-Identifier: GPL-2.0
// Slider of the theme (QSlider::groove, ::sub-page, ::add-page, ::handle of the stylesheet), horizontal and vertical
import QtQuick
import QtQuick.Controls

Slider {
    id: control

    background: Rectangle {
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)

        implicitWidth: control.horizontal ? 200 : 6
        implicitHeight: control.horizontal ? 6 : 200
        width: control.horizontal ? control.availableWidth : implicitWidth
        height: control.horizontal ? implicitHeight : control.availableHeight

        radius: 3
        color: control.enabled ? theme.sliderGroove : theme.inputDisabledBackground
        border.width: 1
        border.color: control.enabled ? theme.sliderGroove : theme.inputDisabledBorder

        // Filled part from the minimum to the handle
        Rectangle {
            x: 0
            y: control.horizontal ? 0 : control.visualPosition * parent.height
            width: control.horizontal ? control.visualPosition * parent.width : parent.width
            height: control.horizontal ? parent.height : parent.height - y

            radius: 3
            // Disabled filled part: QSlider::add-page:vertical:disabled / sub-page:horizontal:disabled of the stylesheets
            color: control.enabled ? theme.accent : (theme.styled ? theme.inputDisabledBorder : theme.disabledBackground)

            gradient: control.enabled && theme.styled ? fill : null

            Gradient {
                id: fill

                orientation: control.horizontal ? Gradient.Horizontal : Gradient.Vertical
                GradientStop { position: 0.0; color: theme.accent }
                GradientStop { position: 1.0; color: theme.accentHover }
            }
        }
    }

    handle: Rectangle {
        x: control.leftPadding + (control.horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))

        implicitWidth: 22
        implicitHeight: 22
        radius: width / 2

        // Disabled handle: light gray in all stylesheets (QSlider::handle:disabled, rgb(224, 224, 224))
        color: control.enabled ? theme.sliderHandle : (theme.styled ? "#e0e0e0" : theme.disabledBackground)
        border.width: 2
        border.color: !control.enabled ? theme.inputDisabledBorder
                    : (control.hovered || control.pressed || control.visualFocus) ? theme.accentHover : theme.accent
    }
}
