// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI: 4-zone RGB keyboard (toolbar page Keyboard on a notebook with a 4-zone keyboard), data type
// KEYBOARD_FOUR_ZONE (Keyboard4Zone of RGBController.proto). The changes are sent at once (autoApply), the colors
// after a short pause of the color picker.
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

    readonly property bool lightsOn: Number(keyboard.value("effect", keyboard.revision)) !== 0   // EFFECT_OFF

    function zoneColor(zone) {
        const value = keyboard.value("zone" + (zone + 1) + "_color", keyboard.revision)
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
            values["zone" + (zone + 1) + "_color"] = page.colorValue(color)

        page.pendingColors = values
        colorTimer.restart()
    }

    // Preview of the keyboard: the 4 zones from the left, a click selects the zone for the color picker
    Section {
        title: "4-Zone Keyboard"
        source: keyboard
        Layout.columnSpan: page.columns

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: 4

                delegate: Rectangle {
                    required property int index

                    Layout.fillWidth: true
                    Layout.preferredHeight: 64
                    radius: 6
                    color: page.lightsOn ? page.zoneColor(index) : theme.base
                    opacity: page.lightsOn ? 1 : 0.4
                    border.width: page.selectedZone === index ? 3 : 1
                    border.color: page.selectedZone === index ? theme.accent : theme.groupBorder

                    Label {
                        anchors.centerIn: parent
                        text: "Zone " + (index + 1)
                        color: parent.color.hslLightness > 0.6 ? "black" : "white"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: page.selectedZone = index
                    }
                }
            }
        }

        Field { source: keyboard; path: "effect" }
        Field { source: keyboard; path: "brightness" }
        Field { source: keyboard; path: "speed" }
        Field { source: keyboard; path: "direction" }
    }

    Section {
        title: "Color of zone " + (page.selectedZone + 1)
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
