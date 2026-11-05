// SPDX-License-Identifier: GPL-2.0
// Apply/Cancel of the changes of a data source
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

RowLayout {
    id: bar

    property DataSource source

    // [{text, values: {path: displayed value}, apply: true|false}]
    //  apply (default): button, values are sent immediately (Default of the widget pages)
    //  apply: false:    item of the "Preset" combo box, values are filled in and sent by Apply (offsets of the widget pages)
    property var        presets: []
    property string     presetLabel: "Preset"   // label of the preset combo box (texts of the widget pages)

    readonly property var applyPresets: presets.filter(preset => preset.apply !== false)
    readonly property var fillPresets:  presets.filter(preset => preset.apply === false)

    Label {
        visible: bar.fillPresets.length > 0
        text: bar.presetLabel
    }

    ThemedComboBox {
        visible: bar.fillPresets.length > 0
        implicitWidth: 180

        model: bar.fillPresets.map(preset => preset.text)
        enabled: bar.source !== null && bar.source.valid
        onActivated: index => bar.source.setValues(bar.fillPresets[index].values)
    }

    Item {
        Layout.fillWidth: true
    }

    Repeater {
        model: bar.applyPresets

        delegate: ThemedButton {
            text: modelData.text
            enabled: bar.source !== null && bar.source.valid
            onClicked: bar.source.applyValues(modelData.values)
        }
    }

    ThemedButton {
        text: "Cancel"
        enabled: bar.source !== null && bar.source.dirty
        onClicked: bar.source.cancel()
    }

    ThemedButton {
        text: "Apply"
        enabled: bar.source !== null && bar.source.dirty
        onClicked: bar.source.apply()
    }
}
