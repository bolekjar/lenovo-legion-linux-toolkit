// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (standard interfaces): Intel P-State, AMD P-State, uncore frequency, TCC offset and CPU voltage offsets, the same data as OtherControl and OffsetsControl
// of the widget pages (UiDesign.md)
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "CPU Power"

    // Groups next to each other on a wide page
    columnWidth: 540

    // All fields of the message by the annotations in CPUOptions.proto
    AutoSection {
        title: "CPU Power State Control"
        dataType: "CPU_PSTATE"
        applyBar: true
        hideWhenAbsent: true

        // The same values as OtherControl::CPU_PSTATE_PRESET
        presets: [
            {
                text: "Default",
                values: {
                    "hwp_dynamic_boost": false,
                    "no_turbo": false,
                    "max_perf_pct": 100,
                    "min_perf_pct": 14,
                    "status": "active"
                }
            }
        ]
    }

    // AMD P-State (amd-pstate): mode, boost, energy performance preference, preferred core (CPUOptions.proto)
    AutoSection {
        title: "AMD P-State"
        dataType: "CPU_AMD_PSTATE"
        applyBar: true
        hideWhenAbsent: true

        presets: [
            {
                text: "Default",
                values: {
                    "status": "active",
                    "boost": true
                }
            }
        ]
    }

    // Order of the planes and presets (mV) the same as OffsetsControl::CPU_VOLTAGE_PRESETS
    // Intel uncore frequency (ring bus, L3 cache, memory controller), hidden without intel_uncore_frequency
    AutoSection {
        title: "Uncore Frequency"
        dataType: "CPU_UNCORE"
        applyBar: true
        hideWhenAbsent: true
    }

    // Intel TCC offset (throttling temperature), hidden without the processor thermal device
    AutoSection {
        title: "Temperature Limit (TCC)"
        dataType: "CPU_TCC"
        applyBar: true
        hideWhenAbsent: true
    }

    // Vendor of the CPU (modalias: 0000 Intel, 0002 AMD), the voltage offsets are Intel only
    DataSource {
        id: cpuInfo

        dataType: "CPU_INFO"
    }

    Section {
        title: "CPU Voltage"
        // Hidden on another CPU than Intel, on Intel the reason is shown (e.g. the kernel module is not loaded)
        readonly property var vendor: cpuInfo.value("vendor_code", cpuInfo.revision)

        visible: !vendor || vendor === "0000"
        presetLabel: "Preset undervolting configuration"
        applyBar: true
        source: DataSource {
            id: voltage

            dataType: "CPU_INTEL_MSR"
        }

        Field { source: voltage; path: "cpu.offset" }
        Field { source: voltage; path: "uncore.offset" }
        Field { source: voltage; path: "cache.offset" }
        Field { source: voltage; path: "gpu.offset" }
        Field { source: voltage; path: "analogio.offset" }

        presets: [
            { text: "None",         apply: false, values: {} },
            { text: "Conservative", apply: false, values: { "cpu.offset": -50,  "uncore.offset": -50,  "cache.offset": -50,  "gpu.offset": -40,  "analogio.offset": 0 } },
            { text: "Moderate",     apply: false, values: { "cpu.offset": -100, "uncore.offset": -80,  "cache.offset": -100, "gpu.offset": -75,  "analogio.offset": 0 } },
            { text: "Aggressive",   apply: false, values: { "cpu.offset": -150, "uncore.offset": -100, "cache.offset": -150, "gpu.offset": -100, "analogio.offset": 0 } },
            { text: "Reset",        apply: false, values: { "cpu.offset": 0,    "uncore.offset": 0,    "cache.offset": 0,    "gpu.offset": 0,    "analogio.offset": 0 } }
        ]
    }
}
