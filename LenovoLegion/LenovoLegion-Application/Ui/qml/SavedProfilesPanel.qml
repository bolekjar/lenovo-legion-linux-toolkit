// SPDX-License-Identifier: GPL-2.0
// Quick switch of the profiles saved in the profile management (toolbar page "Profiles", ToolBarProfilesWidget):
// the saved profiles of the daemon (PROFILES), Load applies the selected one at once (without the confirmation
// of the management page) and all pages read the data again
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

Section {
    id: panel

    title: "Saved Profiles"

    // Outside of a SettingsPage: font of the theme
    font.pixelSize: theme.fontPixelSize

    source: DataSource {
        id: profiles

        dataType: "PROFILES"
    }

    readonly property var names: {
        const result = []

        for (let i = 0; i < (profiles.count("profiles", profiles.revision)); ++i)
            result.push(profiles.value("profiles[" + i + "].name", profiles.revision))

        return result
    }

    function description(index) {
        return index >= 0 ? (profiles.value("profiles[" + index + "].description", profiles.revision)) : ""
    }

    enabled: profiles.available

    Label {
        Layout.fillWidth: true

        visible: profiles.available && panel.names.length === 0
        text: "No saved profiles (Profiles page)"
        color: theme.disabledText
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
    }

    ThemedComboBox {
        id: selection

        Layout.fillWidth: true

        visible: panel.names.length > 0
        model: panel.names

        // The list can come after the combo box was created: the first profile is selected
        onCountChanged: if (currentIndex < 0 && count > 0) currentIndex = 0

        // The list can be changed in the profile management: read it again when the list is opened
        onPressedChanged: if (pressed) profiles.reload()

        ThemedToolTip {
            visible: selection.hovered && text.length > 0
            delay: 500
            text: panel.description(selection.currentIndex)
        }
    }

    ThemedButton {
        Layout.fillWidth: true

        visible: panel.names.length > 0
        text: "Load"
        enabled: selection.currentIndex >= 0
        onClicked: {
            const name = selection.currentText

            if (profiles.sendCommand({ "action": "LOAD", "name": name }))
                profiles.reloadAllSources()

            selection.currentIndex = Math.max(0, panel.names.indexOf(name))
        }
    }
}
