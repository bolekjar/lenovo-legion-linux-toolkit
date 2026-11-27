// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI: 4-zone RGB keyboard (toolbar page Keyboard on a notebook with a 4-zone keyboard), data type
// KEYBOARD_FOUR_ZONE (Keyboard4Zone of RGBController.proto): the presets Off and 1 - 4 (Fn+Space switches them), the
// settings of the active preset on an illustrative keyboard (zones divided evenly). The changes are sent at once
// (autoApply), the colors after a short pause of the color picker.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    id: page

    columnWidth: 540

    DataSource {
        id: keyboard

        dataType: "KEYBOARD_FOUR_ZONE"
        autoApply: true
    }

    // Zone edited by the color picker (0-3)
    property int selectedZone: 0

    // Active preset: 0 off, 1 - 4; the settings shown are the ones of the active preset (of preset 1 when off)
    readonly property int  activePreset: Number(keyboard.value("active_preset", keyboard.revision) || 0)
    readonly property bool lightsOn: activePreset > 0
    readonly property int  editedPreset: Math.max(activePreset, 1) - 1

    function presetPath(field) {
        return "presets[" + page.editedPreset + "]." + field
    }

    function zoneColor(zone) {
        const value = keyboard.value(page.presetPath("zone" + (zone + 1) + "_color"), keyboard.revision)
        const rgb   = value === undefined ? 0xFFFFFF : Number(value)

        return Qt.rgba(((rgb >> 16) & 0xFF) / 255, ((rgb >> 8) & 0xFF) / 255, (rgb & 0xFF) / 255, 1)
    }

    function colorValue(color) {
        return (Math.round(color.r * 255) << 16) | (Math.round(color.g * 255) << 8) | Math.round(color.b * 255)
    }

    // Colors of the picker sent after a pause (the picker reports every move)
    property var pendingColors: ({})

    Timer {
        id: colorTimer

        interval: 150
        onTriggered: {
            keyboard.applyValues(page.pendingColors)
            page.pendingColors = {}
        }
    }

    function setColors(zones, color) {
        const values = Object.assign({}, page.pendingColors)

        for (const zone of zones)
            values[page.presetPath("zone" + (zone + 1) + "_color")] = page.colorValue(color)

        page.pendingColors = values
        colorTimer.restart()
    }

    // Number of the zones of the keyboard (the protocol has 4)
    readonly property int zoneCount: 4

    /*
     * Illustrative keyboard (the keys of the zones differ by the model, the hardware decides them): a generic layout,
     * the main block and the numeric keypad, in units of a key. The width is divided evenly by the zones from the left,
     * a key has the color of the zone of its middle.
     */
    readonly property var keyRows: [
        [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],                 // Esc, F1 - F12, Ins, Del
        [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2],                    // ` 1 - 0 - = Backspace
        [1.5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1.5],                // Tab Q - P [ ] \
        [1.75, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2.25],                 // Caps A - L ; ' Enter
        [2.25, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2.75],                    // Shift Z - / Shift
        [1.25, 1, 1, 1.25, 5.5, 1.25, 1.25, 1, 1, 1.5]                 // Ctrl Fn Win Alt Space Alt Ctrl, arrows
    ]
    readonly property real mainWidth: 15
    readonly property real keypadGap: 0.5
    readonly property real keypadWidth: 4
    readonly property real totalWidth: mainWidth + keypadGap + keypadWidth

    // Keys as {x, y, w} in units: the rows of the main block and the 4 x 6 numeric keypad
    readonly property var keys: {
        const result = []

        for (let row = 0; row < keyRows.length; ++row) {
            let x = 0

            for (const width of keyRows[row]) {
                result.push({ x: x, y: row, w: width })
                x += width
            }
        }

        for (let row = 1; row < keyRows.length; ++row)
            for (let column = 0; column < 4; ++column)
                result.push({ x: mainWidth + keypadGap + column, y: row, w: 1 })

        return result
    }

    function zoneOf(key) {
        return Math.min(zoneCount - 1, Math.floor((key.x + key.w / 2) / totalWidth * zoneCount))
    }

    // Preview of the keyboard, a click on a key selects its zone for the color picker
    Section {
        title: "4-Zone Keyboard"
        source: keyboard
        Layout.columnSpan: page.columns

        // Presets: Off and 1 - 4 (the same order as Fn+Space)
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Label { text: "Preset" }

            Repeater {
                model: ["Off", "1", "2", "3", "4"]

                delegate: Button {
                    required property int    index
                    required property string modelData

                    text: modelData
                    checkable: true
                    checked: page.activePreset === index
                    highlighted: checked
                    onClicked: keyboard.applyValues({ "active_preset": index })
                }
            }

            Item { Layout.fillWidth: true }

            Label {
                opacity: 0.7
                text: "Fn+Space switches the presets"
            }
        }

        Item {
            id: preview

            Layout.fillWidth: true
            Layout.preferredHeight: unit * page.keyRows.length

            readonly property real unit: width / page.totalWidth

            Repeater {
                model: page.keys

                delegate: Rectangle {
                    required property var modelData

                    readonly property int zone: page.zoneOf(modelData)

                    x: modelData.x * preview.unit + 2
                    y: modelData.y * preview.unit + 2
                    width: modelData.w * preview.unit - 4
                    height: preview.unit - 4
                    radius: 4
                    color: page.lightsOn ? page.zoneColor(zone) : theme.base
                    opacity: page.lightsOn ? (page.selectedZone === zone ? 1 : 0.75) : 0.4
                    border.width: page.selectedZone === zone ? 2 : 1
                    border.color: page.selectedZone === zone ? theme.accent : theme.groupBorder

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: page.selectedZone = parent.zone
                    }
                }
            }
        }

        // Zones under the keyboard (the same even division)
        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Repeater {
                model: page.zoneCount

                delegate: Label {
                    required property int index

                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    horizontalAlignment: Text.AlignHCenter
                    text: "Zone " + (index + 1)
                    font.bold: page.selectedZone === index
                    color: page.selectedZone === index ? theme.accent : theme.text

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: page.selectedZone = parent.index
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            opacity: 0.7
            text: "Illustration: the keys of the zones differ by the model of the keyboard."
        }

        // Settings of the active preset
        ColumnLayout {
            Layout.fillWidth: true
            enabled: page.lightsOn

            Field { source: keyboard; path: page.presetPath("effect") }
            Field { source: keyboard; path: page.presetPath("brightness") }
            Field { source: keyboard; path: page.presetPath("speed") }
            Field { source: keyboard; path: page.presetPath("direction") }
        }
    }

    Section {
        title: "Color of zone " + (page.selectedZone + 1) + (page.lightsOn ? " (preset " + page.activePreset + ")" : "")
        source: keyboard
        enabled: page.lightsOn

        ColorPicker {
            Layout.fillWidth: true
            color: page.zoneColor(page.selectedZone)
            onColorPicked: newColor => page.setColors([page.selectedZone], newColor)
        }

        Button {
            text: "Use for all zones"
            onClicked: page.setColors([0, 1, 2, 3], page.zoneColor(page.selectedZone))
        }
    }
}
