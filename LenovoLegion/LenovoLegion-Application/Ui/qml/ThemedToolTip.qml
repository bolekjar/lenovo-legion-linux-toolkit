// SPDX-License-Identifier: GPL-2.0
// Tool tip of the theme (QToolTip of the stylesheet)
import QtQuick
import QtQuick.Controls

ToolTip {
    id: control

    leftPadding: 14
    rightPadding: 14
    topPadding: 8
    bottomPadding: 8

    contentItem: Text {
        text: control.text
        font.pixelSize: theme.fontPixelSize
        color: theme.accentText
        wrapMode: Text.Wrap
    }

    background: Rectangle {
        radius: 8
        border.width: 2
        border.color: theme.accentHover

        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.accent }
            GradientStop { position: 1.0; color: theme.accentPressed }
        }
    }
}
