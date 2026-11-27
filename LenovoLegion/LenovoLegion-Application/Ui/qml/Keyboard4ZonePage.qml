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

    // Phase of the animated swatches of the presets (one period of the effects in 3 s)
    property real swatchPhase: 0

    NumberAnimation on swatchPhase {
        from: 0
        to: 1
        duration: 3000
        loops: Animation.Infinite
        running: page.visible
    }

    // Static and breath use the colors of the zones, wave and smooth their own colors
    readonly property int  effect: Number(keyboard.value(page.presetPath("effect"), keyboard.revision) || 0)
    readonly property bool zonesEditable: lightsOn && (effect === 0 || effect === 1)   // EFFECT_STATIC, EFFECT_BREATH

    // Preview of the keyboard, a click on a key selects its zone for the color picker
    Section {
        title: "4-Zone Keyboard"
        source: keyboard
        Layout.columnSpan: page.columns

        // Presets Off and 1 - 4 (the order of Fn+Space) in the style of the view switcher of the per-key keyboard
        // (KeyboardView): dark chassis, the active preset lit by a light bar in the accent color, a swatch of its colors
        RowLayout {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            Rectangle {
                id: presets

                implicitWidth: presetRow.implicitWidth + 8
                implicitHeight: presetRow.implicitHeight + 8
                radius: height / 2
                border.width: 1
                border.color: "#46464c"
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#2c2c31" }
                    GradientStop { position: 1.0; color: "#16161a" }
                }

                Row {
                    id: presetRow

                    anchors.centerIn: parent
                    spacing: 4

                    Repeater {
                        model: ["Off", "Preset 1", "Preset 2", "Preset 3", "Preset 4"]

                        delegate: AbstractButton {
                            id: presetButton

                            required property int    index
                            required property string modelData

                            readonly property bool current: page.activePreset === index
                            // Colors of the swatch: the zones (static, breath) or a rainbow (wave, smooth)
                            readonly property int  presetEffect: index > 0 ? Number(keyboard.value("presets[" + (index - 1) + "].effect", keyboard.revision) || 0) : 0
                            // Swatch of the preset animated by its effect (page.swatchPhase): static the colors of the
                            // zones, breath the colors fading, wave a moving rainbow, smooth one changing color
                            readonly property var  zoneColors: {
                                const colors = []

                                if (index === 0)
                                    return colors

                                for (let zone = 1; zone <= 4; ++zone) {
                                    const rgb = Number(keyboard.value("presets[" + (index - 1) + "].zone" + zone + "_color", keyboard.revision) || 0)
                                    colors.push(Qt.rgba(((rgb >> 16) & 0xFF) / 255, ((rgb >> 8) & 0xFF) / 255, (rgb & 0xFF) / 255, 1))
                                }
                                return colors
                            }
                            readonly property var  swatch: {
                                const phase = page.swatchPhase
                                const colors = []

                                for (let zone = 0; zone < zoneColors.length; ++zone) {
                                    switch (presetEffect) {
                                    case 1: {   // breath
                                        const level = 0.15 + 0.85 * (0.5 - 0.5 * Math.cos(2 * Math.PI * phase))
                                        const color = zoneColors[zone]
                                        colors.push(Qt.rgba(color.r * level, color.g * level, color.b * level, 1))
                                        break
                                    }
                                    case 2:     // wave
                                        colors.push(Qt.hsva(((zone * 0.22 - phase) % 1 + 1) % 1, 0.85, 1, 1))
                                        break
                                    case 3:     // smooth
                                        colors.push(Qt.hsva(phase, 0.85, 1, 1))
                                        break
                                    default:    // static
                                        colors.push(zoneColors[zone])
                                    }
                                }
                                return colors
                            }
                            readonly property string effectName: ["Static", "Breath", "Wave", "Smooth"][Math.max(0, Math.min(3, presetEffect))]

                            hoverEnabled: true
                            implicitHeight: 36
                            implicitWidth: presetContent.implicitWidth + 28

                            onClicked: keyboard.applyValues({ "active_preset": index })

                            background: Rectangle {
                                radius: height / 2
                                color: presetButton.current ? "#3b3b44" : presetButton.hovered ? "#2f2f36" : "transparent"
                                border.width: presetButton.current ? 1 : 0
                                border.color: "#55555e"

                                // Light bar of the active preset (like the LED bars of the notebook)
                                Rectangle {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: 3
                                    width: parent.width - 2 * parent.radius
                                    height: 3
                                    radius: 1.5
                                    visible: presetButton.current
                                    gradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: theme.accent }
                                        GradientStop { position: 1.0; color: theme.accentHover }
                                    }
                                }
                            }

                            contentItem: Item {
                                implicitWidth: presetContent.implicitWidth
                                implicitHeight: presetContent.implicitHeight

                                Row {
                                    id: presetContent

                                    anchors.centerIn: parent
                                    spacing: 8

                                    // Swatch: the 4 zones of the preset as small keys
                                    Row {
                                        anchors.verticalCenter: parent.verticalCenter
                                        visible: presetButton.swatch.length > 0
                                        spacing: 2

                                        Repeater {
                                            model: presetButton.swatch

                                            delegate: Rectangle {
                                                required property color modelData

                                                width: 6
                                                height: 12
                                                radius: 1.5
                                                color: modelData
                                                opacity: presetButton.current ? 1 : 0.7
                                            }
                                        }
                                    }

                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: presetButton.modelData
                                        font.pixelSize: theme.fontPixelSize
                                        font.bold: presetButton.current
                                        color: presetButton.current ? "white" : "#a0a0a8"
                                    }
                                }
                            }

                            ThemedToolTip {
                                visible: presetButton.hovered
                                delay: 500
                                text: presetButton.index === 0 ? "Keyboard backlight off" : presetButton.effectName + "\nFn+Space switches the presets"
                            }
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        Keyboard4ZoneView {
            Layout.fillWidth: true
            Layout.topMargin: 8

            zoneColors: [page.zoneColor(0), page.zoneColor(1), page.zoneColor(2), page.zoneColor(3)]
            lightsOn: page.lightsOn
            zonesEditable: page.zonesEditable
            selectedZone: page.selectedZone
            effect: page.effect
            speed: Number(keyboard.value(page.presetPath("speed"), keyboard.revision) || 2)
            brightnessHigh: Number(keyboard.value(page.presetPath("brightness"), keyboard.revision) || 0) === 1   // BRIGHTNESS_HIGH
            rightToLeft: Number(keyboard.value(page.presetPath("direction"), keyboard.revision) || 0) === 1      // DIRECTION_RIGHT_TO_LEFT
            onZoneClicked: zone => page.selectedZone = zone
        }

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            opacity: 0.7
            text: !page.lightsOn ? "The keyboard backlight is off"
                : page.zonesEditable ? "Click a key to select its zone. Illustration: the keys of the zones differ by the model of the keyboard."
                : "The effect uses its own colors, the colors of the zones are used by the static and the breath effect (the animation is an illustration)"
        }
    }

    // Settings of the active preset (the whole keyboard)
    Section {
        title: page.lightsOn ? "Effect (preset " + page.activePreset + ")" : "Effect"
        source: keyboard
        enabled: page.lightsOn
        // Not narrower than the rows of the fields (label and control), the color picker next to it takes the rest
        Layout.minimumWidth: implicitWidth

        Field { source: keyboard; path: page.presetPath("effect") }
        Field { source: keyboard; path: page.presetPath("brightness") }
        Field { source: keyboard; path: page.presetPath("speed") }
        Field { source: keyboard; path: page.presetPath("direction") }
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

        ThemedButton {
            text: "Use for all zones"
            onClicked: page.setColors([0, 1, 2, 3], page.zoneColor(page.selectedZone))
        }
    }
}
