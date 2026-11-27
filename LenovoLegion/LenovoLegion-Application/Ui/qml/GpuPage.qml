// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (standard interfaces): NVIDIA clock offsets, the same data as OffsetsControl of the widget pages (UiDesign.md),
// and the runtime power management of the discrete GPU
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "GPU"

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

    // Runtime power management of the discrete GPU: the state read every 2 s, a warning when it is disabled
    Section {
        id: powerManagement

        title: "Power Management"
        hideWhenAbsent: true
        source: DataSource {
            id: runtimePM

            dataType: "GPU_RUNTIME_PM"
            autoApply: true
            interval: 2000
        }

        readonly property bool disabled: runtimePM.available && runtimePM.value("auto_suspend", runtimePM.revision) === false

        Field { source: runtimePM; path: "auto_suspend" }
        Field { source: runtimePM; path: "runtime_status" }
        Field { source: runtimePM; path: "device" }

        Label {
            Layout.fillWidth: true
            visible: powerManagement.disabled
            wrapMode: Text.Wrap
            color: theme.modified
            text: "⚠ The discrete GPU is never powered down: it uses power also when no application needs it. "
                + "The NVIDIA driver normally enables the power management itself; enable it above unless you debug the GPU."
        }
    }
}
