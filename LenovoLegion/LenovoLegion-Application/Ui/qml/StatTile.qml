// SPDX-License-Identifier: GPL-2.0
// Tile with one live value of the detail windows of the HW monitor: name, the value in a large font, a detail line
// and a bar of the value in its range (fraction 0..1, NaN = no bar). The value and the bar in the color of the level
// (warning / critical like HistoryChart), the theme colors otherwise.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: tile

    property string label
    property string value
    property string unit
    property string detail                  // e.g. "of 5400 MHz", "CPU 3"
    property real   fraction: NaN           // bar 0..1
    property string level: "normal"         // normal, warning, critical
    property string tooltip
    property bool   active: true            // false: no live value (e.g. the GPU is suspended), shown dimmed

    readonly property var levelColors: ({ "normal": theme.accent, "warning": "#ffb347", "critical": "#ff5c5c" })

    implicitWidth: 150
    implicitHeight: column.implicitHeight + 16
    radius: theme.radius
    color: theme.base
    border.width: 1
    border.color: theme.groupBorder
    opacity: active ? 1 : 0.55

    ColumnLayout {
        id: column

        anchors.fill: parent
        anchors.margins: 8
        spacing: 2

        Label {
            Layout.fillWidth: true
            text: tile.label
            color: theme.disabledText
            elide: Text.ElideRight
        }

        // Value and unit on one baseline, the value does not change the height of the tile
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: tile.active ? tile.value : "–"
                font.pixelSize: theme.fontPixelSize + 9
                font.bold: true
                color: tile.level === "normal" ? theme.text : tile.levelColors[tile.level]
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                Layout.fillWidth: true
                text: tile.unit
                color: theme.disabledText
                elide: Text.ElideRight
            }
        }

        Label {
            Layout.fillWidth: true
            text: tile.detail
            color: theme.disabledText
            font.pixelSize: theme.fontPixelSize - 1
            elide: Text.ElideRight
            // The place is kept: tiles of one row have the same height
            opacity: text.length > 0 ? 1 : 0
        }

        // Bar of the value in its range, the place is kept also without a bar
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 4
            implicitHeight: 5
            radius: 2
            color: theme.trackBackground
            opacity: isNaN(tile.fraction) ? 0 : 1

            Rectangle {
                width: parent.width * (isNaN(tile.fraction) || !tile.active ? 0 : Math.max(0, Math.min(1, tile.fraction)))
                height: parent.height
                radius: parent.radius
                color: tile.levelColors[tile.level]

                Behavior on width { NumberAnimation { duration: 200 } }
            }
        }
    }

    HoverHandler {
        id: hover
    }

    ThemedToolTip {
        visible: hover.hovered && text.length > 0
        text: tile.tooltip
    }
}
