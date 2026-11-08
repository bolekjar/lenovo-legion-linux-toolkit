// SPDX-License-Identifier: GPL-2.0
// Color picker of the RGB keyboard page: wheel with the hue around and the
// saturation from the middle, value slider, preview, hex field, swatches. color is the picked color; colorPicked is
// emitted for changes by the user (not for setting color).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: picker

    property color color: "#ff0000"

    signal colorPicked(color color)

    spacing: 16

    // HSV of the picked color, the hue and saturation stay when the value is 0 (black)
    property real hue: 0
    property real saturation: 1
    property real brightness: 1

    function setFromColor(newColor) {
        if (newColor.hsvValue > 0 && newColor.hsvSaturation > 0)
            hue = Math.max(0, newColor.hsvHue)
        if (newColor.hsvValue > 0)
            saturation = newColor.hsvSaturation
        brightness = newColor.hsvValue
        wheel.requestPaint()
    }

    function pick(newColor) {
        color = newColor
        setFromColor(newColor)
        colorPicked(newColor)
    }

    function pickHsv(h, s, v) {
        hue = h
        saturation = s
        brightness = v
        color = Qt.hsva(h, s, v, 1)
        wheel.requestPaint()
        colorPicked(color)
    }

    onColorChanged: if (!Qt.colorEqual(color, Qt.hsva(hue, saturation, brightness, 1))) setFromColor(color)
    Component.onCompleted: setFromColor(color)

    // Hue around, saturation from the middle (brightness of the slider), marker of the picked color
    Item {
        Layout.alignment: Qt.AlignTop
        implicitWidth: 168
        implicitHeight: 168

        Canvas {
            id: wheel

            anchors.fill: parent

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            onAvailableChanged: requestPaint()

            onPaint: {
                if (width <= 0 || height <= 0)
                    return

                const ctx = getContext("2d")
                const radius = width / 2
                ctx.reset()

                // Hue around (counter-clockwise from the right, the same as the marker)
                const hue = ctx.createConicalGradient(radius, radius, 0)
                for (let step = 0; step <= 12; ++step)
                    hue.addColorStop(step / 12, Qt.hsva((step / 12) % 1, 1, 1, 1))

                ctx.beginPath()
                ctx.arc(radius, radius, radius, 0, 2 * Math.PI)
                ctx.closePath()
                ctx.fillStyle = hue
                ctx.fill()

                // Saturation: white in the middle
                const saturation = ctx.createRadialGradient(radius, radius, 0, radius, radius, radius)
                saturation.addColorStop(0, Qt.rgba(1, 1, 1, 1))
                saturation.addColorStop(1, Qt.rgba(1, 1, 1, 0))
                ctx.fillStyle = saturation
                ctx.fill()

                // Value of the slider: darker
                ctx.fillStyle = Qt.rgba(0, 0, 0, 1 - picker.brightness)
                ctx.fill()
            }
        }

        // Marker of the picked hue / saturation
        Rectangle {
            readonly property real angle: picker.hue * 2 * Math.PI
            readonly property real distance: picker.saturation * wheel.width / 2

            x: wheel.width / 2 + Math.cos(angle) * distance - width / 2
            y: wheel.height / 2 - Math.sin(angle) * distance - height / 2
            width: 14
            height: 14
            radius: 7
            color: picker.color
            border.width: 2
            border.color: picker.brightness > 0.6 ? "black" : "white"
        }

        MouseArea {
            anchors.fill: parent
            preventStealing: true

            function pickAt(mouse) {
                const radius = wheel.width / 2
                const dx = mouse.x - radius, dy = mouse.y - radius
                const h = (Math.atan2(-dy, dx) / (2 * Math.PI) + 1) % 1
                const s = Math.min(1, Math.sqrt(dx * dx + dy * dy) / radius)
                picker.pickHsv(h, s, picker.brightness > 0 ? picker.brightness : 1)
            }

            onPressed: mouse => pickAt(mouse)
            onPositionChanged: mouse => pickAt(mouse)
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        spacing: 10

        // Preview and hex (RRGGBB)
        RowLayout {
            spacing: 10

            Rectangle {
                implicitWidth: 48
                implicitHeight: 32
                radius: theme.radius / 2
                color: picker.color
                border.width: 1
                border.color: theme.groupBorder
            }

            Label {
                text: "#"
            }

            ThemedTextField {
                id: hex

                implicitWidth: 90
                text: picker.color.toString().substring(1).toUpperCase()
                maximumLength: 6
                validator: RegularExpressionValidator { regularExpression: /[0-9A-Fa-f]{0,6}/ }
                onEditingFinished: {
                    if (text.length === 6)
                        picker.pick(Qt.color("#" + text))
                    text = Qt.binding(() => picker.color.toString().substring(1).toUpperCase())
                }
            }
        }

        // Value (brightness) of the color
        RowLayout {
            spacing: 8

            Label {
                text: "Value"
            }

            ThemedSlider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: Math.round(picker.brightness * 100)
                onMoved: picker.pickHsv(picker.hue, picker.saturation, value / 100)
            }
        }

        // Swatches of the basic colors
        Flow {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: ["#ff0000", "#ff8000", "#ffff00", "#00ff00", "#00ffff", "#0000ff", "#8000ff", "#ff00ff", "#ffffff"]

                delegate: Rectangle {
                    required property string modelData

                    width: 24
                    height: 24
                    radius: 12
                    color: modelData
                    border.width: Qt.colorEqual(picker.color, modelData) ? 3 : 1
                    border.color: Qt.colorEqual(picker.color, modelData) ? theme.accentHover : theme.groupBorder

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: picker.pick(Qt.color(parent.modelData))
                    }
                }
            }
        }
    }
}
