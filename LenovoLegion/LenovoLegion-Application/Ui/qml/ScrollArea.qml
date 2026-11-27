// SPDX-License-Identifier: GPL-2.0
// Scrollable part of a group (thread cells of CpuGrid / FrequencyGrid): only the content scrolls, the controls of
// the group above and below it stay visible. The page (SettingsPage fillHeight) is as high as the window, the group
// (Section stretchContent, Layout.fillHeight) fills it and the area takes the rest of the group.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

ScrollView {
    id: area

    default property alias content: column.data

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumHeight: 120

    // Not the height of the content: the page would be higher than the window and scroll as a whole
    implicitHeight: Layout.minimumHeight

    contentWidth: availableWidth
    contentHeight: column.implicitHeight
    clip: true

    // Space for the scroll bar when the content is longer than the area, the content is not covered
    rightPadding: verticalBar.size < 1.0 ? verticalBar.width + 4 : 0

    // Separated from the controls of the group above it: a gap with a line fading to the sides
    topPadding: 16

    Rectangle {
        parent: area
        x: 0
        y: 7
        width: area.width
        height: 1
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0;  color: "transparent" }
            GradientStop { position: 0.12; color: theme.groupBorder }
            GradientStop { position: 0.88; color: theme.groupBorder }
            GradientStop { position: 1.0;  color: "transparent" }
        }
    }

    // Shadow under the line while the content is scrolled (it goes under the controls)
    Rectangle {
        parent: area
        z: 1
        x: 0
        y: area.topPadding
        width: area.availableWidth
        height: 8
        opacity: area.contentItem.contentY > 0 ? 1 : 0
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.25) }
            GradientStop { position: 1.0; color: "transparent" }
        }

        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    ScrollBar.vertical: ThemedScrollBar {
        id: verticalBar

        parent: area
        x: area.mirrored ? 0 : area.width - width
        y: area.topPadding
        height: area.availableHeight
    }

    ColumnLayout {
        id: column

        width: area.availableWidth
        spacing: 6
    }
}
