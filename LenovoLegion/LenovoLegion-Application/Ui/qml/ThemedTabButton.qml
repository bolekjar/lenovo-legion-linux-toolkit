// SPDX-License-Identifier: GPL-2.0
// Tab of the theme (QTabBar::tab, :selected, :!selected, :hover of the stylesheet)
import QtQuick
import QtQuick.Controls

TabButton {
    id: control

    readonly property bool selected: TabBar.tabBar !== null && TabBar.index === TabBar.tabBar.currentIndex

    // Page without data (e.g. Lenovo driver not loaded): grayed, but selectable, the reason in the tool tip
    property bool   available: true
    property string reason

    // Icon before the text (Lenovo Legion tabs) and the tool tip of an available tab
    property url    iconSource
    property string hint

    // First tab of another group (Lenovo tabs after the generic tabs): gap before the tab
    property bool   groupStart: false

    readonly property bool grayed: !enabled || !available

    // Width by the text (QTabBar), not the equal part of the tab bar
    width: implicitWidth

    topPadding: 10
    bottomPadding: selected ? 12 : 10
    leftPadding: 24 + (groupStart ? 16 : 0)
    leftInset: groupStart ? 16 : 0
    rightPadding: 24

    // Not selected tab is lower (margin-top: 4px)
    topInset: selected ? 0 : 4

    contentItem: Row {
        spacing: 6

        Image {
            anchors.verticalCenter: parent.verticalCenter
            visible: control.iconSource.toString().length > 0
            source: control.iconSource
            width: 16
            height: 16
            sourceSize: Qt.size(32, 32)
            fillMode: Image.PreserveAspectFit
            smooth: true
            opacity: control.grayed && !control.selected ? 0.5 : 1.0
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: control.text
            font.bold: control.selected && theme.styled
            font.pixelSize: theme.fontPixelSize
            color: control.grayed && !control.selected ? theme.disabledText
                 : control.selected ? theme.accentText
                 : control.hovered ? theme.text : theme.tabText
        }
    }

    // Rounded only at the top: taller rounded rectangle clipped at the bottom (border-bottom: none)
    background: Item {
        implicitHeight: 40
        clip: true

        Rectangle {
            width: parent.width
            height: parent.height + radius

            radius: theme.styled ? 10 : 2
            color: control.grayed && !control.selected ? theme.inputDisabledBackground
                 : control.selected ? theme.accent : theme.tabBackground
            border.width: theme.styled ? 2 : 1
            border.color: control.grayed && !control.selected ? theme.inputDisabledBorder
                        : (control.selected || control.hovered) ? theme.accent : theme.groupBorder

            gradient: control.selected && theme.styled ? selectedGradient : null

            Gradient {
                id: selectedGradient

                GradientStop { position: 0.0; color: theme.accent }
                GradientStop { position: 1.0; color: theme.accentPressed }
            }
        }
    }

    ThemedToolTip {
        readonly property string tip: control.available ? control.hint : (control.hint.length > 0 ? control.hint + "\n" : "") + control.reason

        visible: control.hovered && tip.length > 0
        delay: 500
        text: tip
    }
}
