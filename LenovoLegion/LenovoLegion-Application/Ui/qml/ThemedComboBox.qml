// SPDX-License-Identifier: GPL-2.0
// Combo box of the theme (QComboBox, QComboBox:hover/focus/disabled of the stylesheet)
import QtQuick
import QtQuick.Controls

ComboBox {
    id: combo

    // Item of the list (QComboBox QAbstractItemView::item, :selected, :hover of the stylesheet)
    delegate: ItemDelegate {
        id: item

        width: ListView.view ? ListView.view.width : combo.width
        implicitHeight: 32
        leftPadding: 12
        rightPadding: 12

        // Arrays of strings (no textRole) and arrays of objects with textRole (choices of a field)
        text: combo.textRole.length > 0 ? modelData[combo.textRole] : modelData
        highlighted: combo.highlightedIndex === index

        // Tool tip of the value ((legion.ui.value_tooltip) of choices of a field)
        ThemedToolTip {
            visible: item.hovered && text.length > 0
            delay: 500
            text: typeof modelData === "object" && modelData !== null && modelData.tooltip !== undefined ? modelData.tooltip : ""
        }

        // Badge of the value (objects with badge, e.g. "Not on Linux"): muted text and the badge on the right
        readonly property string badge: typeof modelData === "object" && modelData !== null && modelData.badge !== undefined ? modelData.badge : ""

        contentItem: Item {
            implicitHeight: label.implicitHeight

            Text {
                id: label

                anchors.left: parent.left
                anchors.right: badgeItem.visible ? badgeItem.left : parent.right
                anchors.rightMargin: badgeItem.visible ? 8 : 0
                anchors.verticalCenter: parent.verticalCenter
                text: item.text
                font.bold: item.highlighted && theme.styled
                font.pixelSize: theme.fontPixelSize
                color: item.highlighted ? theme.accentText
                     : item.hovered ? theme.itemHoverText : theme.text
                opacity: item.badge.length > 0 && !item.highlighted ? 0.6 : 1
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            Rectangle {
                id: badgeItem

                visible: item.badge.length > 0
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: badgeText.implicitWidth + 12
                implicitHeight: badgeText.implicitHeight + 4
                radius: height / 2
                color: "transparent"
                border.width: 1
                border.color: item.highlighted ? theme.accentText : "#ffb347"

                Text {
                    id: badgeText

                    anchors.centerIn: parent
                    text: item.badge
                    font.pixelSize: theme.fontPixelSize - 2
                    color: item.highlighted ? theme.accentText : "#ffb347"
                }
            }
        }

        background: Rectangle {
            radius: 6
            color: item.highlighted ? theme.accent
                 : item.hovered ? theme.itemHoverBackground : "transparent"

            // Selection: gradient to the hover accent and the left bar
            Rectangle {
                anchors.fill: parent
                visible: item.highlighted && theme.styled
                radius: parent.radius

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: theme.accent }
                    GradientStop { position: 1.0; color: theme.accentHover }
                }
            }

            Rectangle {
                visible: item.highlighted && theme.styled
                width: 4
                height: parent.height
                color: theme.accentHover
            }
        }
    }

    // List (QComboBox QAbstractItemView of the stylesheet)
    popup: Popup {
        y: combo.height + 2
        width: combo.width
        padding: 4
        implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

        contentItem: ListView {
            clip: true
            spacing: 2
            implicitHeight: contentHeight
            model: combo.popup.visible ? combo.delegateModel : null
            currentIndex: combo.highlightedIndex
        }

        background: Rectangle {
            color: theme.popupBackground
            radius: theme.styled ? 8 : 2
            border.width: theme.styled ? 2 : 1
            border.color: theme.styled ? theme.accent : theme.groupBorder
        }
    }

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: 28

        radius: theme.inputRadius
        color: combo.enabled ? (theme.styled ? "transparent" : theme.base) : theme.inputDisabledBackground
        border.width: theme.styled ? 2 : 1
        border.color: !combo.enabled ? theme.inputDisabledBorder
                    : (combo.hovered || combo.visualFocus || combo.popup.visible) ? theme.accentHover
                    : (theme.styled ? theme.accent : theme.groupBorder)
    }
}
