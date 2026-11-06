// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI: title and sections in a scrollable column (UiDesign.md)
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

ScrollView {
    id: page

    property string title
    property bool   transparent: false   // inside a pane (tab widget of PreviewPage), the pane paints the background

    // Change bar at the bottom of the page: "N changes not applied · Cancel · Apply" of all data sources of the page,
    // shown only with changes, stays visible while scrolling. The groups (Section) show only their presets.
    property bool   changeBar: true

    // Minimum width of a column of the groups: a wide page shows the groups next to each other (0 = one column)
    property int    columnWidth: 0
    readonly property int  columns: column.columns
    readonly property Item grid: column
    readonly property bool isSettingsPage: true
    readonly property int  changeCount: sources.changeCount
    property bool   fillHeight: false    // content at least as high as the page, items with Layout.fillHeight are stretched

    default property alias content: column.data

    // State of the data sources of the page (tab of the page): without data the page is shown disabled with the reason
    readonly property bool   available: sources.available
    readonly property string platform:  sources.platform
    readonly property string reason:    sources.reason
    readonly property string description: sources.description

    // "page" inside PageSources would be its own property
    readonly property Item pageItem: page

    PageSources {
        id: sources

        page: pageItem
    }

    contentWidth: availableWidth

    // Theme of the application (Theme, context property "theme"), controls of the page take the colors from the palette
    palette.window:            theme.window
    palette.windowText:        theme.text
    palette.text:              theme.text
    palette.base:              theme.base
    palette.alternateBase:     theme.groupBackground
    // Fusion paints switches, combo boxes and spin buttons with the button color, push buttons are ThemedButton
    palette.button:            theme.base
    palette.buttonText:        theme.text
    palette.highlight:         theme.accent
    palette.highlightedText:   theme.accentText
    palette.mid:               theme.groupBorder
    palette.dark:              theme.groupBorder
    palette.disabled.text:       theme.disabledText
    palette.disabled.windowText: theme.disabledText
    palette.disabled.buttonText: theme.disabledText
    palette.disabled.button:     theme.disabledBackground

    font.pixelSize: theme.fontPixelSize

    background: Rectangle {
        color: page.transparent ? "transparent" : theme.window
    }

    // Space for the scroll bar when the content is longer than the page, the content is not covered
    rightPadding: verticalBar.size < 1.0 ? verticalBar.width + 4 : 0

    // Space for the change bar, the end of the content is not covered. Reserved always: the content does not shrink
    // (stretched pages) or the scroll range change (smaller window) and jump when the bar appears.
    bottomPadding: page.changeBar || changes.visible ? changes.height + 12 : 0

    Rectangle {
        id: changes

        parent: page
        z: 10
        visible: page.changeBar && page.changeCount > 0

        x: 8
        y: page.height - height - 8
        width: page.width - 16 - (verticalBar.size < 1.0 ? verticalBar.width + 4 : 0)
        height: changesRow.implicitHeight + 16

        radius: theme.radius
        color: theme.groupBackground
        border.width: theme.styled ? 2 : 1
        border.color: theme.accent

        RowLayout {
            id: changesRow

            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            // Not applied changes: the change color of the widget pages
            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: theme.modified
            }

            Label {
                Layout.fillWidth: true
                text: page.changeCount === 1 ? "1 change not applied" : page.changeCount + " changes not applied"
                color: theme.modified
                font.bold: true
            }

            ThemedButton {
                text: "Cancel"
                onClicked: sources.cancel()
            }

            ThemedButton {
                text: "Apply"
                onClicked: sources.apply()
            }
        }
    }

    ScrollBar.vertical: ThemedScrollBar {
        id: verticalBar

        parent: page
        x: page.mirrored ? 0 : page.width - width
        y: page.topPadding
        height: page.availableHeight
    }

    ColumnLayout {
        width: page.availableWidth
        height: page.fillHeight ? Math.max(implicitHeight, page.availableHeight) : implicitHeight
        spacing: 12

        Label {
            text: page.title
            visible: text.length > 0
            font.bold: true
            font.pixelSize: 18
        }

        Label {
            Layout.fillWidth: true

            visible: !page.available
            text: page.reason
            color: theme.modified
            font.bold: true
            wrapMode: Text.Wrap
        }

        // Groups below each other, on a wide page in columns (columnWidth: minimum width of a column, 0 = one column)
        GridLayout {
            id: column

            Layout.fillWidth: true
            Layout.fillHeight: page.fillHeight
            columns: page.columnWidth > 0 ? Math.max(1, Math.floor((width + columnSpacing) / (page.columnWidth + columnSpacing))) : 1
            rowSpacing: 12
            columnSpacing: 12
            enabled: page.available
        }
    }
}
