// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (standard interfaces): NVIDIA clock offsets, the same data as OffsetsControl of the widget pages (UiDesign.md)
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "GPU (QML preview)"

    // Presets the same as OffsetsControl::GPU_CLOCK_PRESETS
    Section {
        title: "GPU Clock"
        presetLabel: "Preset offsets configuration"
        applyBar: true
        source: DataSource {
            id: clock

            dataType: "NVIDIA_NWML"
        }

        Field { source: clock; path: "gpu_offset.value" }
        Field { source: clock; path: "memory_offset.value" }

        presets: [
            { text: "None",         apply: false, values: {} },
            { text: "Conservative", apply: false, values: { "gpu_offset.value": 100, "memory_offset.value": 200 } },
            { text: "Moderate",     apply: false, values: { "gpu_offset.value": 150, "memory_offset.value": 300 } },
            { text: "Aggressive",   apply: false, values: { "gpu_offset.value": 200, "memory_offset.value": 500 } },
            { text: "Reset",        apply: false, values: { "gpu_offset.value": 0,   "memory_offset.value": 0 } }
        ]
    }
}
