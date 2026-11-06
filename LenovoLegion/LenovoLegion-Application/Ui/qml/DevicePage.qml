// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (Lenovo driver): input devices and GPU mode, the same data as OtherControl of the widget pages (UiDesign.md)
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "Device"

    // Groups next to each other on a wide page
    columnWidth: 540

    // Selected fields, written immediately
    Section {
        title: "Hardware Control"
        source: DataSource {
            id: other

            dataType: "OTHER"
            autoApply: true
        }

        Field { source: other; path: "touch_pad.current" }
        Field { source: other; path: "win_key.current" }
    }

    // Radio buttons below each other with the tool tips of the modes, the same as OtherControl
    Section {
        title: "GPU Control"
        source: DataSource {
            id: gpuSwitch

            dataType: "OTHER_GPU_SWITCH"
            autoApply: true
        }

        Field { source: gpuSwitch; path: "current"; radioColumns: 1; labelVisible: false }
    }
}
