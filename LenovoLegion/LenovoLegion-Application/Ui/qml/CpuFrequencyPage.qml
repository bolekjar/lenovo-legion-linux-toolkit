// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (standard interfaces): frequency limits of the CPU threads, the same data as CPUFrequencyControl of the widget pages (UiDesign.md)
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "CPU Freq Control"

    DataSource {
        id: topology

        dataType: "CPU_TOPOLOGY"
    }

    FrequencyGrid {
        topology: topology
    }
}
