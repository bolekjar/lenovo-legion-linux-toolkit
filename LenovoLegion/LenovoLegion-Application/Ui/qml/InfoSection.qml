// SPDX-License-Identifier: GPL-2.0
// Group of read-only values (InformationPage): rows "label  value", empty values are hidden. The values can be
// selected by the mouse, the copy button of a row (shown on hover) emits copyRequested with the value. Children of
// an InfoSection (e.g. chips) are placed under the rows.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Section {
    id: infoSection

    // Rows [{ label, value, hint, reportOnly }]: hint is a tool tip of the value (e.g. the raw value of a decoded one),
    // reportOnly rows are not shown (only in the text report of the page, e.g. shown by chips)
    property var    rows: []
    property string emptyText: "No information"

    signal copyRequested(string label, string value)

    readonly property var shownRows: rows.filter(row => row.reportOnly !== true && row.value !== undefined && row.value !== null
                                                        && String(row.value).length > 0)

    // Width of the labels: the longest label of the group, the values aligned in one column
    readonly property real labelWidth: {
        let width = 0

        for (const row of shownRows)
            width = Math.max(width, labelMetrics.advanceWidth(row.label))

        return Math.ceil(width)
    }

    FontMetrics {
        id: labelMetrics

        font.pixelSize: theme.fontPixelSize
    }

    Label {
        Layout.fillWidth: true

        visible: infoSection.shownRows.length === 0
        text: infoSection.emptyText
        color: theme.disabledText
        wrapMode: Text.Wrap
    }

    Repeater {
        model: infoSection.shownRows

        delegate: Rectangle {
            id: row

            required property var modelData

            Layout.fillWidth: true
            implicitHeight: rowLayout.implicitHeight + 8
            radius: theme.inputRadius
            color: rowHover.hovered ? theme.itemHoverBackground : "transparent"

            HoverHandler {
                id: rowHover
            }

            RowLayout {
                id: rowLayout

                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: 4
                spacing: 12

                Label {
                    Layout.preferredWidth: infoSection.labelWidth
                    Layout.alignment: Qt.AlignBaseline
                    text: row.modelData.label
                    color: rowHover.hovered ? theme.itemHoverText : theme.disabledText
                }

                // Selectable value
                TextEdit {
                    id: valueText

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    readOnly: true
                    selectByMouse: true
                    wrapMode: TextEdit.Wrap
                    text: String(row.modelData.value)
                    font.pixelSize: theme.fontPixelSize
                    font.bold: true
                    color: rowHover.hovered ? theme.itemHoverText : theme.text
                    selectionColor: theme.accent
                    selectedTextColor: theme.accentText

                    ThemedToolTip {
                        visible: valueHover.hovered && text.length > 0
                        delay: 600
                        text: row.modelData.hint !== undefined ? row.modelData.hint : ""
                    }

                    HoverHandler {
                        id: valueHover

                        cursorShape: Qt.IBeamCursor
                    }
                }

                // Copy of the value (two sheets), the place is kept (no jumping of the values)
                AbstractButton {
                    id: copyButton

                    Layout.alignment: Qt.AlignTop
                    implicitWidth: 22
                    implicitHeight: 20
                    opacity: rowHover.hovered ? 1 : 0
                    hoverEnabled: true
                    onClicked: infoSection.copyRequested(row.modelData.label, String(row.modelData.value))

                    contentItem: Item {
                        Rectangle {
                            x: 7
                            y: 2
                            width: 10
                            height: 12
                            radius: 2
                            color: "transparent"
                            border.width: 1.5
                            border.color: copyButton.hovered ? theme.accentHover : theme.disabledText
                        }

                        Rectangle {
                            x: 3
                            y: 6
                            width: 10
                            height: 12
                            radius: 2
                            color: row.color.a > 0 ? theme.itemHoverBackground : theme.groupBackground
                            border.width: 1.5
                            border.color: copyButton.hovered ? theme.accentHover : theme.disabledText
                        }
                    }

                    background: null

                    ThemedToolTip {
                        visible: copyButton.hovered
                        delay: 500
                        text: "Copy " + row.modelData.label
                    }
                }
            }
        }
    }
}
