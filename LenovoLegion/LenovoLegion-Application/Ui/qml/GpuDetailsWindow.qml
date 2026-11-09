// SPDX-License-Identifier: GPL-2.0
// Detail window of the HW monitor (GPU Details, NVIDIA NVML):
//  - GPU information: state, memory, PCIe link (current and maximum), power limits, temperature thresholds,
//    clock ranges and offsets,
//  - live tiles: clocks, utilization, memory, temperature, power, PCIe throughput,
//  - charts of the last 100 values grouped by clocks, load and memory, thermal and power, PCIe, next to them
//    the specifications.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

DetailWindow {
    id: window

    required property DataSource nvmlSource

    title: "GPU Details"
    width: 1200
    height: 900
    minimumWidth: 900
    minimumHeight: 600

    function value(path) { return window.nvmlSource.value(path, window.nvmlSource.revision) }
    function hm(path)    { return value("hardware_monitor." + path) }

    readonly property bool suspended: value("gpu_suspended") === true

    // PCIe GB/s per lane by the generation, theoretical maximum of the link in MB/s
    readonly property var  pcieLane: [0, 0.25, 0.5, 0.984, 1.969, 3.938, 7.877]
    function pcieThroughput(generation, width) { return Math.floor((pcieLane[generation] ?? 0) * width * 1024) }

    readonly property real pcieMax:      pcieThroughput(hm("pcie.generation_max"), hm("pcie.width_max"))
    readonly property real memoryTotal:  hm("memory_use.total") / (1024 * 1024)            // MB
    readonly property real powerLimit:   (hm("power.enforced_value") > 0 ? hm("power.enforced_value") : hm("power.max_value")) / 1000
    readonly property int  slowdown:     hm("temperature.slowdown")
    readonly property int  shutdown:     hm("temperature.shutdown")
    readonly property int  chartHeight:  150
    readonly property int  infoWidth:    380      // specifications next to the charts

    // Live values of the tiles (sample())
    property real memoryUsedNow: 0

    function sample() {
        coreClock.push(hm("gpu_clock.value"))
        memoryClock.push(hm("memory_clock.value"))
        smClock.push(hm("sm_clock.value"))
        gpuUtilization.push(hm("gpu_utilization.value"))
        memoryUtilization.push(hm("memory_utilization.value"))
        memoryUsedNow = hm("memory_use.used") / (1024 * 1024)
        memoryUsed.push(memoryUsedNow)
        temperature.push(hm("temperature.value"))
        power.push(hm("power.value") / 1000)
        pcieRx.push(Math.floor(hm("pcie.rx_bytes") / 1024))
        pcieTx.push(Math.floor(hm("pcie.tx_bytes") / 1024))
    }

    function gb(megabytes) { return (megabytes / 1024).toFixed(1) }

    function temperatureLevel(value) {
        if (slowdown > 0 && value >= slowdown)
            return "critical"

        return slowdown > 0 && value >= slowdown - 10 ? "warning" : "normal"
    }

    function range(path, unit) {
        return hm(path + ".min_value") + " – " + hm(path + ".max_value") + " " + unit
    }

    function offset(path) {
        const offset = value(path + ".value")

        return offset === undefined ? "" : (offset > 0 ? "+" : "") + offset + " MHz"
    }

    // Name and state of the GPU
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: heroLayout.implicitHeight + 24
        radius: theme.radius
        color: theme.groupBackground
        border.width: 1
        border.color: theme.groupBorder

        RowLayout {
            id: heroLayout

            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: String(window.value("name") ?? "NVIDIA GPU")
                    font.pixelSize: theme.fontPixelSize + 8
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    color: theme.disabledText
                    elide: Text.ElideRight
                    text: window.gb(window.memoryTotal) + " GB VRAM · PCIe Gen " + window.hm("pcie.generation_max") + " x" + window.hm("pcie.width_max")
                          + " · power limit " + Math.round(window.powerLimit) + " W"
                }
            }

            // State chip: active, or suspended (runtime power management, the values are not updated)
            Rectangle {
                implicitWidth: stateLabel.implicitWidth + 20
                implicitHeight: stateLabel.implicitHeight + 8
                radius: height / 2
                color: window.suspended ? theme.disabledBackground : theme.accent

                Label {
                    id: stateLabel

                    anchors.centerIn: parent
                    text: window.suspended ? "Suspended" : "Active"
                    font.bold: true
                    color: window.suspended ? theme.disabledText : theme.accentText
                }

                HoverHandler { id: stateHover }

                ThemedToolTip {
                    visible: stateHover.hovered
                    text: window.suspended ? "The GPU is suspended (runtime power management), the values are not updated"
                                           : "The GPU is active"
                }
            }
        }
    }

    // Live values
    Section {
        title: "Now"

        GridLayout {
            Layout.fillWidth: true
            columns: Math.max(1, Math.floor((width + columnSpacing) / (140 + columnSpacing)))
            columnSpacing: 8
            rowSpacing: 8

            StatTile {
                Layout.fillWidth: true
                active: !window.suspended
                label: "Core clock"
                value: Math.round(coreClock.value)
                unit: "MHz"
                detail: "of " + window.hm("gpu_clock.max_value") + " MHz"
                fraction: coreClock.value / Math.max(window.hm("gpu_clock.max_value"), 1)
            }

            StatTile {
                Layout.fillWidth: true
                active: !window.suspended
                label: "Memory clock"
                value: Math.round(memoryClock.value)
                unit: "MHz"
                detail: "of " + window.hm("memory_clock.max_value") + " MHz"
                fraction: memoryClock.value / Math.max(window.hm("memory_clock.max_value"), 1)
            }

            StatTile {
                Layout.fillWidth: true
                active: !window.suspended
                label: "GPU load"
                value: Math.round(gpuUtilization.value)
                unit: "%"
                detail: "memory controller " + Math.round(memoryUtilization.value) + " %"
                fraction: gpuUtilization.value / 100
            }

            StatTile {
                Layout.fillWidth: true
                active: !window.suspended
                label: "VRAM used"
                value: window.gb(window.memoryUsedNow)
                unit: "GB"
                detail: "of " + window.gb(window.memoryTotal) + " GB"
                fraction: window.memoryTotal > 0 ? window.memoryUsedNow / window.memoryTotal : NaN
                level: window.memoryTotal > 0 && window.memoryUsedNow / window.memoryTotal > 0.9 ? "warning" : "normal"
            }

            StatTile {
                Layout.fillWidth: true
                active: !window.suspended
                label: "Temperature"
                value: Math.round(temperature.value)
                unit: "°C"
                detail: window.slowdown > 0 ? "slowdown at " + window.slowdown + " °C" : ""
                fraction: window.shutdown > 0 ? temperature.value / window.shutdown : NaN
                level: window.temperatureLevel(temperature.value)
                tooltip: "Slowdown threshold " + window.slowdown + " °C, shutdown threshold " + window.shutdown + " °C"
            }

            StatTile {
                Layout.fillWidth: true
                active: !window.suspended
                label: "Power"
                value: Math.round(power.value)
                unit: "W"
                detail: "of " + Math.round(window.powerLimit) + " W limit"
                fraction: window.powerLimit > 0 ? power.value / window.powerLimit : NaN
                level: window.powerLimit > 0 && power.value >= window.powerLimit * 0.95 ? "warning" : "normal"
                tooltip: "Enforced power limit " + Math.round(window.hm("power.enforced_value") / 1000) + " W, default "
                         + Math.round(window.hm("power.default_value") / 1000) + " W"
            }

            StatTile {
                Layout.fillWidth: true
                active: !window.suspended
                label: "PCIe RX / TX"
                value: Math.round(pcieRx.value) + " / " + Math.round(pcieTx.value)
                unit: "MB/s"
                detail: "link Gen " + window.hm("pcie.generation") + " x" + window.hm("pcie.width")
                fraction: window.pcieMax > 0 ? (pcieRx.value + pcieTx.value) / window.pcieMax : NaN
                tooltip: "Current PCIe link Gen " + window.hm("pcie.generation") + " x" + window.hm("pcie.width") + " (the link slows down when idle),"
                         + " maximum Gen " + window.hm("pcie.generation_max") + " x" + window.hm("pcie.width_max") + " = " + window.pcieMax + " MB/s"
            }
        }
    }

    // Charts of one group in one row
    component ChartRow: RowLayout {
        Layout.fillWidth: true
        spacing: 6
    }

    // Charts and next to them the specifications
    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 12

            Section {
                title: "Clocks"

                ChartRow {
                    HistoryChart {
                        id: coreClock
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Core [%1 MHz]"
                        maximum: window.hm("gpu_clock.max_value")
                        tooltip: "Core clock: " + Math.round(value) + " MHz\nRange: " + window.range("gpu_clock", "MHz")
                    }

                    HistoryChart {
                        id: smClock
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "SM [%1 MHz]"
                        maximum: window.hm("sm_clock.max_value")
                        tooltip: "Streaming multiprocessor clock: " + Math.round(value) + " MHz\nRange: " + window.range("sm_clock", "MHz")
                    }

                    HistoryChart {
                        id: memoryClock
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Memory [%1 MHz]"
                        maximum: window.hm("memory_clock.max_value")
                        tooltip: "Memory clock: " + Math.round(value) + " MHz\nRange: " + window.range("memory_clock", "MHz")
                    }
                }
            }

            Section {
                title: "Load and memory"

                ChartRow {
                    HistoryChart {
                        id: gpuUtilization
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "GPU load [%1 %]"
                        maximum: 100
                        tooltip: "GPU utilization: " + Math.round(value) + " % (time a kernel was running)"
                    }

                    HistoryChart {
                        id: memoryUtilization
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Memory controller [%1 %]"
                        maximum: 100
                        tooltip: "Memory utilization: " + Math.round(value) + " % (time the memory was read or written)"
                    }

                    HistoryChart {
                        id: memoryUsed
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "VRAM used [%1 MB]"
                        maximum: window.memoryTotal
                        warning: window.memoryTotal * 0.9
                        tooltip: "Memory used: " + Math.round(value) + " MB of " + Math.round(window.memoryTotal) + " MB"
                    }
                }
            }

            Section {
                title: "Thermal, power and PCIe"

                ChartRow {
                    HistoryChart {
                        id: temperature
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Temperature [%1 °C]"
                        // Slowdown threshold of NVML: critical, 10 °C under it warning
                        warning: window.slowdown > 0 ? window.slowdown - 10 : NaN
                        critical: window.slowdown > 0 ? window.slowdown : NaN
                        maximum: window.shutdown > 0 ? window.shutdown : 100
                        tooltip: "Temperature: " + Math.round(value) + " °C\nSlowdown: " + window.slowdown + " °C\nShutdown: " + window.shutdown + " °C"
                    }

                    HistoryChart {
                        id: power
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Power [%1 W]"
                        maximum: Math.floor(window.hm("power.max_value") / 1000)
                        warning: window.powerLimit * 0.95
                        tooltip: "Power: " + Math.round(value) + " W\nLimit: " + Math.round(window.powerLimit) + " W\nRange: "
                                 + Math.floor(window.hm("power.min_value") / 1000) + " – " + Math.floor(window.hm("power.max_value") / 1000) + " W"
                    }

                    HistoryChart {
                        id: pcieRx
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "PCIe RX [%1 MB/s]"
                        maximum: window.pcieMax
                        tooltip: "PCIe receive throughput: " + Math.round(value) + " MB/s of " + window.pcieMax + " MB/s"
                    }

                    HistoryChart {
                        id: pcieTx
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "PCIe TX [%1 MB/s]"
                        maximum: window.pcieMax
                        tooltip: "PCIe transmit throughput: " + Math.round(value) + " MB/s of " + window.pcieMax + " MB/s"
                    }
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: window.infoWidth
            Layout.maximumWidth: window.infoWidth
            Layout.alignment: Qt.AlignTop
            spacing: 12

            InfoSection {
                Layout.fillWidth: true

                title: "Clocks and memory"
                onCopyRequested: (label, value) => window.copy(value)

                rows: [
                    { label: "Core clock",          value: window.range("gpu_clock", "MHz") },
                    { label: "SM clock",            value: window.range("sm_clock", "MHz") },
                    { label: "Memory clock",        value: window.range("memory_clock", "MHz") },
                    { label: "Core clock offset",   value: window.offset("gpu_offset"),    hint: "GPU page" },
                    { label: "Memory clock offset", value: window.offset("memory_offset"), hint: "GPU page" },
                    { label: "VRAM",                value: window.gb(window.memoryTotal) + " GB (" + Math.round(window.memoryTotal) + " MB)" }
                ]
            }

            InfoSection {
                Layout.fillWidth: true

                title: "Power, thermal and PCIe"
                onCopyRequested: (label, value) => window.copy(value)

                rows: [
                    { label: "Power limit",       value: Math.round(window.hm("power.enforced_value") / 1000) + " W (default " + Math.round(window.hm("power.default_value") / 1000) + " W)",
                      hint: "Enforced power limit of the driver and its default value" },
                    { label: "Power range",       value: Math.floor(window.hm("power.min_value") / 1000) + " – " + Math.floor(window.hm("power.max_value") / 1000) + " W" },
                    { label: "Slowdown at",       value: window.slowdown + " °C" },
                    { label: "Shutdown at",       value: window.shutdown + " °C" },
                    { label: "PCIe link",         value: "Gen " + window.hm("pcie.generation") + " x" + window.hm("pcie.width") + " (max Gen " + window.hm("pcie.generation_max") + " x" + window.hm("pcie.width_max") + ")",
                      hint: "The link slows down when the GPU is idle" },
                    { label: "PCIe max throughput", value: window.pcieMax + " MB/s" }
                ]
            }
        }
    }
}
