// SPDX-License-Identifier: GPL-2.0
// Battery in the status bar of the window (Shell.qml): icon with the charge level (red / orange when low, green when
// charging, plug on AC), charge in %, state with the time to full / left; on hover a card with the details (energy,
// health, power, voltage, cycles, power source, model). Data of BATTERY, read again every refreshInterval ms and on
// the notifications of the daemon (battery change, AC adapter plugged / unplugged).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

Item {
    id: indicator

    property int refreshInterval: 5000

    implicitWidth: row.implicitWidth
    implicitHeight: row.implicitHeight

    DataSource {
        id: battery

        dataType: "BATTERY"
        interval: indicator.refreshInterval
    }

    function value(path) {
        return battery.available ? battery.value(path, battery.revision) : undefined
    }

    function number(path) {
        const result = value(path)
        return result !== undefined && result !== null ? Number(result) : 0
    }

    readonly property bool   supported:   battery.available && value("supported") === true
    readonly property string status:      supported ? String(value("baterry_status") ?? "").trim() : ""
    readonly property int    capacity:    number("capacity")
    readonly property bool   hasCapacity: capacity > 0 || number("energy_full") > 0
    readonly property bool   charging:    status === "Charging"
    readonly property bool   discharging: status === "Discharging"
    readonly property bool   acOnline:    value("ac_online") === true || (value("charge_mode_supported") === true && number("current_charge_mode_value") === 1)

    readonly property real energyNow:    number("energy_now") / 1e6           // Wh
    readonly property real energyFull:   number("energy_full") / 1e6
    readonly property real energyDesign: number("energy_full_design") / 1e6
    readonly property real power:        number("power_now") / 1e6            // W
    readonly property real voltage:      number("voltage_now") / 1e6          // V

    // Hours to full / left by the current power (none without the power)
    readonly property real hours: power <= 0.5 ? -1
                                : charging ? Math.max(0, energyFull - energyNow) / power
                                : discharging ? energyNow / power : -1

    function duration(hours) {
        if (hours < 0)
            return ""

        const minutes = Math.round(hours * 60)
        return minutes >= 60 ? Math.floor(minutes / 60) + " h " + (minutes % 60) + " min" : minutes + " min"
    }

    readonly property string stateText: !supported ? "No battery"
        : iconState === "charging" ? (hours >= 0 ? "Charging, " + duration(hours) + " to full" : "Charging")
        : iconState === "full" ? "Fully charged"
        : iconState === "plugged" ? "Plugged in, not charging"
        : iconState === "discharging" ? (critical ? "Battery low, " : "") + (hours >= 0 ? duration(hours) + " left" : "On battery")
        : status.length > 0 ? status : "Unknown"

    // State of the icon: charging (bolt), full (check mark), plugged in without charging (plug, e.g. conservation
    // mode), discharging (level, low orange / critical red, blinking), unknown (question mark)
    readonly property string iconState: !supported ? "unknown"
        : charging ? "charging"
        : status === "Full" || (acOnline && capacity >= 100) ? "full"
        : acOnline || status === "Not charging" ? "plugged"
        : discharging ? "discharging" : "unknown"

    // Plugged in without charging: blue (not the accent, the dark theme has an orange accent like a low battery)
    readonly property color pluggedColor: "#58a6ff"

    readonly property bool critical: iconState === "discharging" && capacity > 0 && capacity <= 10

    readonly property color levelColor: iconState === "charging" || iconState === "full" ? "#3fb950"
                                      : iconState === "plugged" ? indicator.pluggedColor
                                      : capacity > 0 && capacity <= 15 ? "#e5534b"
                                      : capacity > 0 && capacity <= 30 ? "#f0a020" : theme.text

    readonly property string health: energyDesign > 0 && energyFull > 0 ? Math.round(100 * energyFull / energyDesign) + " %" : ""

    RowLayout {
        id: row

        anchors.fill: parent
        spacing: 8

        // Battery: body with the level and the terminal, a sign of the state inside (bolt, check mark, question mark)
        Canvas {
            id: icon

            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 32
            implicitHeight: 16

            readonly property color  outline: theme.text
            readonly property color  fill: indicator.levelColor
            readonly property color  back: theme.groupBackground
            readonly property real   level: indicator.iconState === "full" ? 1
                                          : indicator.hasCapacity ? Math.max(0, Math.min(100, indicator.capacity)) / 100 : 0
            readonly property string sign: indicator.iconState

            onOutlineChanged: requestPaint()
            onFillChanged: requestPaint()
            onBackChanged: requestPaint()
            onLevelChanged: requestPaint()
            onSignChanged: requestPaint()

            // Critical charge: blinking
            SequentialAnimation on opacity {
                running: indicator.critical
                loops: Animation.Infinite
                onStopped: icon.opacity = 1

                NumberAnimation { to: 0.35; duration: 700; easing.type: Easing.InOutQuad }
                NumberAnimation { to: 1.0;  duration: 700; easing.type: Easing.InOutQuad }
            }

            onPaint: {
                const ctx = getContext("2d")
                const bodyWidth = width - 4.5
                ctx.reset()

                // Body and terminal
                ctx.lineWidth = 1.5
                ctx.strokeStyle = outline
                ctx.beginPath()
                ctx.roundedRect(0.75, 0.75, bodyWidth, height - 1.5, 3, 3)
                ctx.stroke()
                ctx.fillStyle = outline
                ctx.fillRect(width - 3, height / 2 - 3, 2.5, 6)

                // Level
                if (level > 0) {
                    ctx.fillStyle = fill
                    ctx.beginPath()
                    ctx.roundedRect(3, 3, Math.max(2, (bodyWidth - 4.5) * level), height - 6, 1.5, 1.5)
                    ctx.fill()
                }

                // Sign of the state in the middle of the body, with an outline of the background (visible on the level)
                const x = p => 0.75 + bodyWidth * p
                const y = p => height * p

                function strokeTwice(width) {
                    ctx.lineWidth = width + 2.5
                    ctx.strokeStyle = back
                    ctx.stroke()
                    ctx.lineWidth = width
                    ctx.strokeStyle = sign === "unknown" ? outline : "white"
                    ctx.stroke()
                }

                ctx.lineCap = "round"
                ctx.lineJoin = "round"

                if (sign === "charging") {
                    ctx.beginPath()
                    ctx.moveTo(x(0.56), y(0.12))
                    ctx.lineTo(x(0.36), y(0.56))
                    ctx.lineTo(x(0.52), y(0.56))
                    ctx.lineTo(x(0.44), y(0.88))
                    ctx.lineTo(x(0.66), y(0.42))
                    ctx.lineTo(x(0.5), y(0.42))
                    ctx.closePath()
                    ctx.lineWidth = 2.5
                    ctx.strokeStyle = back
                    ctx.stroke()
                    ctx.fillStyle = "white"
                    ctx.fill()
                } else if (sign === "full") {
                    ctx.beginPath()
                    ctx.moveTo(x(0.34), y(0.52))
                    ctx.lineTo(x(0.46), y(0.72))
                    ctx.lineTo(x(0.68), y(0.3))
                    strokeTwice(1.8)
                } else if (sign === "unknown") {
                    ctx.beginPath()
                    ctx.arc(x(0.5), y(0.38), height * 0.17, Math.PI, 2.2 * Math.PI)
                    ctx.lineTo(x(0.5), y(0.62))
                    strokeTwice(1.4)
                    ctx.beginPath()
                    ctx.arc(x(0.5), y(0.8), 0.9, 0, 2 * Math.PI)
                    ctx.fillStyle = outline
                    ctx.fill()
                }
            }
        }

        // Plugged in without charging: plug
        TintedIcon {
            Layout.alignment: Qt.AlignVCenter
            visible: indicator.iconState === "plugged"
            Layout.preferredWidth: 14
            Layout.preferredHeight: 14
            source: "qrc:/ui/icons/plug.svg"
            color: indicator.pluggedColor
        }

        Label {
            Layout.alignment: Qt.AlignVCenter
            visible: indicator.supported && indicator.hasCapacity
            text: indicator.capacity + " %"
            font.bold: true
            color: theme.text
        }

        Label {
            Layout.alignment: Qt.AlignVCenter
            text: indicator.stateText
            color: theme.disabledText
        }
    }

    HoverHandler {
        id: hover
    }

    // Details on hover, above the status bar
    Popup {
        id: details

        visible: hover.hovered && indicator.supported
        x: indicator.width - width
        y: -height - 10
        width: 320
        padding: 14
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            radius: theme.radius
            color: theme.groupBackground
            border.width: theme.styled ? 2 : 1
            border.color: theme.accent
        }

        contentItem: ColumnLayout {
            spacing: 10

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    text: "Battery"
                    font.bold: true
                    font.pixelSize: theme.fontPixelSize + 2
                    color: theme.text
                }

                Label {
                    visible: indicator.hasCapacity
                    text: indicator.capacity + " %"
                    font.bold: true
                    font.pixelSize: theme.fontPixelSize + 6
                    color: theme.text
                }
            }

            // Charge level bar
            Rectangle {
                Layout.fillWidth: true
                visible: indicator.hasCapacity
                implicitHeight: 8
                radius: 4
                color: theme.window
                border.width: 1
                border.color: theme.groupBorder

                Rectangle {
                    width: Math.max(height, parent.width * Math.min(100, indicator.capacity) / 100)
                    height: parent.height
                    radius: 4
                    color: indicator.levelColor === theme.text ? theme.accent : indicator.levelColor
                }
            }

            Label {
                Layout.fillWidth: true
                text: indicator.stateText
                color: theme.text
                wrapMode: Text.Wrap
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 4

                Repeater {
                    model: [
                        { label: "Status",       value: indicator.status },
                        { label: "Power source", value: indicator.acOnline ? "AC adapter" : "Battery" },
                        { label: "Energy",       value: indicator.energyFull > 0 ? indicator.energyNow.toFixed(1) + " Wh of " + indicator.energyFull.toFixed(1) + " Wh" : "" },
                        { label: "Health",       value: indicator.health.length > 0 ? indicator.health + " (design " + indicator.energyDesign.toFixed(1) + " Wh)" : "" },
                        { label: "Power",        value: indicator.power > 0 ? indicator.power.toFixed(1) + " W" + (indicator.charging ? " in" : indicator.discharging ? " out" : "") : "" },
                        { label: "Voltage",      value: indicator.voltage > 0 ? indicator.voltage.toFixed(2) + " V" : "" },
                        { label: "Cycles",       value: indicator.number("cycle_count") > 0 ? String(indicator.number("cycle_count")) : "" },
                        { label: "Level",        value: String(indicator.value("capacity_level") ?? "") },
                        { label: "Technology",   value: String(indicator.value("technology") ?? "") },
                        { label: "Model",        value: [String(indicator.value("manufacturer") ?? "").trim(), String(indicator.value("model_name") ?? "").trim()].filter(part => part.length > 0).join(" ") }
                    ].filter(row => row.value.length > 0)

                    delegate: Item {
                        id: detail

                        required property var modelData

                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        implicitHeight: detailRow.implicitHeight

                        RowLayout {
                            id: detailRow

                            anchors.fill: parent
                            spacing: 16

                            Label {
                                Layout.preferredWidth: 96
                                text: detail.modelData.label
                                color: theme.disabledText
                            }

                            Label {
                                Layout.fillWidth: true
                                text: detail.modelData.value
                                font.bold: true
                                color: theme.text
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                }
            }
        }
    }
}
