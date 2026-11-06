// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (standard interfaces): CPU threads and SMT, the same data as CPUControl of the widget pages (UiDesign.md)
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "CPU Control"

    DataSource {
        id: topology

        dataType: "CPU_TOPOLOGY"
    }

    DataSource {
        id: smt

        dataType: "CPU_SMT"
    }

    CpuGrid {
        topology: topology
        smt: smt
    }
}
