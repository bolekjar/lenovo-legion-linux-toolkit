// SPDX-License-Identifier: GPL-2.0
// Power profiles always visible next to the tabs (stretched to the bottom, controls in the middle), the same as PowerProfileControl in verticalLayout_PowerProfiles
// of ToolBarHomeWidget: profiles below each other, Custom - Toggle Fn+Q, written immediately
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

Section {
    title: "Power Profiles"
    titleIcon: "qrc:/images/icons/LenovoLegionIco.png"   // controlled by the Lenovo Legion kernel driver

    // Outside of a SettingsPage: font of the theme
    font.pixelSize: theme.fontPixelSize

    source: DataSource {
        id: power

        dataType: "POWER_PROFILE"
        autoApply: true
    }

    // Without the Lenovo driver (no data) grayed with the reason
    enabled: power.available

    Label {
        Layout.fillWidth: true

        visible: !power.available
        text: "Requires the Lenovo Legion kernel driver"
        color: theme.modified
        font.bold: true
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
    }

    // Controls in the middle of the stretched panel
    Item { Layout.fillHeight: true }

    Field { source: power; path: "current_value"; vertical: true; labelVisible: false }

    Item { Layout.fillHeight: true }

    // At the bottom, the label right of the switch (checkBox_customFnQ of PowerProfileControl)
    Field { source: power; path: "custom_fnq_enabled.current_value"; labelAfter: true }
}
