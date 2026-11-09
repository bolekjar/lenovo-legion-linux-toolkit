// SPDX-License-Identifier: GPL-2.0
// Detail window of the HW monitor (CPU Details):
//  - live tiles: average frequency of all / P / E threads, the fastest and the slowest thread, CPU power and
//    temperature (values of the page),
//  - frequency chart of every active thread, P- and E-threads in their own groups (columns by the width), next to
//    them the processor and cache information (CPU_INFO, CPU_TOPOLOGY), frequency ranges of the P- and E-threads.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

DetailWindow {
    id: window

    required property DataSource hwSource
    required property DataSource infoSource
    required property DataSource topologySource
    required property var        performance   // CPU numbers of the P-threads
    required property var        efficient     // CPU numbers of the E-threads
    required property var        range         // frequency range of all threads (min, max, scaleMin, scaleMax)
    property real                cpuPower: 0   // W, computed by the page (RAPL energy)
    property real                cpuPowerMax: 250

    readonly property var  threads: performance.concat(efficient).sort((a, b) => a - b)
    readonly property bool hybrid:  efficient.length > 0

    // Width of a thread chart, the columns of the thread groups follow the width of the window;
    // width of the processor and cache information next to the charts
    readonly property int  chartWidth: 200
    readonly property int  infoWidth:  380

    title: "CPU Details"
    width: 1280
    height: 1000
    minimumWidth: 900
    minimumHeight: 600

    // Live values, updated by sample()
    property var current: ({})          // CPU number -> MHz
    property int averageAll: 0
    property int averageP: 0
    property int averageE: 0
    property int fastest: -1
    property int slowest: -1
    property real temperature: NaN

    function info(path)     { return window.infoSource.value(path, window.infoSource.revision) }
    function text(path)     { const value = info(path); return value === undefined || value === null ? "" : String(value).trim() }
    function hw(cpu, field) { return window.hwSource.value("cpux_freq[" + cpu + "]." + field, window.hwSource.revision) / 1000 }

    function average(cpus) {
        let sum = 0

        for (const cpu of cpus)
            sum += current[cpu] ?? 0

        return cpus.length > 0 ? Math.round(sum / cpus.length) : 0
    }

    // Index of the CPU temperature of the Legion sensors (label "CPU"), -1 = none
    function cpuTemperatureIndex() {
        for (let i = 0; i < window.hwSource.count("legion.temps", window.hwSource.revision); ++i)
            if (String(window.hwSource.value("legion.temps[" + i + "].temp_label", window.hwSource.revision)).toUpperCase().startsWith("CPU"))
                return i

        return -1
    }

    // New sample of every thread, only while the window is shown (HwMonitorPage)
    function sample() {
        const values = {}
        let fast = -1
        let slow = -1

        for (const cpu of threads) {
            values[cpu] = Math.floor(hw(cpu, "cpu_scaling_cur_freq"))

            if (fast < 0 || values[cpu] > values[fast])
                fast = cpu

            if (slow < 0 || values[cpu] < values[slow])
                slow = cpu
        }

        current = values
        averageAll = average(threads)
        averageP = average(performance)
        averageE = average(efficient)
        fastest = fast
        slowest = slow

        const temp = cpuTemperatureIndex()
        temperature = temp >= 0 ? window.hwSource.value("legion.temps[" + temp + "].temp_value", window.hwSource.revision) / 1000 : NaN

        for (const group of [performanceCharts, efficientCharts])
            for (let i = 0; i < group.count; ++i)
                group.itemAt(i).sample()
    }

    function kindName(cpu) {
        return !hybrid ? "Thread" : (efficient.indexOf(cpu) >= 0 ? "E-thread" : "P-thread")
    }

    // Range "min – max MHz (base X MHz)" of the threads of a kind (info limits of the hardware)
    function rangeText(cpus) {
        if (cpus.length === 0)
            return ""

        let min = Number.MAX_VALUE
        let max = 0

        for (const cpu of cpus) {
            min = Math.min(min, hw(cpu, "cpu_info_min_freq"))
            max = Math.max(max, hw(cpu, "cpu_info_max_freq"))
        }

        const base = hw(cpus[0], "cpu_base_freq")

        return min + " – " + max + " MHz" + (base > 0 ? " (base " + base + " MHz)" : "")
    }

    // "36864K" -> "36 MB", "48K" -> "48 KB"
    function cacheSize(text) {
        const match = String(text).trim().match(/^(\d+)\s*([KMG]?)/i)

        if (match === null)
            return String(text)

        let kb = Number(match[1]) * ({ "": 1 / 1024, "K": 1, "M": 1024, "G": 1024 * 1024 })[match[2].toUpperCase()]

        return kb >= 1024 ? (Math.round(kb / 1024 * 10) / 10) + " MB" : Math.round(kb) + " KB"
    }

    function cacheRows() {
        const rows = []

        for (let i = 0; i < window.infoSource.count("caches", window.infoSource.revision); ++i) {
            const level = info("caches[" + i + "].level")
            const type  = String(info("caches[" + i + "].type"))

            rows.push({ label: "L" + level + (type === "Unified" ? "" : " " + type),
                        value: cacheSize(info("caches[" + i + "].size")),
                        hint: "Level " + level + ", " + type + ": " + info("caches[" + i + "].size") + " (cache of CPU 0)" })
        }

        return rows
    }

    readonly property int possibleThreads: window.topologySource.value("possible_cpus.cpu_max", window.topologySource.revision) + 1

    // Live values
    Section {
        title: "Now"

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            StatTile {
                Layout.fillWidth: true
                label: "Average"
                value: window.averageAll
                unit: "MHz"
                detail: window.threads.length + " threads"
                fraction: window.range.max > 0 ? window.averageAll / window.range.max : NaN
                tooltip: "Average frequency of all active threads, of " + window.range.max + " MHz"
            }

            StatTile {
                Layout.fillWidth: true
                visible: window.hybrid
                label: "P-threads"
                value: window.averageP
                unit: "MHz"
                detail: window.performance.length + " threads"
                fraction: window.averageP / Math.max(window.range.max, 1)
                tooltip: "Average frequency of the performance threads"
            }

            StatTile {
                Layout.fillWidth: true
                visible: window.hybrid
                label: "E-threads"
                value: window.averageE
                unit: "MHz"
                detail: window.efficient.length + " threads"
                fraction: window.averageE / Math.max(window.range.max, 1)
                tooltip: "Average frequency of the efficient threads"
            }

            StatTile {
                Layout.fillWidth: true
                label: "Fastest"
                value: window.fastest >= 0 ? window.current[window.fastest] : 0
                unit: "MHz"
                detail: window.fastest >= 0 ? window.kindName(window.fastest) + " " + window.fastest : ""
                fraction: window.fastest >= 0 ? window.current[window.fastest] / Math.max(window.range.max, 1) : NaN
                tooltip: "Thread with the highest frequency now"
            }

            StatTile {
                Layout.fillWidth: true
                label: "Slowest"
                value: window.slowest >= 0 ? window.current[window.slowest] : 0
                unit: "MHz"
                detail: window.slowest >= 0 ? window.kindName(window.slowest) + " " + window.slowest : ""
                fraction: window.slowest >= 0 ? window.current[window.slowest] / Math.max(window.range.max, 1) : NaN
                tooltip: "Thread with the lowest frequency now"
            }

            StatTile {
                Layout.fillWidth: true
                label: "Power"
                value: Math.round(window.cpuPower)
                unit: "W"
                detail: "package (RAPL)"
                fraction: window.cpuPower / window.cpuPowerMax
                tooltip: "Power of the CPU package from the RAPL energy counter, chart range 0 – " + window.cpuPowerMax + " W"
            }

            StatTile {
                Layout.fillWidth: true
                visible: !isNaN(window.temperature)
                label: "Temperature"
                value: Math.round(window.temperature)
                unit: "°C"
                detail: "Legion sensor"
                fraction: (window.temperature - 30) / 80
                level: window.temperature >= 90 ? "critical" : window.temperature >= 75 ? "warning" : "normal"
                tooltip: "CPU temperature of the Legion sensors (warning from 75 °C, critical from 90 °C)"
            }
        }
    }

    // Charts of the threads of one kind, column by column in the order of the CPU numbers
    component ThreadCharts: Section {
        id: group

        property var    cpus: []
        property alias  count: charts.count

        function itemAt(index) { return charts.itemAt(index) }

        visible: cpus.length > 0

        GridLayout {
            id: grid

            readonly property int chartColumns: Math.max(1, Math.min(group.cpus.length, Math.floor((width + columnSpacing) / (window.chartWidth + columnSpacing))))

            Layout.fillWidth: true
            columns: chartColumns
            columnSpacing: 6
            rowSpacing: 6

            Repeater {
                id: charts

                model: group.cpus

                delegate: HistoryChart {
                    required property int modelData

                    readonly property string prefix: "cpux_freq[" + modelData + "]."

                    property int scalingMin: 0
                    property int scalingMax: 0
                    property int base: 0

                    function sample() {
                        scalingMin = window.hw(modelData, "cpu_scaling_min_freq")
                        scalingMax = window.hw(modelData, "cpu_scaling_max_freq")
                        base = window.hw(modelData, "cpu_base_freq")
                        push(window.current[modelData] ?? 0)
                    }

                    Layout.fillWidth: true
                    Layout.preferredWidth: window.chartWidth
                    implicitHeight: 104
                    title: "CPU " + modelData + " [%1 MHz]"
                    fieldWidth: 4
                    minimum: window.hw(modelData, "cpu_info_min_freq")
                    maximum: window.hw(modelData, "cpu_info_max_freq")
                    tooltip: window.kindName(modelData) + " " + modelData + "\n"
                           + "Current frequency: " + Math.round(value) + " MHz\n"
                           + "Limits: " + scalingMin + " – " + scalingMax + " MHz\n"
                           + "Base frequency: " + base + " MHz\n"
                           + "Hardware range: " + minimum + " – " + maximum + " MHz"
                }
            }
        }
    }

    // Charts of the threads and next to them the processor and cache information
    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 12

            ThreadCharts {
                id: performanceCharts

                title: (window.hybrid ? "Performance threads" : "Threads") + " · " + cpus.length
                cpus: window.hybrid ? window.performance.slice().sort((a, b) => a - b) : window.threads
            }

            ThreadCharts {
                id: efficientCharts

                title: "Efficient threads · " + cpus.length
                cpus: window.hybrid ? window.efficient.slice().sort((a, b) => a - b) : []
            }
        }

        ColumnLayout {
            Layout.preferredWidth: window.infoWidth
            Layout.maximumWidth: window.infoWidth
            Layout.alignment: Qt.AlignTop
            spacing: 12

            InfoSection {
                Layout.fillWidth: true

                title: "Processor"
                source: window.infoSource
                onCopyRequested: (label, value) => window.copy(value)

                rows: [
                    { label: "Model",        value: window.text("cpu_model") },
                    { label: "Generation",   value: window.text("cpu_generation") },
                    { label: "Architecture", value: window.text("architecture") },
                    { label: "Threads",      value: window.threads.length + " active"
                                                    + (window.hybrid ? ": " + window.performance.length + " P + " + window.efficient.length + " E" : "")
                                                    + (window.possibleThreads > window.threads.length ? " (" + window.possibleThreads + " in total)" : ""),
                      hint: "Active threads of the CPU topology, offline threads are not shown" },
                    { label: window.hybrid ? "P-threads" : "Frequency", value: window.rangeText(window.performance.length > 0 ? window.performance : window.threads),
                      hint: "Hardware range of the threads (cpuinfo_min_freq – cpuinfo_max_freq) and the base frequency" },
                    { label: "E-threads",    value: window.hybrid ? window.rangeText(window.efficient) : "" },
                    { label: "Limits",       value: window.range.scaleMin + " – " + window.range.scaleMax + " MHz",
                      hint: "Frequency limits set for the threads (scaling_min_freq – scaling_max_freq), CPU Frequency page" },
                    { label: "Package",      value: window.text("physical_package_id") }
                ]
            }

            InfoSection {
                Layout.fillWidth: true

                title: "Cache"
                source: window.infoSource
                emptyText: "No cache information"
                onCopyRequested: (label, value) => window.copy(value)

                rows: window.cacheRows()
            }
        }
    }
}
