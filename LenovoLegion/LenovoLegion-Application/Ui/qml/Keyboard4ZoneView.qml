// SPDX-License-Identifier: GPL-2.0
// 4-zone RGB keyboard drawn the same way as the per-key keyboard (KeyboardView: stage, deck, key caps), without labels:
// an illustrative layout (the main block and the numeric keypad), the 4 zones as on Legion 5 15IAH7H (reported by a
// user; the keys may differ by the model, the hardware decides them), other counts divided evenly. A click on a key
// selects its zone (only when the zones can be edited), the selected zone has an outline, the others are dimmed.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import LenovoLegion.Ui 1.0

Item {
    id: view

    property int  zoneCount: 4
    property var  zoneColors: []        // color of every zone
    property bool lightsOn: true        // false: keys off (dark)
    property bool zonesEditable: true   // false: the effect has its own colors (no zone selected, rainbow)
    property int  selectedZone: 0

    // Effect of the preset, shown as an illustration (the keyboard can not be read): 0 static, 1 breath, 2 wave,
    // 3 smooth (Keyboard4Zone.Effect), speed 1 - 4, low brightness dimmer, direction of the wave
    property int  effect: 0
    property int  speed: 2
    property bool brightnessHigh: true
    property bool rightToLeft: false

    // Phase of the animation 0 - 1, one period per cycle of the effect
    property real phase: 0

    NumberAnimation on phase {
        from: 0
        to: 1
        loops: Animation.Infinite
        running: view.visible && view.lightsOn && view.effect !== 0
        // Speed 1 slow - 4 fast
        duration: [4000, 3000, 2000, 1200][Math.max(1, Math.min(4, view.speed)) - 1]
    }

    signal zoneClicked(int zone)

    /*
     * Generic layout in units of a key: rows of the main block (Esc / F-keys, numbers, Tab, Caps, Shift with the up
     * arrow, Ctrl / space / the arrows) and the numeric keypad (+ and Enter of two rows, 0 of two columns)
     */
    readonly property var keyRows: [
        [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
        [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2],
        [1.5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1.5],
        [1.75, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2.25],
        [1.25, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1.75],
        [1.25, 1, 1, 1.25, 5.5, 1, 1]
    ]
    readonly property real mainWidth: 15
    readonly property real keypadGap: 0.5
    readonly property real keypadWidth: 4
    readonly property real unitsWide: mainWidth + keypadGap + keypadWidth
    readonly property real unitsHigh: keyRows.length

    // Arrows (inverted T) right of the shortened right Shift and of the bottom row
    readonly property var arrowKeys: [
        { x: 13, y: 4, w: 1, h: 1 },
        { x: 12, y: 5, w: 1, h: 1 }, { x: 13, y: 5, w: 1, h: 1 }, { x: 14, y: 5, w: 1, h: 1 }
    ]

    // Numeric keypad: column, row, width, height (Home End PgUp PgDn, NumLock / * -, 7 8 9 +, 4 5 6, 1 2 3 Enter, 0 .)
    readonly property var keypadKeys: [
        [0, 0, 1, 1], [1, 0, 1, 1], [2, 0, 1, 1], [3, 0, 1, 1],
        [0, 1, 1, 1], [1, 1, 1, 1], [2, 1, 1, 1], [3, 1, 1, 1],
        [0, 2, 1, 1], [1, 2, 1, 1], [2, 2, 1, 1], [3, 2, 1, 2],
        [0, 3, 1, 1], [1, 3, 1, 1], [2, 3, 1, 1],
        [0, 4, 1, 1], [1, 4, 1, 1], [2, 4, 1, 1], [3, 4, 1, 2],
        [0, 5, 2, 1], [2, 5, 1, 1]
    ]

    readonly property var keys: {
        const result = []

        for (let row = 0; row < keyRows.length; ++row) {
            let x = 0

            for (const width of keyRows[row]) {
                result.push({ x: x, y: row, w: width, h: 1 })
                x += width
            }
        }

        for (const arrow of arrowKeys)
            result.push(arrow)

        for (const key of keypadKeys)
            result.push({ x: mainWidth + keypadGap + key[0], y: key[1], w: key[2], h: key[3] })

        return result
    }

    /*
     * 4 zones of Legion 5 15IAH7H: zone 4 the numeric keypad (with Home, End, PgUp, PgDn), zones 1 - 3 the main block
     * split per row (the middle of a key left of the first bound: zone 1, of the second: zone 2), e.g. zone 1 Esc - F4,
     * ` - 3, Tab - E, Caps - D, Shift - C, Ctrl - Alt; zone 2 F5 - F9, 4 - 9, R - I, F - K, V - M, Space
     */
    readonly property var zoneBounds: [[5, 10], [4, 10], [4.5, 9.5], [4.75, 9.75], [4.25, 8.25], [4.5, 10]]

    function zoneOf(key) {
        const middle = key.x + key.w / 2

        if (zoneCount !== 4)
            return Math.min(zoneCount - 1, Math.floor(middle / unitsWide * zoneCount))

        if (key.x >= mainWidth)
            return 3

        const bounds = zoneBounds[Math.max(0, Math.min(zoneBounds.length - 1, Math.floor(key.y)))]
        return middle < bounds[0] ? 0 : middle < bounds[1] ? 1 : 2
    }

    // Position of the middle of a key over the width, 0 - 1
    function position(key) {
        return (key.x + key.w / 2) / unitsWide
    }

    function scaled(color, factor) {
        return Qt.rgba(color.r * factor, color.g * factor, color.b * factor, 1)
    }

    function keyColor(key) {
        if (!lightsOn)
            return "#1a1a1e"

        const level = brightnessHigh ? 1 : 0.55
        const zoneColor = zoneColors[zoneOf(key)] !== undefined ? zoneColors[zoneOf(key)] : Qt.rgba(1, 1, 1, 1)

        switch (effect) {
        case 1:     // breath: the colors of the zones fade in and out together
            return scaled(zoneColor, level * (0.15 + 0.85 * (0.5 - 0.5 * Math.cos(2 * Math.PI * phase))))
        case 2: {   // wave: a rainbow moving over the keyboard
            const shift = rightToLeft ? phase : -phase
            const hue = ((position(key) * 0.85 + shift) % 1 + 1) % 1
            return scaled(Qt.hsva(hue, 0.85, 1, 1), level)
        }
        case 3:     // smooth: the whole keyboard changes its color
            return scaled(Qt.hsva(phase, 0.85, 1, 1), level)
        default:    // static
            return scaled(zoneColor, level)
        }
    }

    // The same look as KeyboardView
    readonly property color selectionColor: {
        const accent = theme.accentHover
        const luma = 0.2126 * accent.r + 0.7152 * accent.g + 0.0722 * accent.b
        return luma >= 0.6 ? accent : Qt.tint(accent, Qt.rgba(1, 1, 1, (0.6 - luma) / (1 - luma)))
    }
    readonly property color ledRimColor: "#8a8a94"

    // Not wider than maxDrawWidth and not higher than maxHeightPart of the window, centered (as KeyboardView)
    property real maxDrawWidth: 900
    property real maxHeightPart: 0.35
    readonly property real drawWidth: Math.min(view.width, maxDrawWidth,
                                               Window.height > 0 ? Window.height * maxHeightPart * unitsWide / unitsHigh : view.width)

    implicitHeight: stage.height

    Rectangle {
        id: stage

        anchors.horizontalCenter: parent.horizontalCenter
        width: view.drawWidth
        height: width > 0 ? (width - 2 * padding) * view.unitsHigh / view.unitsWide + 2 * padding : 0

        readonly property real padding: Math.max(10, width / 60)

        radius: theme.styled ? 12 : 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.groupBackground }
            GradientStop { position: 1.0; color: theme.window }
        }

        Item {
            id: area

            x: stage.padding
            y: stage.padding
            width: stage.width - 2 * stage.padding
            height: stage.height - 2 * stage.padding

            readonly property real unit: width / view.unitsWide
            readonly property real keyGap: Math.max(3, width / 180)

            // Plate of the keys
            Rectangle {
                x: -stage.padding / 2
                y: -stage.padding / 2
                width: area.width + stage.padding
                height: area.height + stage.padding
                radius: stage.padding / 2
                color: "#101013"
                border.width: 1
                border.color: "#050506"
            }

            Repeater {
                model: view.keys

                delegate: Item {
                    id: key

                    required property var modelData

                    readonly property int   zone: view.zoneOf(modelData)
                    readonly property bool  selected: view.lightsOn && view.zonesEditable && view.selectedZone === zone
                    readonly property bool  dimmed: view.lightsOn && view.zonesEditable && !selected
                    readonly property color rawColor: view.keyColor(modelData)
                    readonly property color ledColor: dimmed ? Qt.rgba(rawColor.r * 0.45, rawColor.g * 0.45, rawColor.b * 0.45, 1) : rawColor
                    readonly property real  corner: Math.min(width, height) * 0.16

                    x: modelData.x * area.unit
                    y: modelData.y * area.unit
                    width: modelData.w * area.unit
                    height: modelData.h * area.unit

                    Item {
                        anchors.fill: parent
                        anchors.margins: area.keyGap / 2

                        Rectangle {
                            anchors.fill: parent
                            radius: key.corner
                            color: Qt.darker(key.ledColor, 2.2)
                        }

                        Rectangle {
                            id: cap

                            anchors.fill: parent
                            anchors.bottomMargin: Math.max(2, parent.height * 0.08)
                            radius: key.corner
                            color: key.ledColor
                            border.width: 1
                            border.color: view.ledRimColor

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: cap.border.width
                                radius: cap.radius
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.25) }
                                    GradientStop { position: 0.55; color: Qt.rgba(1, 1, 1, 0.0) }
                                }
                            }
                        }

                        // The selected zone: outline around its keys
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -2
                            visible: key.selected
                            radius: key.corner + 2
                            color: "transparent"
                            border.width: 2
                            border.color: view.selectionColor
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: view.lightsOn && view.zonesEditable
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: view.zoneClicked(key.zone)
                    }

                    ThemedToolTip {
                        visible: keyHover.hovered && view.lightsOn && view.zonesEditable
                        delay: 600
                        text: "Zone " + (key.zone + 1)
                    }

                    HoverHandler {
                        id: keyHover
                    }
                }
            }
        }
    }
}
