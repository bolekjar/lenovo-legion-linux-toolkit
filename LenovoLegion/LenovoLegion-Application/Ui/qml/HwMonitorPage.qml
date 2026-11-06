// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI: hardware monitor, the same data and calculations as HWMonitoring of the widget pages (UiDesign.md):
//  - CPU frequency averages (all, P-cores, E-cores) of cpu_scaling_cur_freq,
//  - CPU power from the RAPL energy difference per time, GPU power from NVML (0 when the GPU is suspended),
//  - temperatures (30..110 °C) and fan speeds (min..max + 100 RPM) of the Legion sensors,
//  - data reloaded every 500 ms in a worker thread (DataSource.interval),
//  - check boxes of "CPU Details" and "GPU details" open the detail windows (frequency per thread, NVIDIA GPU),
//    the windows take the data of the page and add samples only while shown.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    id: page

    title: "HW Monitoring (QML preview)"

    readonly property int refreshInterval: 500

    DataSource {
        id: hw

        dataType: "HW_MONITORING"
        interval: page.refreshInterval
    }

    DataSource {
        id: nvml

        dataType: "NVIDIA_NWML"
        interval: page.refreshInterval
    }

    DataSource {
        id: topology

        dataType: "CPU_TOPOLOGY"
    }

    DataSource {
        id: cpuInfo

        dataType: "CPU_INFO"
    }

    function ranges(field) {
        const cpus = []

        for (let range = 0; range < topology.count(field); ++range) {
            const min = topology.value(field + "[" + range + "].cpu_min")
            const max = topology.value(field + "[" + range + "].cpu_max")

            for (let cpu = min; cpu <= max; ++cpu)
                cpus.push(cpu)
        }

        return cpus
    }

    readonly property var performance: (topology.revision, ranges("active_cpus_core"))
    readonly property var efficient:   (topology.revision, ranges("active_cpus_atom"))

    // Range of the frequency charts (MHz): info min/max and scaling min/max of the threads
    function frequencyRange(cpus) {
        const range = { min: Number.MAX_VALUE, max: 0, scaleMin: Number.MAX_VALUE, scaleMax: 0 }

        for (const cpu of cpus) {
            const prefix = "cpux_freq[" + cpu + "]."
            range.min      = Math.min(range.min,      hw.value(prefix + "cpu_info_min_freq") / 1000)
            range.max      = Math.max(range.max,      hw.value(prefix + "cpu_info_max_freq") / 1000)
            range.scaleMin = Math.min(range.scaleMin, hw.value(prefix + "cpu_scaling_min_freq") / 1000)
            range.scaleMax = Math.max(range.scaleMax, hw.value(prefix + "cpu_scaling_max_freq") / 1000)
        }

        return cpus.length > 0 ? range : { min: 0, max: 0, scaleMin: 0, scaleMax: 0 }
    }

    function averageFrequency(cpus) {
        let sum = 0

        for (const cpu of cpus)
            sum += hw.value("cpux_freq[" + cpu + "].cpu_scaling_cur_freq")

        return cpus.length > 0 ? Math.floor(sum / cpus.length / 1000) : 0
    }

    function frequencyText(name, value, range) {
        return name + " Average Frequency: " + value + " MHz\n"
             + name + " Min Frequency: " + range.min + " MHz\n"
             + name + " Max Frequency: " + range.max + " MHz\n"
             + name + " Scale Min Frequency: " + range.scaleMin + " MHz\n"
             + name + " Scale Max Frequency: " + range.scaleMax + " MHz"
    }

    readonly property var performanceRange: (hw.revision, frequencyRange(performance))
    readonly property var efficientRange:   (hw.revision, frequencyRange(efficient))
    readonly property var allRange:         efficient.length > 0 ? { min: Math.min(performanceRange.min, efficientRange.min), max: Math.max(performanceRange.max, efficientRange.max),
                                                                     scaleMin: Math.min(performanceRange.scaleMin, efficientRange.scaleMin), scaleMax: Math.max(performanceRange.scaleMax, efficientRange.scaleMax) }
                                                                 : performanceRange

    // CPU power: energy difference (uJ) per time (us), only for regular samples (0.8..1.2 interval, the same as HWMonitoring)
    property real previousEnergy: -1
    property real previousTime: 0
    property real cpuPower: 0

    function sample() {
        const now = Date.now()
        const energy = hw.value("intel_power.power_cap_cpu_energy")
        const elapsed = (now - previousTime) * 1000

        if (previousEnergy >= 0 && elapsed > refreshInterval * 800 && elapsed < refreshInterval * 1200)
            cpuPower = Math.floor((energy - previousEnergy) / elapsed)

        previousEnergy = energy
        previousTime = now

        cpuAverage.push(averageFrequency(performance.concat(efficient)))
        pAverage.push(averageFrequency(performance))
        eAverage.push(averageFrequency(efficient))
        cpuPowerChart.push(cpuPower)
        gpuPowerChart.push(nvml.value("gpu_suspended") ? 0 : nvml.value("hardware_monitor.power.value") / 1000)

        for (let i = 0; i < temps.count; ++i)
            temps.itemAt(i).push(hw.value("legion.temps[" + i + "].temp_value") / 1000)

        for (let i = 0; i < fans.count; ++i)
            fans.itemAt(i).push(hw.value("legion.fans[" + i + "].fan_speed"))

        if (cpuWindow.open)
            cpuWindow.sample()
    }

    Connections {
        target: hw

        function onMessageChanged() { page.sample() }
    }

    Connections {
        target: nvml

        function onMessageChanged() {
            if (gpuWindow.open)
                gpuWindow.sample()
        }
    }

    CpuFrequencyWindow {
        id: cpuWindow

        // Assignments in both directions, closing the window unchecks the section
        onOpenChanged: cpuSection.checked = open
        hwSource: hw
        infoSource: cpuInfo
        topologySource: topology
        performance: page.performance
        efficient: page.efficient
        range: page.allRange
    }

    GpuDetailsWindow {
        id: gpuWindow

        onOpenChanged: gpuSection.checked = open
        nvmlSource: nvml
    }

    // All groups in one row, the width is divided by the number of the charts (charts of the same width):
    // tiny preferred widths, the whole width is distributed by the stretch factors
    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Section {
            id: cpuSection

            title: "CPU Details"
            Layout.horizontalStretchFactor: 3
            Layout.preferredWidth: 3
            checkable: true
            onCheckedChanged: cpuWindow.open = checked
            Layout.fillWidth: true

            RowLayout {
                HistoryChart {
                    id: cpuAverage
                    Layout.fillWidth: true
                    twoLines: true
                    title: "Avg [%1 MHz]"
                    fieldWidth: 4
                    minimum: page.allRange.min
                    maximum: page.allRange.max
                    tooltip: page.frequencyText("CPU", Math.round(value), page.allRange)
                }

                HistoryChart {
                    id: pAverage
                    Layout.fillWidth: true
                    twoLines: true
                    title: "P-Core [%1 MHz]"
                    fieldWidth: 4
                    minimum: page.performanceRange.min
                    maximum: page.performanceRange.max
                    tooltip: page.frequencyText("P-Core", Math.round(value), page.performanceRange)
                }

                HistoryChart {
                    id: eAverage
                    Layout.fillWidth: true
                    twoLines: true
                    title: "E-Core [%1 MHz]"
                    fieldWidth: 4
                    minimum: page.efficientRange.min
                    maximum: page.efficientRange.max
                    tooltip: page.frequencyText("E-Core", Math.round(value), page.efficientRange)
                }
            }
        }

        Section {
            id: gpuSection

            title: "GPU details"
            Layout.horizontalStretchFactor: 2
            Layout.preferredWidth: 2
            checkable: true
            onCheckedChanged: gpuWindow.open = checked
            Layout.fillWidth: true

            RowLayout {
                HistoryChart {
                    id: cpuPowerChart
                    Layout.fillWidth: true
                    twoLines: true
                    title: "CPU Power [%1 W]"
                    minimum: 0
                    maximum: 250
                    tooltip: (cpuInfo.revision, cpuInfo.value("cpu_model")) + "\nPower : " + Math.round(value) + " W\nPower min: 0 W\nPower max: 250 W"
                }

                HistoryChart {
                    id: gpuPowerChart
                    Layout.fillWidth: true
                    twoLines: true
                    title: "GPU Power [%1 W]"
                    minimum: 0
                    maximum: (nvml.revision, nvml.value("hardware_monitor.power.max_value") / 1000)
                    tooltip: (nvml.revision, nvml.value("gpu_suspended")) ? nvml.value("name") + "\nGPU is suspended (power saving)"
                           : nvml.value("name") + "\nPower: " + Math.round(value) + " W\nPower min: " + Math.round(nvml.value("hardware_monitor.power.min_value") / 1000)
                             + " W\nPower max: " + Math.round(nvml.value("hardware_monitor.power.max_value") / 1000) + " W"
                }
            }
        }

        Section {
            title: "Temperatures"
            visible: temps.count > 0
            Layout.horizontalStretchFactor: Math.max(temps.count, 1)
            Layout.preferredWidth: Math.max(temps.count, 1)
            Layout.fillWidth: true

            RowLayout {
                Repeater {
                    id: temps

                    model: (hw.revision, hw.count("legion.temps"))

                    delegate: HistoryChart {
                        required property int index

                        readonly property string label: hw.value("legion.temps[" + index + "].temp_label")

                        Layout.fillWidth: true

                        twoLines: true
                        title: label + " [%1 °C]"
                        minimum: 30
                        maximum: 110
                        warning: 75
                        critical: 90
                        tooltip: label + ": " + Math.round(value) + " °C\n" + label + " min: 30 °C\n" + label + " max: 100 °C"
                    }
                }
            }
        }

        Section {
            title: "FANs"
            visible: fans.count > 0
            Layout.horizontalStretchFactor: Math.max(fans.count, 1)
            Layout.preferredWidth: Math.max(fans.count, 1)
            Layout.fillWidth: true

            RowLayout {
                Repeater {
                    id: fans

                    model: (hw.revision, hw.count("legion.fans"))

                    delegate: HistoryChart {
                        required property int index

                        readonly property string label: hw.value("legion.fans[" + index + "].fan_label")
                        readonly property int    speedMin: hw.value("legion.fans[" + index + "].fan_speed_min")
                        readonly property int    speedMax: hw.value("legion.fans[" + index + "].fan_speed_max") + 100

                        Layout.fillWidth: true

                        twoLines: true
                        title: label + " [%1 RPM]"
                        minimum: speedMin
                        maximum: speedMax
                        // High speed is only loud, not a failure: warning only
                        warning: speedMin + (speedMax - 100 - speedMin) * 0.85
                        tooltip: "Fan " + label + " Speed: " + Math.round(value) + " RPM\nFan " + label + " Speed min: " + speedMin + " RPM\nFan " + label + " Speed max: " + speedMax + " RPM"
                    }
                }
            }
        }
    }
}
