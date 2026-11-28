// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (standard interfaces, any notebook): power saving of the platform, e.g. the PCIe ASPM policy of
// the kernel; not part of the Lenovo pages (the Device page depends on the Lenovo Legion kernel driver)
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "System"

    columnWidth: 540

    // PCIe ASPM policy of the kernel, hidden when the kernel does not offer it
    AutoSection {
        title: "PCIe Power Saving"
        dataType: "PCIE_ASPM"
        applyBar: true
        hideWhenAbsent: true
    }
}
