// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (Lenovo driver): fan full speed (Maximum / Custom) and fan curve, the same data as FanControl
// of the widget pages (UiDesign.md)
import QtQuick
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    title: "Fan Curve"

    // The fan curve uses the whole page the same as FanControl in its tab
    fillHeight: true

    // Power profile: defaults of the fan curve of the thermal mode, changed in PowerProfilesPanel next to the tabs
    DataSource {
        id: power

        dataType: "POWER_PROFILE"
    }

    DataSource {
        id: fan

        dataType: "FAN_OPTION"
        autoApply: true
    }

    FanCurveEditor {
        Layout.fillHeight: true

        profileSource: power
        fanSource: fan
    }
}
