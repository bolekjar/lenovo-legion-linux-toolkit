// SPDX-License-Identifier: GPL-2.0
// Fan curve the same as FanControl of the widget pages:
//  - Maximum / Custom: fan full speed (FanOption), only in the custom thermal mode when supported,
//  - 10 points, level 1..10, point n is at least n, point10 at least 9 (min of FanControl.proto), a point dragged
//    under its limit stays at the limit, the curve stays monotonic when a point is moved,
//  - stretched to the size of the page (sliders over the whole height, points over the whole width),
//  - edited only in the custom thermal mode with Custom, otherwise shown: 1..10 (not custom mode), all 10 (Maximum),
//  - tool tip of a point: temperatures and fan speeds of the default curves of the power profile (127 = N/A),
//  - Apply sends the whole curve (atomic in FanControl.proto).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

// Buttons above the group "Fan curve" (pushButton_MaxSpeed, pushButton_Custom, groupBox_fanCurve of FanControl.ui)
ColumnLayout {
    id: editor

    property DataSource profileSource    // POWER_PROFILE
    property DataSource fanSource        // FAN_OPTION

    Layout.fillWidth: true
    spacing: 12

    DataSource {
        id: curve

        dataType: "FAN_CURVE"
    }

    readonly property var  points:             ["point1", "point2", "point3", "point4", "point5", "point6", "point7", "point8", "point9", "point10"]
    readonly property bool custom:             profileSource !== null && profileSource.revision >= 0 && profileSource.value("thermal_mode") === 255
    readonly property bool fullSpeedSupported: fanSource !== null && fanSource.revision >= 0 && fanSource.value("full_speed.supported") === true
    // value() is not tracked: the revision of the source is a dependency of the binding
    readonly property bool fullSpeed:          fullSpeedSupported && fanSource.revision >= 0 && fanSource.value("full_speed.current_value") === true
    readonly property bool editable:           custom && !fullSpeed
    readonly property int  profile:            profileSource !== null && profileSource.revision >= 0 ? profileSource.value("current_value") : 0

    function pointModel(index) {
        return curve.field("current_value." + points[index])
    }

    /*
     * Monotonic curve: later points are raised to the value, earlier points are lowered to it (normalizeCurrentFanCurveControlData)
     */
    function setPoint(index, value) {
        const field = pointModel(index)
        const level = Math.max(field.min, Math.min(field.max, value))

        curve.beginChanges()   // one notification for all points

        for (let i = 0; i < points.length; ++i) {
            const model = pointModel(i)
            const current = model.value

            if (i === index)
                model.value = level
            else if (i > index && current < level)
                model.value = level
            else if (i < index && current > level)
                model.value = level
        }

        curve.endChanges()
    }

    function pointToolTip(index) {
        const curves = [["cpu_default", "CPU"], ["gpu_default", "GPU"], ["sys_default", "SYS"], ["cpusen_default", "CPU sensor"]]
        const temperatures = []
        const speeds = []

        for (const [map, name] of curves) {
            const sensors = (curve.revision, curve.repeatedValues(map + "[" + profile + "].sensors"))
            const fans    = curve.repeatedValues(map + "[" + profile + "].fan")

            if (sensors.length > index)
                temperatures.push(name + " temp: " + (sensors[index] === 127 ? "N/A" : sensors[index]) + " ℃")
            if (fans.length > index)
                speeds.push(name + " fan speed: " + fans[index] + " rpm")
        }

        return temperatures.concat(speeds).join("\n")
    }

    // Fan full speed (pushButton_MaxSpeed / pushButton_Custom), both over the whole width
    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        ThemedButton {
            Layout.fillWidth: true

            text: "Maximum"
            checkable: true
            autoExclusive: true
            checked: editor.fullSpeed
            enabled: editor.custom && editor.fullSpeedSupported
            onClicked: editor.fanSource.field("full_speed.current_value").value = true
        }

        ThemedButton {
            Layout.fillWidth: true

            text: "Custom"
            checkable: true
            autoExclusive: true
            checked: !editor.fullSpeed
            enabled: editor.custom && editor.fullSpeedSupported
            onClicked: editor.fanSource.field("full_speed.current_value").value = false
        }
    }

    Section {
        Layout.fillWidth: true
        Layout.fillHeight: true

        title: "Fan curve"
        applyBar: true
        source: curve
        stretchContent: true

        // Default curve: levels 1..10 (the curve FanControl shows outside the custom mode), sent at once (whole curve)
        presets: [{ text: "Default", values: (function () {
            const values = {}

            for (let i = 0; i < editor.points.length; ++i)
                values["current_value." + editor.points[i]] = i + 1

            return values
        })() }]

        // Disabled as a whole outside the custom mode and with Maximum (all points at 10), the same as groupBox_fanCurve
        enabled: editor.editable

        // Sliders over the whole height of the stretched section (minimum 220), the axis labels along the sliders
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // Fan speed axis: labels at the levels of the handles of the sliders (top 100 %, bottom 0 %)
            Item {
                id: axis

                readonly property Item track: pointRepeater.count > 0 && pointRepeater.itemAt(0) !== null ? pointRepeater.itemAt(0).track : null

                Layout.fillHeight: true
                implicitWidth: axisMetrics.width

                TextMetrics {
                    id: axisMetrics

                    text: "100 %"
                }

                Repeater {
                    model: ["100 %", "75 %", "50 %", "25 %", "0 %"]

                    delegate: Label {
                        required property string modelData
                        required property int    index

                        readonly property real handle: 22   // handle of ThemedSlider

                        visible: axis.track !== null
                        y: axis.track === null ? 0
                         : axis.track.topPadding + handle / 2 + index / 4 * (axis.track.availableHeight - handle) - height / 2
                        text: modelData
                        color: axis.enabled ? theme.text : theme.disabledText
                    }
                }
            }

            Repeater {
                id: pointRepeater

                model: editor.points.length

                delegate: ColumnLayout {
                    id: point

                    required property int index

                    readonly property FieldModel pointField: (curve.revision, editor.pointModel(index))
                    readonly property int        shown: !editor.custom ? index + 1
                                                      : editor.fullSpeed ? 10
                                                      : (pointField !== null ? pointField.value : 0)

                    readonly property Item track: slider

                    // Points over the whole width in columns of the same width, independent of the content
                    // (a changed label must not move the other points)
                    Layout.fillWidth: true
                    Layout.maximumWidth: Number.POSITIVE_INFINITY
                    Layout.preferredWidth: 1
                    Layout.horizontalStretchFactor: 1
                    Layout.fillHeight: true

                    ThemedSlider {
                        id: slider

                        Layout.fillHeight: true
                        Layout.minimumHeight: 220
                        Layout.alignment: Qt.AlignHCenter

                        orientation: Qt.Vertical
                        from: 1
                        to: 10
                        stepSize: 1
                        snapMode: Slider.SnapAlways
                        value: point.shown
                        enabled: editor.editable
                        // The limit and the monotonic curve are applied to the model, the handle shows the model value
                        // (a value under the limit does not change the model, the binding would not move the handle back)
                        onMoved: {
                            editor.setPoint(point.index, Math.round(value))
                            value = Qt.binding(() => point.shown)
                        }

                        HoverHandler {
                            id: pointHover
                        }

                        ThemedToolTip {
                            visible: pointHover.hovered && text.length > 0
                            delay: 300
                            text: editor.pointToolTip(point.index)
                        }
                    }

                    Label {
                        id: pointLabel

                        Layout.alignment: Qt.AlignHCenter

                        text: point.pointField !== null ? point.pointField.label : ""
                        color: !editor.editable ? theme.disabledText
                             : point.pointField !== null && point.pointField.modified ? theme.modified : theme.text

                        // Mark of a changed point left of the label, outside of the layout (the label keeps its width)
                        Label {
                            anchors.right: parent.left
                            anchors.rightMargin: 2
                            anchors.verticalCenter: parent.verticalCenter
                            visible: point.pointField !== null && point.pointField.modified
                            text: "●"
                            color: pointLabel.color
                        }
                    }
                }
            }
        }
    }
}
