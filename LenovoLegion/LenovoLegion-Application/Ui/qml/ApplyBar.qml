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
    property bool       showApply: true         // Cancel/Apply (hidden on a page with the change bar, SettingsPage)

    readonly property var applyPresets: presets.filter(preset => preset.apply !== false)
    readonly property var fillPresets:  presets.filter(preset => preset.apply === false)

    // Preset of the shown values (after Cancel, a failed Apply or a change from elsewhere the combo box does not keep
    // a preset which is not shown), the preset without values (None) otherwise. Not supported fields are not compared
    // (they are not filled in either).
    readonly property int shownPreset: {
        const revision = source !== null ? source.revision : 0
        let fallback = -1

        for (let i = 0; i < fillPresets.length; ++i) {
            const paths = Object.keys(fillPresets[i].values)

            if (paths.length === 0) {
                if (fallback < 0)
                    fallback = i
                continue
            }

            if (source === null)
                continue

            let compared = 0
            let matches = true

            for (const path of paths) {
                const model = source.field(path, revision)

                if (model === null || !model.visible)
                    continue

                const shown = model.value
                const wanted = fillPresets[i].values[path]

                ++compared

                // Displayed values of scaled fields are rounded (-49804 uV shown as -50 mV)
                if (typeof shown === "number" && typeof wanted === "number" ? Math.abs(shown - wanted) >= 0.5 : shown !== wanted) {
                    matches = false
                    break
                }
            }

            if (matches && compared > 0)
                return i
        }

        return fallback
    }

    Label {
        visible: bar.fillPresets.length > 0
        text: bar.presetLabel
    }

    ThemedComboBox {
        visible: bar.fillPresets.length > 0
        implicitWidth: 180

        model: bar.fillPresets.map(preset => preset.text)
        enabled: bar.source !== null && bar.source.valid
        currentIndex: bar.shownPreset
        onActivated: index => {
            bar.source.setValues(bar.fillPresets[index].values)
            // The selection of the user replaced the binding
            currentIndex = Qt.binding(() => bar.shownPreset)
        }
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
        visible: bar.showApply
        text: "Cancel"
        enabled: bar.source !== null && bar.source.dirty
        onClicked: bar.source.cancel()
    }

    ThemedButton {
        visible: bar.showApply
        text: "Apply"
        enabled: bar.source !== null && bar.source.dirty
        onClicked: bar.source.apply()
    }
}
