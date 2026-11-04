// SPDX-License-Identifier: GPL-2.0
// Group of fields of one data source, error of the last request and optional Apply/Cancel
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

GroupBox {
    id: section

    property DataSource source
    property bool       applyBar: false
    property var        presets: []      // buttons of the ApplyBar, see ApplyBar.presets
    property bool       checkable: false // check box in the title bar (checkable QGroupBox), e.g. opens a detail window
    property bool       checked: false
    property url        titleIcon        // icon before the title (Lenovo Legion: PowerProfilesPanel)

    default property alias content: fields.data

    Layout.fillWidth: true

    topPadding: (title.length > 0 ? titleBar.height : 0) + padding

    // Group and title bar of the theme (QGroupBox, QGroupBox::title of the stylesheet)
    background: Rectangle {
        color: theme.groupBackground
        border.color: theme.groupBorder
        border.width: theme.styled ? 2 : 1
        radius: theme.radius
    }

    label: Rectangle {
        id: titleBar

        visible: section.title.length > 0
        width: section.width
        height: titleText.implicitHeight + 12
        color: theme.titleBackground
        radius: theme.radius

        // Square bottom corners of the title bar
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.radius
            color: parent.color
        }

        // Check box of a checkable group (QGroupBox::indicator of the stylesheet)
        Rectangle {
            id: indicator

            visible: section.checkable
            x: 12
            anchors.verticalCenter: parent.verticalCenter
            width: 16
            height: 16
            radius: theme.styled ? 6 : 2
            border.width: theme.styled ? 2 : 1
            border.color: section.checked ? theme.accentHover : (theme.styled ? theme.accent : theme.groupBorder)
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0; color: section.checked ? theme.accent : theme.window }
                GradientStop { position: 1; color: section.checked ? theme.accentHover : theme.window }
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: section.checkable
            cursorShape: Qt.PointingHandCursor
            onClicked: section.checked = !section.checked
        }

        Image {
            id: titleImage

            x: section.checkable ? indicator.x + indicator.width + 6 : 12
            anchors.verticalCenter: parent.verticalCenter
            visible: section.titleIcon.toString().length > 0
            source: section.titleIcon
            width: visible ? 16 : 0
            height: 16
            sourceSize: Qt.size(32, 32)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Label {
            id: titleText

            x: titleImage.x + (titleImage.visible ? titleImage.width + 6 : 0)
            anchors.verticalCenter: parent.verticalCenter
            text: section.title
            color: theme.titleText
            font.bold: true
        }
    }

    ColumnLayout {
        width: parent.width
        // Whole height of a stretched group (PowerProfilesPanel), the same as the content otherwise
        height: parent.height

        ColumnLayout {
            id: fields

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Label {
            Layout.fillWidth: true

            // A disabled page without data shows the reason once (SettingsPage), not the error of every section
            visible: section.source !== null && section.source.lastError.length > 0 && (section.enabled || section.source.available)
            text: section.source !== null ? section.source.lastError : ""
            color: "red"
            wrapMode: Text.Wrap
        }

        ApplyBar {
            Layout.fillWidth: true

            visible: section.applyBar
            source: section.source
            presets: section.presets
        }
    }
}
