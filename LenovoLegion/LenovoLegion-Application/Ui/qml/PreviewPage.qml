// SPDX-License-Identifier: GPL-2.0
// Pages of the generic GUI (toolbar page "Preview", UiDesign.md): HW monitor always visible over the whole width,
// below it the power profiles (always visible) next to the settings pages in tabs
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: theme.window

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Height of the content, no scrolling and no title of the page
        HwMonitorPage {
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight + topPadding + bottomPadding
            title: ""
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // Next to the pane of the pages (below the tab bar) down to the bottom
            PowerProfilesPanel {
                Layout.preferredWidth: 220
                Layout.fillWidth: false
                Layout.fillHeight: true
                Layout.topMargin: tabs.height
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                ThemedTabBar {
                    id: tabs

                    // Tabs over the top border of the pane
                    z: 1

                    // Width of the column, more tabs than the width are flicked (the layout is not widened)
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    Layout.preferredWidth: 0

                    // Generic tabs (standard interfaces), then the Lenovo tabs (Lenovo Legion kernel driver),
                    // the group and the state come from the data sources of the page ((legion.ui.platform))
                    Repeater {
                        model: pages.children

                        delegate: ThemedTabButton {
                            required property var modelData
                            required property int index

                            text: modelData.tabText
                            available: modelData.available
                            reason: modelData.reason
                            hint: modelData.description
                            iconSource: modelData.platform === "lenovo" ? "qrc:/images/icons/LenovoLegionIco.png" : ""
                            groupStart: index > 0 && modelData.platform !== pages.children[index - 1].platform
                        }
                    }
                }

                // Pane of the tab widget with the pages (QTabWidget::pane of the stylesheet: gradient, border,
                // radius 12 px, padding 8 px, top -1 px under the tabs)
                Rectangle {
                    id: pane

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.topMargin: -1

                    radius: theme.styled ? 12 : 2
                    border.width: theme.styled ? 2 : 1
                    border.color: theme.paneBorder
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: theme.groupBackground }
                        GradientStop { position: 1.0; color: theme.window }
                    }

                    StackLayout {
                        id: pages

                        anchors.fill: parent
                        anchors.margins: pane.border.width + 8

                        currentIndex: tabs.currentIndex

                        // The tab names the page, no title (the first section is aligned with the power profiles)
                        CpuControlPage   { title: ""; transparent: true; readonly property string tabText: "CPU Control" }
                        CpuFrequencyPage { title: ""; transparent: true; readonly property string tabText: "CPU Freq Control" }
                        CpuPowerPage     { title: ""; transparent: true; readonly property string tabText: "CPU Power" }
                        GpuPage    { title: ""; transparent: true; readonly property string tabText: "GPU" }
                        PowerPage  { title: ""; transparent: true; readonly property string tabText: "Power" }
                        FanCurvePage { title: ""; transparent: true; readonly property string tabText: "Fan Curve" }
                        DevicePage { title: ""; transparent: true; readonly property string tabText: "Device" }
                    }
                }
            }
        }
    }
}
