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

        ColumnLayout {
            id: column

            Layout.fillWidth: true
            Layout.fillHeight: page.fillHeight
            spacing: 12
            enabled: page.available
        }
    }
}
