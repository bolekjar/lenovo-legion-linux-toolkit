// SPDX-License-Identifier: GPL-2.0
// Fan curve the same as FanControl of the widget pages:
//  - Maximum / Custom: fan full speed (FanOption), only in the custom thermal mode when supported,
//  - 10 points, level 1..10 in the limits of the firmware (min_value / max_value of FanCurve, without them min / max
//    of FanControl.proto), a point dragged
//    under its limit stays at the limit, the curve stays monotonic when a point is moved,
//  - stretched to the size of the page (sliders over the whole height, points over the whole width),
//  - edited only in the custom thermal mode with Custom, otherwise shown: 1..10 (not custom mode), all 10 (Maximum),
//  - axes of the firmware: a point is labeled by the sensor temperatures of the default tables of the power profile
//    (the tables complement each other, 16IAX10H: points 1..4 by the SYS sensor, 5..10 by the CPU and GPU),
//    the label of FanControl.proto without them,
//    the fan speed axis shows the levels (the fan speeds are not linear and differ by the fan),
//  - tool tip of a point: temperatures of the sensors at the point (default tables of the power profile, 127 = N/A)
//    and the fan speeds of its level (the firmware runs all fans at the level of the point, fan table index level - 1),
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
    readonly property bool custom:             profileSource !== null && profileSource.revision >= 0 && profileSource.value("thermal_mode", profileSource.revision) === 255
    readonly property bool fullSpeedSupported: fanSource !== null && fanSource.revision >= 0 && fanSource.value("full_speed.supported", fanSource.revision) === true
    // value() is not tracked: the revision of the source is a dependency of the binding
    readonly property bool fullSpeed:          fullSpeedSupported && fanSource.revision >= 0 && fanSource.value("full_speed.current_value", fanSource.revision) === true
    readonly property bool editable:           custom && !fullSpeed
    readonly property int  profile:            profileSource !== null && profileSource.revision >= 0 ? profileSource.value("current_value", profileSource.revision) : 0

    function pointModel(index) {
        return curve.field("current_value." + points[index], curve.revision)
    }

    // Limits of the firmware (the EC does not check them, the default curve 1..10 may only be raised), otherwise
    // min / max of the field
    readonly property var minLevels: (curve.repeatedValues("min_value", curve.revision))
    readonly property var maxLevels: (curve.repeatedValues("max_value", curve.revision))

    function minLevel(index) {
        return minLevels.length === points.length ? minLevels[index] : pointModel(index).min
    }

    function maxLevel(index) {
        return maxLevels.length === points.length ? maxLevels[index] : pointModel(index).max
    }

    function clampLevel(index, value) {
        return Math.max(minLevel(index), Math.min(maxLevel(index), value))
    }

    /*
     * Monotonic curve: later points are raised to the value, earlier points are lowered to it (normalizeCurrentFanCurveControlData),
     * every point stays in its limits
     */
    function setPoint(index, value) {
        const level = clampLevel(index, value)

        curve.beginChanges()   // one notification for all points

        for (let i = 0; i < points.length; ++i) {
            const model = pointModel(i)
            const current = model.value

            if (i === index)
                model.value = level
            else if (i > index && current < level)
                model.value = clampLevel(i, level)
            else if (i < index && current > level)
                model.value = clampLevel(i, level)
        }

        curve.endChanges()
    }

    // Default tables (fan speeds, sensor temperatures) of the tool tip and the labels of the points
    readonly property var defaultCurves: [["cpu_default", "CPU"], ["gpu_default", "GPU"], ["sys_default", "SYS"], ["cpusen_default", "CPU sensor"]]

    /*
     * Label of a point: temperatures of the sensors of the default tables of the power profile (127 = N/A), one line
     * each. The label of the field without the tables (older firmware).
     */
    function pointLabel(index) {
        const lines = []

        for (const [map, name] of defaultCurves) {
            const sensors = curve.repeatedValues(map + "[" + profile + "].sensors", curve.revision)

            if (sensors.length === points.length)
                lines.push(name + " " + (sensors[index] === 127 ? "N/A" : sensors[index] + " ℃"))
        }

        if (lines.length === 0) {
            const field = pointModel(index)
            return field !== null ? field.label : ""
        }

        return lines.join("\n")
    }

    // Lines of the longest label: all labels have its height, the sliders of the points stay aligned with the axis
    readonly property int labelLines: {
        let lines = 1

        for (let i = 0; i < points.length; ++i)
            lines = Math.max(lines, pointLabel(i).split("\n").length)

        return lines
    }

    FontMetrics {
        id: labelMetrics

        font.pixelSize: theme.fontPixelSize   // font of the labels (SettingsPage)
    }

    function pointToolTip(index, level) {
        const temperatures = []
        const speeds = []

        for (const [map, name] of defaultCurves) {
            const sensors = (curve.repeatedValues(map + "[" + profile + "].sensors", curve.revision))
            const fans    = curve.repeatedValues(map + "[" + profile + "].fan", curve.revision)

            if (sensors.length > index)
                temperatures.push(name + " temp: " + (sensors[index] === 127 ? "N/A" : sensors[index]) + " ℃")
            if (level >= 1 && fans.length >= level)
                speeds.push(name + " fan speed: " + fans[level - 1] + " rpm")
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
            onClicked: editor.fanSource.field("full_speed.current_value", editor.fanSource.revision).value = true
        }

        ThemedButton {
            Layout.fillWidth: true

            text: "Custom"
            checkable: true
            autoExclusive: true
            checked: !editor.fullSpeed
            enabled: editor.custom && editor.fullSpeedSupported
            onClicked: editor.fanSource.field("full_speed.current_value", editor.fanSource.revision).value = false
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

            // Fan speed axis: levels at the handles of the sliders (top 10, bottom 1)
            Item {
                id: axis

                readonly property Item track: pointRepeater.count > 0 && pointRepeater.itemAt(0) !== null ? pointRepeater.itemAt(0).track : null

                Layout.fillHeight: true
                implicitWidth: axisMetrics.width

                TextMetrics {
                    id: axisMetrics

                    text: "10"
                }

                Repeater {
                    model: [10, 7, 4, 1]

                    delegate: Label {
                        required property int modelData

                        readonly property real handle: 22   // handle of ThemedSlider

                        visible: axis.track !== null
                        y: axis.track === null ? 0
                         : axis.track.topPadding + handle / 2 + (10 - modelData) / 9 * (axis.track.availableHeight - handle) - height / 2
                        width: axis.width
                        horizontalAlignment: Text.AlignRight
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

                    readonly property FieldModel pointField: (editor.pointModel(index))
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
                            text: editor.pointToolTip(point.index, point.shown)
                        }
                    }

                    Label {
                        id: pointLabel

                        Layout.alignment: Qt.AlignHCenter

                        Layout.preferredHeight: labelMetrics.lineSpacing * editor.labelLines

                        text: editor.pointLabel(point.index)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignTop
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
