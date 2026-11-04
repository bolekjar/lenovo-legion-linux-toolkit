// SPDX-License-Identifier: GPL-2.0
// Rolling bar chart of the last 100 values the same as HWMonitor of the widget pages:
// gray background, white bars, horizontal grid lines, title with the current value ("%1" in the title), tool tip
import QtQuick
import QtQuick.Controls

Rectangle {
    id: chart

    property string title           // e.g. "CPU Power [%1 W]"
    property real   minimum: 0
    property real   maximum: 100
    property string tooltip
    property int    fieldWidth: 0   // value padded with zeros to the width (frequencies: 4)
    property bool   twoLines: false // name and value ("[%1 ...]") on two lines, narrow charts in one row

    readonly property int samples: 100
    property var          history: []
    property real         value: minimum

    function push(newValue) {
        value = newValue

        const values = history.length === samples ? history.slice(1) : history.slice()

        values.push(newValue)
        history = values
        canvas.requestPaint()
    }

    // Narrow charts in one row of the HW monitor, a too long title is elided (the tool tip has the whole text)
    implicitWidth: 80
    implicitHeight: twoLines ? 116 : 100

    // Colors of the widget page (QChart background 0x999999, white bar set), not part of the themes
    color: "#999999"
    radius: 4

    Text {
        id: titleText

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 4

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight

        text: (chart.twoLines ? chart.title.replace(" [", "\n[") : chart.title).arg(chart.fieldWidth > 0 ? String(Math.round(chart.value)).padStart(chart.fieldWidth,"0") : Math.round(chart.value))
        font.pixelSize: theme.fontPixelSize
        color: "black"
    }

    Canvas {
        id: canvas

        anchors.top: titleText.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5

        onPaint: {
            const context = getContext("2d")
            const range = Math.max(chart.maximum - chart.minimum, 1)
            const barWidth = width / chart.samples

            context.clearRect(0,0,width,height)

            // Grid of the default value axis (5 lines)
            context.strokeStyle = "#b8b8b8"
            context.lineWidth = 1
            for (let line = 0; line <= 4; ++line) {
                const y = Math.round(height * line / 4) + 0.5
                context.beginPath()
                context.moveTo(0,y)
                context.lineTo(width,y)
                context.stroke()
            }

            // Newest value on the right
            context.fillStyle = "white"
            const offset = chart.samples - chart.history.length
            for (let i = 0; i < chart.history.length; ++i) {
                const level = Math.max(0, Math.min(1, (chart.history[i] - chart.minimum) / range))
                const barHeight = level * height
                context.fillRect((offset + i) * barWidth, height - barHeight, Math.max(barWidth, 1), barHeight)
            }
        }
    }

    HoverHandler {
        id: hover
    }

    ThemedToolTip {
        visible: hover.hovered && text.length > 0
        text: chart.tooltip
    }
}
