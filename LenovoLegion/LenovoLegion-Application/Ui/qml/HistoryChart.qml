// SPDX-License-Identifier: GPL-2.0
// Rolling bar chart of the last 100 values the same as HWMonitor of the widget pages:
// gray background, white bars (orange / red above the warning / critical thresholds), grid with axis labels (values, time),
// title with the current value ("%1" in the title), tool tip
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
    property int    sampleInterval: 500 // ms between the samples (time axis: samples * interval)

    // Thresholds of the values (NaN = none): bars and the value above warning orange, above critical red
    property real   warning: NaN
    property real   critical: NaN

    // Colors of the levels: bars (on the gray background) and the value text (darker, readable on the gray)
    readonly property var barColors:  ({ "normal": "white", "warning": "#ffb347", "critical": "#ff5c5c" })
    readonly property var textColors: ({ "normal": "black", "warning": "#7a3e00", "critical": "#9a0000" })

    function level(value) {
        if (!isNaN(critical) && value >= critical)
            return "critical"

        if (!isNaN(warning) && value >= warning)
            return "warning"

        return "normal"
    }

    readonly property string valueText: chart.fieldWidth > 0 ? String(Math.round(chart.value)).padStart(chart.fieldWidth,"0") : String(Math.round(chart.value))

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
    implicitHeight: twoLines ? 130 : 110

    onWarningChanged: canvas.requestPaint()
    onCriticalChanged: canvas.requestPaint()

    onMinimumChanged: canvas.requestPaint()
    onMaximumChanged: canvas.requestPaint()

    // Colors of the widget page (QChart background 0x999999, white bar set), not part of the themes
    color: "#999999"
    radius: 4

    // Title: one line ("Core Clock [1565 MHz]"), or in narrow charts the name and below it the current value
    // in a larger bold font ("Avg" / "2326 MHz"), the value in the color of its level
    Column {
        id: titleText

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 4

        Text {
            width: parent.width
            visible: !chart.twoLines

            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight

            text: chart.title.arg(chart.valueText)
            font.pixelSize: theme.fontPixelSize
            color: chart.textColors[chart.level(chart.value)]
        }

        Text {
            width: parent.width
            visible: chart.twoLines

            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight

            text: chart.title.split(" [")[0]
            font.pixelSize: theme.fontPixelSize
            color: "black"
        }

        Text {
            width: parent.width
            visible: chart.twoLines

            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight

            // "[%1 MHz]" -> "2326 MHz"
            text: chart.title.indexOf(" [") >= 0 ? chart.title.substring(chart.title.indexOf(" [") + 2).replace("]", "").arg(chart.valueText) : ""
            font.pixelSize: theme.fontPixelSize + 3
            font.bold: true
            color: chart.textColors[chart.level(chart.value)]
        }
    }

    Canvas {
        id: canvas

        anchors.top: titleText.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            const context = getContext("2d")
            const range = Math.max(chart.maximum - chart.minimum, 1)

            context.clearRect(0,0,width,height)

            // Axis labels in a small font: values of the grid rows on the left, time under the plot
            const decimals = chart.maximum - chart.minimum < 4 ? 1 : 0
            const values = []

            for (let line = 0; line <= 4; ++line)
                values.push((chart.maximum - (chart.maximum - chart.minimum) * line / 4).toFixed(decimals))

            context.font = "9px sans-serif"
            context.fillStyle = "#202020"

            let labelWidth = 0

            for (const text of values)
                labelWidth = Math.max(labelWidth, context.measureText(text).width)

            // Plot area right of the value labels and above the time labels
            const left = Math.ceil(labelWidth) + 3
            const top = 5
            const bottom = height - 11
            const plotWidth = Math.max(width - left, 1)
            const plotHeight = Math.max(bottom - top, 1)

            context.textAlign = "right"
            context.textBaseline = "middle"

            for (let line = 0; line <= 4; ++line)
                context.fillText(values[line], left - 3, top + plotHeight * line / 4)

            context.textBaseline = "top"
            context.textAlign = "left"
            context.fillText("-" + Math.round(chart.samples * chart.sampleInterval / 1000) + " s", left, bottom + 2)
            context.textAlign = "right"
            context.fillText("0 s", width, bottom + 2)

            // Grid: 4 rows of the value axis, columns of about the same size (square cells in narrow and wide charts)
            context.strokeStyle = "#b8b8b8"
            context.lineWidth = 1
            context.beginPath()

            for (let line = 0; line <= 4; ++line) {
                const y = Math.round(top + plotHeight * line / 4) + 0.5
                context.moveTo(left,y)
                context.lineTo(left + plotWidth,y)
            }

            const columns = Math.max(1, Math.round(plotWidth / Math.max(plotHeight / 4, 1)))

            for (let line = 0; line <= columns; ++line) {
                const x = left + Math.min(Math.round(plotWidth * line / columns), plotWidth - 1) + 0.5
                context.moveTo(x,top)
                context.lineTo(x,top + plotHeight)
            }

            context.stroke()

            // Newest value on the right
            const barWidth = plotWidth / chart.samples
            const offset = chart.samples - chart.history.length

            // Bar of every sample in the color of its level (warning / critical history is visible)
            for (let i = 0; i < chart.history.length; ++i) {
                const level = Math.max(0, Math.min(1, (chart.history[i] - chart.minimum) / range))
                const barHeight = level * plotHeight

                context.fillStyle = chart.barColors[chart.level(chart.history[i])]
                context.fillRect(left + (offset + i) * barWidth, top + plotHeight - barHeight, Math.max(barWidth, 1), barHeight)
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
