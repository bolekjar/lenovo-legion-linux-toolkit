// SPDX-License-Identifier: GPL-2.0
// Power limit (CPUPower.Limit, GPUPower.Limit) the same as PowerControl of the widget pages:
// in the custom thermal mode the current value is edited, otherwise the default value of the power profile is shown.
// Range, steps (combo box) and support come from the descriptor of the custom mode (CpuPower.proto).
import QtQuick
import LenovoLegion.Ui 1.0

Field {
    id: limit

    property DataSource profileSource    // POWER_PROFILE
    property string     limitPath    // e.g. "cpu_ltp_limit"

    readonly property bool custom: profileSource !== null && profileSource.revision >= 0 && profileSource.value("thermal_mode", profileSource.revision) === 255

    path: limitPath + ".current_value"

    overrideValue: custom || profileSource === null || source === null ? undefined
                 : (source.value(limitPath + ".mode_descriptor_map[" + profileSource.value("current_value") + "].default_value", source.revision))
}
