// SPDX-License-Identifier: GPL-2.0
// One field: label, unit, control by the widget of the field options, hint
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

Item {
    id: field

    property DataSource source
    property string     path
    property string     label: ""       // overrides the label of the field options
    property bool       labelVisible: true
    property bool       vertical: false  // label above the control, radio buttons below each other (narrow panels)
    property bool       labelAfter: false // label right of the control (switch with the text like a check box)
    property int        radioColumns: 0   // columns of the radio buttons, 0 = all in one row (1: below each other)

    // Shown value instead of the value of the field (e.g. default of the power profile for a disabled limit), undefined = field value
    property var        overrideValue: undefined

    readonly property FieldModel fieldModel: source !== null && path.length > 0 ? source.field(path) : null
    readonly property var        shownValue: overrideValue !== undefined ? overrideValue : (fieldModel !== null ? fieldModel.value : undefined)

    visible: fieldModel !== null && fieldModel.visible
    implicitHeight: column.implicitHeight
    Layout.fillWidth: true

    ColumnLayout {
        id: column

        width: parent.width

        GridLayout {
            Layout.fillWidth: true
            // Without a stretched child (hidden label) the layout keeps its width, centered in the vertical mode
            Layout.alignment: field.vertical ? Qt.AlignHCenter : Qt.AlignLeft
            columns: field.vertical ? 1 : 3

            // Label before the control, or after it (labelAfter: check box like, "Custom - Toggle Fn+Q")
            FieldLabel {
                owner: field
                visible: field.labelVisible && !field.labelAfter
            }

            Loader {
                id: control

                // Tool tip of the field over the control too (the widget pages have it on the label and the control)
                HoverHandler {
                    id: controlHover
                }

                ThemedToolTip {
                    visible: controlHover.hovered && text.length > 0 && field.fieldModel !== null && field.fieldModel.widget !== "radio"
                    delay: 500
                    text: field.fieldModel !== null ? field.fieldModel.tooltip : ""
                }

                // Only the slider uses the whole width, other controls keep their size
                Layout.fillWidth: field.fieldModel !== null && (field.fieldModel.widget === "slider" || (field.fieldModel.widget === "radio" && !field.vertical))
                Layout.alignment: field.vertical ? Qt.AlignHCenter : Qt.AlignLeft | Qt.AlignVCenter

                sourceComponent: {
                    if (field.fieldModel === null)
                        return null

                    switch (field.fieldModel.widget) {
                    case "switch": return switchControl
                    case "slider": return sliderControl
                    case "spin":   return spinControl
                    case "combo":  return comboControl
                    case "radio":  return radioControl
                    default:       return labelControl
                    }
                }
            }

            FieldLabel {
                owner: field
                visible: field.labelVisible && field.labelAfter
            }

            Item {
                visible: !field.vertical && !field.labelAfter
                Layout.fillWidth: true
            }
        }

        Label {
            Layout.fillWidth: true

            visible: field.fieldModel !== null && field.fieldModel.hint.length > 0
            text: field.fieldModel !== null ? field.fieldModel.hint : ""
            font.italic: true
            opacity: 0.7
            wrapMode: Text.Wrap
        }
    }

    Component {
        id: switchControl

        ThemedSwitch {
            checked: field.shownValue
            enabled: !field.fieldModel.readonly
            onToggled: field.fieldModel.value = checked
        }
    }

    Component {
        id: sliderControl

        RowLayout {
            ThemedSlider {
                Layout.fillWidth: true

                from: field.fieldModel.min !== undefined ? field.fieldModel.min : 0
                to: field.fieldModel.max !== undefined ? field.fieldModel.max : 100
                stepSize: field.fieldModel.step
                snapMode: Slider.SnapAlways
                value: field.shownValue
                enabled: !field.fieldModel.readonly
                onMoved: field.fieldModel.value = Math.round(value / field.fieldModel.step) * field.fieldModel.step
            }

            Label {
                Layout.preferredWidth: 50

                text: field.shownValue
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    Component {
        id: spinControl

        ThemedSpinBox {
            from: field.fieldModel.min !== undefined ? field.fieldModel.min : 0
            to: field.fieldModel.max !== undefined ? field.fieldModel.max : 1000000
            value: field.shownValue
            stepSize: field.fieldModel.step
            editable: true
            enabled: !field.fieldModel.readonly
            onValueModified: field.fieldModel.value = value
        }
    }

    Component {
        id: comboControl

        ThemedComboBox {
            implicitWidth: 250

            model: field.fieldModel.choices
            textRole: "text"
            valueRole: "value"
            // count: evaluated again when the items are created
            currentIndex: count > 0 ? indexOfValue(field.shownValue) : -1
            enabled: !field.fieldModel.readonly
            onActivated: field.fieldModel.value = currentValue
        }
    }

    Component {
        id: radioControl

        // One radio button per choice (allowed values from the data)
        Grid {
            columns: field.vertical ? 1 : field.radioColumns > 0 ? field.radioColumns : Math.max(field.fieldModel.choices.length, 1)
            spacing: field.vertical || field.radioColumns === 1 ? 6 : 16

            Repeater {
                model: field.fieldModel.choices

                delegate: ThemedRadioButton {
                    text: modelData.text
                    textColor: modelData.color !== undefined ? modelData.color : theme.text
                    checked: field.shownValue === modelData.value
                    enabled: !field.fieldModel.readonly
                    onClicked: field.fieldModel.value = modelData.value

                    // Tool tip of the value ((legion.ui.value_tooltip), radio buttons of OtherControl)
                    ThemedToolTip {
                        visible: parent.hovered && text.length > 0
                        delay: 500
                        text: modelData.tooltip !== undefined ? modelData.tooltip : ""
                    }
                }
            }
        }
    }

    Component {
        id: labelControl

        Label {
            text: field.shownValue !== undefined ? String(field.shownValue) : ""
        }
    }

    component FieldLabel: Label {
        // Inline component: no access to the ids of the file
        required property Item owner

        // Long labels (power limits) are wrapped, the controls stay aligned
        Layout.preferredWidth: owner.vertical || owner.labelAfter ? -1 : 300
        Layout.fillWidth: owner.vertical || owner.labelAfter
        wrapMode: Text.Wrap
        horizontalAlignment: owner.vertical ? Text.AlignHCenter : Text.AlignLeft

        // Changed value not applied yet (VALUE_CHANGE_COLOR of the widget pages), disabled field (limits out of the custom mode)
        color: !owner.enabled ? theme.disabledText
             : owner.fieldModel !== null && owner.fieldModel.modified ? theme.modified : theme.text

        text: owner.fieldModel === null ? ""
            : (owner.label.length > 0 ? owner.label : owner.fieldModel.label)
              + (owner.fieldModel.unit.length > 0 ? " [" + owner.fieldModel.unit + "]" : "")

        HoverHandler {
            id: hover
        }

        ThemedToolTip {
            visible: hover.hovered && text.length > 0
            delay: 500
            text: owner.fieldModel !== null ? owner.fieldModel.tooltip : ""
        }
    }
}
