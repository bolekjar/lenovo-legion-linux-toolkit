// SPDX-License-Identifier: GPL-2.0
// Detail window of the HW monitor: CPU information and frequency of every active thread,
// the same as CPUFrequency/ThreadFrequency of the widget pages (8 rows, column by column, P- and E-threads in order of the CPU numbers)
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
    required property var        range         // frequency range of all threads (scaleMin, scaleMax)

    readonly property var threads: performance.concat(efficient).sort((a, b) => a - b)

    title: "Lenovo Legion Linux Toolkit - Threads Frequency"
    width: 1280
    height: 1200
    minimumWidth: 1280
    minimumHeight: 600

    // New sample of every thread, only while the window is shown (HwMonitorPage)
    function sample() {
        for (let i = 0; i < charts.count; ++i)
            charts.itemAt(i).sample()
    }

    function cacheText() {
        const lines = []

        for (let i = 0; i < window.infoSource.count("caches"); ++i)
            lines.push("CPU Cache L" + window.infoSource.value("caches[" + i + "].level") + "," + window.infoSource.value("caches[" + i + "].type") + ": " + window.infoSource.value("caches[" + i + "].size"))

        return lines.join("\n")
    }

    Section {
        title: (window.infoSource.revision, "CPU Details (" + String(window.infoSource.value("cpu_model")).trim() + ")")

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: (window.infoSource.revision, window.topologySource.revision,
                   "CPU Generation: " + String(window.infoSource.value("cpu_generation")).trim() + "\n"
                 + "CPU Architecture: " + String(window.infoSource.value("architecture")).trim() + "\n"
                 + "CPU Cores: " + (window.topologySource.count("active_cpus") > 0 ? window.topologySource.value("active_cpus[0].cpu_max") + 1 : -1) + "\n"
                 + "CPU Min Frequency: " + window.range.scaleMin + " MHz\n"
                 + "CPU Max Frequency: " + window.range.scaleMax + " MHz\n"
                 + window.cacheText() + "\n"
                 + "CPU Physical Package Id: " + window.infoSource.value("physical_package_id"))
        }
    }

    Section {
        title: "Frequency per thread"

        GridLayout {
            Layout.fillWidth: true
            flow: GridLayout.TopToBottom
            rows: 8
            columnSpacing: 6
            rowSpacing: 6

            Repeater {
                id: charts

                model: window.threads

                delegate: HistoryChart {
                    required property int modelData

                    readonly property bool   isPerformance: window.performance.indexOf(modelData) >= 0
                    readonly property string prefix: "cpux_freq[" + modelData + "]."

                    property int scalingMin: 0
                    property int scalingMax: 0
                    property int base: 0

                    function sample() {
                        scalingMin = window.hwSource.value(prefix + "cpu_scaling_min_freq") / 1000
                        scalingMax = window.hwSource.value(prefix + "cpu_scaling_max_freq") / 1000
                        base = window.hwSource.value(prefix + "cpu_base_freq") / 1000
                        push(Math.floor(window.hwSource.value(prefix + "cpu_scaling_cur_freq") / 1000))
                    }

                    Layout.fillWidth: true
                    implicitHeight: 104
                    title: (isPerformance ? "P-Core: " : "E-Core: ") + modelData + " [%1 MHz]"
                    fieldWidth: 4
                    minimum: (window.hwSource.revision, window.hwSource.value(prefix + "cpu_info_min_freq") / 1000)
                    maximum: (window.hwSource.revision, window.hwSource.value(prefix + "cpu_info_max_freq") / 1000)
                    tooltip: "Current CPU Frequency: " + Math.round(value) + " MHz\n"
                           + "Scaling Min Frequency: " + scalingMin + " MHz\n"
                           + "Scaling Max Frequency: " + scalingMax + " MHz\n"
                           + "Base Frequency: " + base + " MHz\n"
                           + "Min Frequency: " + minimum + " MHz\n"
                           + "Max Frequency: " + maximum + " MHz"
                }
            }
        }
    }
}
