// SPDX-License-Identifier: GPL-2.0
// Power profiles always visible next to the tabs (stretched to the bottom), the data of PowerProfileControl in
// verticalLayout_PowerProfiles of ToolBarHomeWidget: the active profile, a card per profile with its color,
// Custom - Toggle Fn+Q at the bottom, written immediately
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

Section {
    id: panel

    title: "Power Profiles"
    titleIcon: "qrc:/images/icons/LenovoLegionIco.png"   // controlled by the Lenovo Legion kernel driver

    // Cards in the middle of the stretched panel
    stretchContent: true

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
        text: power.moduleError.length > 0 ? power.moduleError : "Requires the Lenovo Legion kernel driver"
        color: theme.modified
        font.bold: true
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
    }

    // Profile shown (thermal mode, value_from) and the allowed profiles with their colors ((legion.ui.value_color))
    readonly property FieldModel profile:  (power.field("current_value", power.revision))
    readonly property var        choices:  profile !== null ? profile.choices : []
    readonly property var        active:   {
        for (const choice of choices)
            if (profile.value === choice.value)
                return choice

        return null
    }

    function profileColor(choice) {
        return choice !== null && choice.color !== undefined ? choice.color : theme.accent
    }

    // Active profile in a large font with its color mark
    // (Item over the whole width: a layout without a stretched child would keep its width and stay on the left)
    Item {
        Layout.fillWidth: true
        Layout.topMargin: 4
        implicitHeight: activeColumn.implicitHeight
        visible: power.available && panel.active !== null

        ColumnLayout {
            id: activeColumn

            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: "Active profile"
                color: panel.enabled ? theme.text : theme.disabledText
                opacity: 0.7
            }

            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 12
                    height: 12
                    radius: 6
                    color: panel.profileColor(panel.active)
                    border.width: 1
                    border.color: theme.groupBorder
                }

                Label {
                    text: panel.active !== null ? panel.active.text : ""
                    font.bold: true
                    font.pixelSize: theme.fontPixelSize + 5
                    color: panel.enabled ? theme.text : theme.disabledText
                }
            }
        }
    }

    // Cards in the middle of the stretched panel
    Item { Layout.fillHeight: true }

    // One card per profile over the whole width: color bar of the profile, the active one in the accent of the theme,
    // a click anywhere on the card switches the profile (written immediately)
    Repeater {
        model: panel.choices

        delegate: AbstractButton {
            id: card

            required property var modelData

            readonly property bool selected: panel.active !== null && panel.active.value === modelData.value

            Layout.fillWidth: true
            implicitHeight: 40
            hoverEnabled: true
            enabled: panel.profile !== null && !panel.profile.readonly

            onClicked: if (!selected) panel.profile.value = modelData.value

            background: Rectangle {
                radius: theme.radius
                color: !card.enabled ? theme.disabledBackground
                     : card.selected ? theme.accent
                     : card.hovered ? theme.itemHoverBackground : theme.base
                gradient: card.enabled && card.selected && theme.styled ? selectedGradient : null
                border.width: theme.styled ? 2 : 1
                border.color: !card.enabled ? theme.disabledBackground
                            : card.selected || card.hovered ? theme.accentHover : theme.groupBorder

                Gradient {
                    id: selectedGradient

                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: theme.accent }
                    GradientStop { position: 1.0; color: theme.accentHover }
                }

                // Color of the profile (PowerProfileControl.ui)
                Rectangle {
                    x: 6
                    anchors.verticalCenter: parent.verticalCenter
                    width: 6
                    height: parent.height - 14
                    radius: 3
                    color: panel.profileColor(card.modelData)
                    opacity: card.enabled ? 1.0 : 0.4
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.5)
                }
            }

            contentItem: Item {
                Label {
                    x: 20
                    anchors.verticalCenter: parent.verticalCenter
                    text: card.modelData.text
                    font.bold: card.selected
                    color: !card.enabled ? theme.disabledText
                         : card.selected ? theme.accentText
                         : card.hovered ? theme.itemHoverText : theme.text
                }

                Label {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    visible: card.selected
                    text: "✓"
                    font.bold: true
                    color: card.enabled ? theme.accentText : theme.disabledText
                }
            }
        }
    }

    Item { Layout.fillHeight: true }

    // At the bottom, the label right of the switch (checkBox_customFnQ of PowerProfileControl)
    Field { source: power; path: "custom_fnq_enabled.current_value"; labelAfter: true }
}
