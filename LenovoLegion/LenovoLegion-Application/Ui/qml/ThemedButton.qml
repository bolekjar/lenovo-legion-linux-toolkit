// SPDX-License-Identifier: GPL-2.0
// Button of the theme (QPushButton, QPushButton:hover, QPushButton:disabled of the stylesheet)
import QtQuick
import QtQuick.Controls

Button {
    id: button

    background: Rectangle {
        implicitWidth: 90
        implicitHeight: 32

        radius: theme.radius
        // QPushButton:pressed, :checked, :hover of the stylesheet
        color: !button.enabled ? theme.disabledBackground
             : button.down ? theme.accentPressed
             : button.checked ? theme.buttonChecked
             : button.hovered ? theme.accent : theme.buttonBackground
        border.width: theme.styled ? 2 : 1
        border.color: !button.enabled ? theme.disabledBackground
                    : button.checked ? theme.buttonChecked : theme.accent
    }

    contentItem: Text {
        text: button.text
        font.bold: theme.styled
        font.pixelSize: theme.fontPixelSize
        color: !button.enabled ? theme.disabledText
             : (button.hovered || button.down || button.checked) ? theme.accentText : theme.buttonText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
