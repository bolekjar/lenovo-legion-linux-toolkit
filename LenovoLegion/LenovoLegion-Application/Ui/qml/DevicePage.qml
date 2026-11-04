// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (Lenovo driver): input devices and GPU mode, the same data as OtherControl of the widget pages (UiDesign.md)
import QtQuick
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "Device (QML preview)"

    // Selected fields, written immediately
    Section {
        title: "Input"
        source: DataSource {
            id: other

            dataType: "OTHER"
            autoApply: true
        }

        Field { source: other; path: "touch_pad.current" }
        Field { source: other; path: "win_key.current" }
    }

    AutoSection {
        title: "GPU Mode"
        dataType: "OTHER_GPU_SWITCH"
        autoApply: true
    }
}
