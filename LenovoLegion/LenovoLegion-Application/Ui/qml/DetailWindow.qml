// SPDX-License-Identifier: GPL-2.0
// Top level window of the generic GUI with the theme of the application (detail windows of the HW monitor),
// shown while "open" is true, closing the window sets "open" to false (checkable section of the page)
import QtQuick
import QtQuick.Controls

Window {
    id: window

    property bool open: false

    default property alias content: page.content

    // Copy of a value (copy buttons of the InfoSection rows)
    function copy(value) {
        clipboard.text = value
        clipboard.selectAll()
        clipboard.copy()
        clipboard.text = ""
    }

    // Not transient for the main window, the same as the widget windows (parent nullptr)
    transientParent: null
    color: theme.window
    visible: open

    onClosing: open = false

    // Clipboard: a hidden text edit (QML has no clipboard API)
    TextEdit {
        id: clipboard

        visible: false
    }

    SettingsPage {
        id: page

        anchors.fill: parent
        anchors.margins: 9
    }
}
