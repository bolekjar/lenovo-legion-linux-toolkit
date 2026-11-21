// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (standard interfaces): frequency limits of the CPU threads, the same data as CPUFrequencyControl of the widget pages (UiDesign.md)
import QtQuick
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "CPU Freq Control"

    // The thread cells scroll in the group (FrequencyGrid), the controls above them stay visible
    fillHeight: true

    DataSource {
        id: cpuTopology

        dataType: "CPU_TOPOLOGY"
    }

    FrequencyGrid {
        Layout.fillHeight: true

        topology: cpuTopology
    }
}
