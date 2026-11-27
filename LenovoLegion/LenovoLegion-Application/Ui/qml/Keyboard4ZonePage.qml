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

    // Static and breath use the colors of the zones, wave and smooth their own colors
    readonly property int  effect: Number(keyboard.value(page.presetPath("effect"), keyboard.revision) || 0)
    readonly property bool zonesEditable: lightsOn && (effect === 0 || effect === 1)   // EFFECT_STATIC, EFFECT_BREATH

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

        Keyboard4ZoneView {
            Layout.fillWidth: true
            Layout.topMargin: 4

            zoneColors: [page.zoneColor(0), page.zoneColor(1), page.zoneColor(2), page.zoneColor(3)]
            lightsOn: page.lightsOn
            zonesEditable: page.zonesEditable
            selectedZone: page.selectedZone
            onZoneClicked: zone => page.selectedZone = zone
        }

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            opacity: 0.7
            text: !page.lightsOn ? "The keyboard backlight is off"
                : page.zonesEditable ? "Click a key to select its zone. Illustration: the keys of the zones differ by the model of the keyboard."
                : "The effect uses its own colors, the colors of the zones are used by the static and the breath effect"
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
        enabled: page.zonesEditable

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
