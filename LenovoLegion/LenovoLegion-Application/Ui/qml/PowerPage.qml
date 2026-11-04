// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (Lenovo driver): CPU and GPU power limits, the same data as PowerControl of the widget pages
// (UiDesign.md), power profiles are in PowerProfilesPanel, the fan curve in FanCurvePage
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "Power (QML preview)"

    // Power profile (custom mode, presets), changed in PowerProfilesPanel next to the tabs
    DataSource {
        id: power

        dataType: "POWER_PROFILE"
    }

    /*
     * Presets with the default values of the supported power profiles (the same as the preset combo box of PowerControl)
     */
    function profilePresets(source,limits) {
        const profile = power.field("current_value")
        const presets = [{ text: "NONE", apply: false, values: {} }]

        if (source === null || profile === null)
            return presets

        for (const choice of profile.choices) {
            if (choice.value === 255)
                continue

            const values = {}

            for (const limit of limits)
                values[limit + ".current_value"] = source.value(limit + ".mode_descriptor_map[" + choice.value + "].default_value")

            presets.push({ text: choice.text, apply: false, values: values })
        }

        return presets
    }

    readonly property bool customMode: power.revision >= 0 && power.value("thermal_mode") === 255

    // Limits can be changed only in the custom thermal mode, order the same as PowerControl
    Section {
        id: cpuSection

        title: "CPU Power Control"
        applyBar: true
        enabled: customMode
        source: DataSource {
            id: cpuPower

            dataType: "CPU_POWER"
        }

        readonly property var limits: ["cpu_tmp_limit", "cpu_clp_limit", "cpu_stp_limit", "cpu_ltp_limit", "gpu_total_onac", "cpu_pl1_tau", "gpu_to_cpu_dynamic_boost"]

        Repeater {
            model: cpuSection.limits

            delegate: LimitField { source: cpuPower; profileSource: power; limitPath: modelData }
        }

        presets: (cpuPower.revision, power.revision, profilePresets(cpuPower,cpuSection.limits))
    }

    Section {
        id: gpuSection

        title: "GPU Power Control"
        applyBar: true
        enabled: customMode
        source: DataSource {
            id: gpuPower

            dataType: "GPU_POWER"
        }

        readonly property var limits: ["gpu_temperature_limit", "gpu_power_boost", "gpu_configurable_tgp"]

        Repeater {
            model: gpuSection.limits

            delegate: LimitField { source: gpuPower; profileSource: power; limitPath: modelData }
        }

        presets: (gpuPower.revision, power.revision, profilePresets(gpuPower,gpuSection.limits))
    }
}
