// SPDX-License-Identifier: GPL-2.0
// Detail window of the HW monitor: NVIDIA GPU charts and summary, the same as GPUDetails of the widget pages
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

DetailWindow {
    id: window

    required property DataSource nvmlSource

    title: "GPU Details"
    width: 1200
    height: 768
    minimumWidth: 1200
    minimumHeight: 600

    function hm(path) {
        return window.nvmlSource.value("hardware_monitor." + path)
    }

    // Theoretical maximum of the PCIe link in MB/s (GB/s per lane by the generation)
    readonly property real pcieMax: (window.nvmlSource.revision, Math.floor(([0, 0.25, 0.5, 0.984, 1.969, 3.938, 7.877][hm("pcie.generation_max")] ?? 0) * hm("pcie.width_max") * 1024))
    readonly property int  chartHeight: 180
    readonly property real memoryTotal: (window.nvmlSource.revision, hm("memory_use.total") / (1024 * 1024))

    function sample() {
        coreClock.push(hm("gpu_clock.value"))
        memoryClock.push(hm("memory_clock.value"))
        smClock.push(hm("sm_clock.value"))
        gpuUtilization.push(hm("gpu_utilization.value"))
        memoryUtilization.push(hm("memory_utilization.value"))
        memoryUsed.push(hm("memory_use.used") / (1024 * 1024))
        temperature.push(hm("temperature.value"))
        power.push(Math.floor(hm("power.value") / 1000))
        pcieRx.push(Math.floor(hm("pcie.rx_bytes") / 1024))
        pcieTx.push(Math.floor(hm("pcie.tx_bytes") / 1024))
    }

    function minMaxText(name, value, unit, path) {
        return name + ": " + Math.round(value) + " " + unit + "\n" + name + " min: " + hm(path + ".min_value") + " " + unit + "\n" + name + " max: " + hm(path + ".max_value") + " " + unit
    }

    Section {
        title: (window.nvmlSource.revision, "GPU Details - " + window.nvmlSource.value("name") + (window.nvmlSource.value("gpu_suspended") ? " (suspended)" : ""))

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Section {
                title: "Clock"
                Layout.fillWidth: true

                RowLayout {
                    Layout.fillWidth: true

                    HistoryChart {
                        id: coreClock
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Core Clock [%1 MHz]"
                        maximum: (window.nvmlSource.revision, window.hm("gpu_clock.max_value"))
                        tooltip: (window.nvmlSource.revision, window.minMaxText("Core Clock", value, "MHz", "gpu_clock"))
                    }

                    HistoryChart {
                        id: memoryClock
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Memory Clock [%1 MHz]"
                        maximum: (window.nvmlSource.revision, window.hm("memory_clock.max_value"))
                        tooltip: (window.nvmlSource.revision, window.minMaxText("Memory Clock", value, "MHz", "memory_clock"))
                    }

                    HistoryChart {
                        id: smClock
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "SM Clock [%1 MHz]"
                        maximum: (window.nvmlSource.revision, window.hm("sm_clock.max_value"))
                        tooltip: (window.nvmlSource.revision, window.minMaxText("SM Clock", value, "MHz", "sm_clock"))
                    }
                }
            }

            Section {
                title: "Utilization"
                Layout.fillWidth: true

                RowLayout {
                    Layout.fillWidth: true

                    HistoryChart {
                        id: gpuUtilization
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "GPU [%1 %]"
                        maximum: (window.nvmlSource.revision, window.hm("gpu_utilization.max_value"))
                        tooltip: (window.nvmlSource.revision, window.minMaxText("GPU Utilization", value, "%", "gpu_utilization"))
                    }

                    HistoryChart {
                        id: memoryUtilization
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Memory [%1 %]"
                        maximum: (window.nvmlSource.revision, window.hm("memory_utilization.max_value"))
                        tooltip: (window.nvmlSource.revision, window.minMaxText("Memory Utilization", value, "%", "memory_utilization"))
                    }

                    HistoryChart {
                        id: memoryUsed
                        Layout.preferredHeight: window.chartHeight
                        Layout.fillWidth: true
                        title: "Memory Used [%1 MB]"
                        maximum: window.memoryTotal
                        tooltip: "Memory Used: " + Math.round(value) + " MB\nMemory Used min: 0 MB\nMemory Used max: " + Math.round(window.memoryTotal) + " MB"
                    }
                }
            }
        }

        Section {
            title: "Other"

            RowLayout {
                Layout.fillWidth: true

                HistoryChart {
                    id: temperature
                    Layout.preferredHeight: window.chartHeight
                    Layout.fillWidth: true
                    title: "Temperature [%1 °C]"
                    // Slowdown threshold of NVML: critical, 10 °C under it warning
                    warning: (window.nvmlSource.revision, window.hm("temperature.slowdown")) > 0 ? window.hm("temperature.slowdown") - 10 : NaN
                    critical: (window.nvmlSource.revision, window.hm("temperature.slowdown")) > 0 ? window.hm("temperature.slowdown") : NaN
                    maximum: (window.nvmlSource.revision, window.hm("temperature.shutdown"))
                    tooltip: "Temperature: " + Math.round(value) + " °C\nTemperature min: 0 °C\nTemperature max: " + maximum + " °C"
                }

                HistoryChart {
                    id: power
                    Layout.preferredHeight: window.chartHeight
                    Layout.fillWidth: true
                    title: "Power [%1 W]"
                    maximum: (window.nvmlSource.revision, Math.floor(window.hm("power.max_value") / 1000))
                    tooltip: (window.nvmlSource.revision, "Power: " + Math.round(value) + " W\nPower min: " + Math.floor(window.hm("power.min_value") / 1000) + " W\nPower max: " + maximum + " W")
                }

                HistoryChart {
                    id: pcieRx
                    Layout.preferredHeight: window.chartHeight
                    Layout.fillWidth: true
                    title: "PCIe RX Throughput [%1 MB/s]"
                    maximum: window.pcieMax
                    tooltip: "PCIe RX Throughput: " + Math.round(value) + " MB/s\nPCIe RX Throughput min: 0 MB/s\nPCIe RX Throughput max: " + window.pcieMax + " MB/s"
                }

                HistoryChart {
                    id: pcieTx
                    Layout.preferredHeight: window.chartHeight
                    Layout.fillWidth: true
                    title: "PCIe TX Throughput [%1 MB/s]"
                    maximum: window.pcieMax
                    tooltip: "PCIe TX Throughput: " + Math.round(value) + " MB/s\nPCIe TX Throughput min: 0 MB/s\nPCIe TX Throughput max: " + window.pcieMax + " MB/s"
                }
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: (window.nvmlSource.revision,
                   "GPU Max Clock: " + window.hm("gpu_clock.max_value") + " MHz\n"
                 + "GPU Min Clock: " + window.hm("gpu_clock.min_value") + " MHz\n"
                 + "GPU Max SM Clock: " + window.hm("sm_clock.max_value") + " MHz\n"
                 + "GPU Min SM Clock: " + window.hm("sm_clock.min_value") + " MHz\n"
                 + "GPU Max Memory Clock: " + window.hm("memory_clock.max_value") + " MHz\n"
                 + "GPU Min Memory Clock: " + window.hm("memory_clock.min_value") + " MHz\n"
                 + "TDP Max: " + Math.floor(window.hm("power.max_value") / 1000) + " W\n"
                 + "TDP Min: " + Math.floor(window.hm("power.min_value") / 1000) + " W\n"
                 + "VRAM Size: " + (window.hm("memory_use.total") / (1024 * 1024 * 1024)).toFixed(2) + " GB\n"
                 + "PCIe Max Lanes: " + window.hm("pcie.width_max") + "\n"
                 + "PCIe Max generation: " + window.hm("pcie.generation_max") + "\n"
                 + "PCIe Theoretical Max Throughput: " + window.pcieMax + " MB/s\n"
                 + "GPU Temperature Shutdown Threshold: " + window.hm("temperature.shutdown") + " °C\n"
                 + "GPU Temperature Slowdown Threshold: " + window.hm("temperature.slowdown") + " °C")
        }
    }
}
