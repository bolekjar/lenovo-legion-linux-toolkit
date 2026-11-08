// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (toolbar page "Profiles"): named snapshots of the hardware configuration, saved and applied
// by the daemon (PROFILES, ProfileCommand SAVE / LOAD / DELETE). Save form on the left, the saved profiles as cards on
// the right (Load, Edit = into the form, Delete). Questions in ConfirmDialog, results and errors in Notifications.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    id: page

    title: ""
    changeBar: false

    DataSource {
        id: profiles

        dataType: "PROFILES"
    }

    readonly property var list: {
        const result = []

        for (let i = 0; i < profiles.count("profiles", profiles.revision); ++i)
            result.push({ name: String(profiles.value("profiles[" + i + "].name", profiles.revision) ?? ""),
                          description: String(profiles.value("profiles[" + i + "].description", profiles.revision) ?? "") })

        return result
    }

    readonly property string enteredName: nameField.text.trim()
    readonly property bool   nameExists: list.some(profile => profile.name === enteredName)

    // Command to the daemon, the result in the notifications; true when done
    function command(action, name, description, done) {
        if (!profiles.sendCommand({ "action": action, "name": name, "description": description })) {
            notifications.notify("Profile '" + name + "'", profiles.lastError.length > 0 ? profiles.lastError : "The daemon did not accept the command")
            return false
        }

        notifications.notify("Profiles", done, "info")
        return true
    }

    function save() {
        const name = enteredName, description = descriptionField.text.trim()
        const run = () => {
            if (command("SAVE", name, description, "Profile '" + name + "' saved")) {
                nameField.text = ""
                descriptionField.text = ""
            }
        }

        if (nameExists)
            confirm.ask("Overwrite Profile", "Profile '" + name + "' already exists. Overwrite it with the current configuration?",
                        "Overwrite", true, run)
        else
            run()
    }

    function load(name) {
        confirm.ask("Load Profile", "Apply profile '" + name + "'? All its saved settings are applied to the hardware.", "Load", false, () => {
            if (command("LOAD", name, "", "Profile '" + name + "' applied"))
                profiles.reloadAllSources()
        })
    }

    function remove(name) {
        confirm.ask("Delete Profile", "Delete profile '" + name + "'? This cannot be undone.", "Delete", true, () => {
            command("DELETE", name, "", "Profile '" + name + "' deleted")
        })
    }

    function edit(profile) {
        nameField.text = profile.name
        descriptionField.text = profile.description
        nameField.forceActiveFocus()
    }

    // Title of the page
    Rectangle {
        id: hero

        Layout.fillWidth: true
        implicitHeight: heroLayout.implicitHeight + 36

        radius: theme.radius
        border.width: theme.styled ? 2 : 1
        border.color: theme.groupBorder
        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.groupBackground }
            GradientStop { position: 1.0; color: theme.styled ? theme.window : theme.groupBackground }
        }
        clip: true

        Rectangle {
            anchors.fill: parent
            anchors.margins: parent.border.width
            radius: parent.radius
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.22) }
                GradientStop { position: 0.6; color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.0) }
            }
        }

        Rectangle {
            x: parent.border.width
            y: parent.radius
            width: 4
            height: parent.height - 2 * parent.radius
            radius: 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: theme.accentHover }
                GradientStop { position: 1.0; color: theme.accent }
            }
        }

        RowLayout {
            id: heroLayout

            anchors.fill: parent
            anchors.margins: 18
            anchors.leftMargin: 28
            spacing: 20

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: "Profiles"
                    font.pixelSize: theme.fontPixelSize + 12
                    font.bold: true
                    color: theme.text
                }

                Label {
                    Layout.fillWidth: true
                    text: "Named snapshots of the whole hardware configuration. The daemon saves and applies them, so they work without this application too."
                    color: theme.disabledText
                    wrapMode: Text.Wrap
                }
            }

            // Number of the saved profiles
            Rectangle {
                implicitWidth: countColumn.implicitWidth + 32
                implicitHeight: countColumn.implicitHeight + 16
                radius: theme.radius
                color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.14)
                border.width: 1
                border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.55)

                ColumnLayout {
                    id: countColumn

                    anchors.centerIn: parent
                    spacing: 0

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: page.list.length
                        font.pixelSize: theme.fontPixelSize + 10
                        font.bold: true
                        color: theme.text
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: page.list.length === 1 ? "saved profile" : "saved profiles"
                        color: theme.disabledText
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        // New profile from the current configuration (or overwrite / new description of an existing one)
        Section {
            Layout.preferredWidth: 380
            Layout.maximumWidth: 380
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignTop
            title: "Save Current Configuration"
            source: profiles

            Label {
                Layout.fillWidth: true
                text: "Saves the current settings of the hardware: power profile, power limits, fan curve, CPU and GPU settings."
                color: theme.disabledText
                wrapMode: Text.Wrap
            }

            Label {
                Layout.topMargin: 6
                text: "Name"
            }

            ThemedTextField {
                id: nameField

                Layout.fillWidth: true
                placeholderText: "e.g. Gaming, Silent, Battery"
                maximumLength: 64
                onAccepted: if (page.enteredName.length > 0) page.save()

                // Name of an existing profile: its description (not lost by overwriting)
                onTextChanged: {
                    const profile = page.list.find(item => item.name === text.trim())
                    if (profile !== undefined && descriptionField.text.length === 0)
                        descriptionField.text = profile.description
                }
            }

            Label {
                text: "Description"
            }

            ThemedTextField {
                id: descriptionField

                Layout.fillWidth: true
                placeholderText: "Optional"
                maximumLength: 200
                onAccepted: if (page.enteredName.length > 0) page.save()
            }

            // Existing name: the profile is overwritten (the place is kept, no jumping)
            Label {
                Layout.fillWidth: true
                text: "A profile with this name exists, it will be overwritten."
                color: theme.modified
                wrapMode: Text.Wrap
                opacity: page.nameExists ? 1 : 0
            }

            RowLayout {
                Layout.fillWidth: true

                ThemedButton {
                    text: "Clear"
                    enabled: nameField.text.length > 0 || descriptionField.text.length > 0
                    onClicked: {
                        nameField.text = ""
                        descriptionField.text = ""
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                ThemedButton {
                    text: page.nameExists ? "Overwrite Profile" : "Save Profile"
                    enabled: page.enteredName.length > 0 && profiles.available
                    onClicked: page.save()
                }
            }
        }

        // Saved profiles as cards
        Section {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            title: "Saved Profiles"
            source: profiles

            // No profiles yet
            ColumnLayout {
                Layout.fillWidth: true
                Layout.topMargin: 24
                Layout.bottomMargin: 24
                visible: page.list.length === 0
                spacing: 6

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: "No saved profiles"
                    font.bold: true
                    font.pixelSize: theme.fontPixelSize + 2
                    color: theme.text
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Set the hardware as you like it and save it on the left."
                    color: theme.disabledText
                }
            }

            GridLayout {
                id: cards

                Layout.fillWidth: true
                visible: page.list.length > 0
                columns: Math.max(1, Math.floor((width + columnSpacing) / (280 + columnSpacing)))
                columnSpacing: 10
                rowSpacing: 10

                Repeater {
                    model: page.list

                    delegate: Rectangle {
                        id: card

                        required property var modelData

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        implicitHeight: cardLayout.implicitHeight + 24
                        radius: theme.radius
                        color: cardHover.hovered ? theme.itemHoverBackground : theme.window
                        border.width: 1
                        border.color: cardHover.hovered ? theme.accentHover : theme.groupBorder

                        HoverHandler {
                            id: cardHover
                        }

                        ColumnLayout {
                            id: cardLayout

                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // Initial of the profile
                                Rectangle {
                                    Layout.alignment: Qt.AlignTop
                                    implicitWidth: 36
                                    implicitHeight: 36
                                    radius: 18
                                    gradient: Gradient {
                                        GradientStop { position: 0.0; color: theme.accentHover }
                                        GradientStop { position: 1.0; color: theme.accent }
                                    }

                                    Label {
                                        anchors.centerIn: parent
                                        text: card.modelData.name.charAt(0).toUpperCase()
                                        font.bold: true
                                        font.pixelSize: theme.fontPixelSize + 4
                                        color: theme.accentText
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        Layout.fillWidth: true
                                        text: card.modelData.name
                                        font.bold: true
                                        font.pixelSize: theme.fontPixelSize + 2
                                        color: cardHover.hovered ? theme.itemHoverText : theme.text
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: card.modelData.description.length > 0 ? card.modelData.description : "No description"
                                        font.italic: card.modelData.description.length === 0
                                        color: cardHover.hovered ? theme.itemHoverText : theme.disabledText
                                        wrapMode: Text.Wrap
                                        maximumLineCount: 3
                                        elide: Text.ElideRight
                                    }
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                ThemedButton {
                                    Layout.fillWidth: true
                                    text: "Load"
                                    onClicked: page.load(card.modelData.name)
                                }

                                ThemedButton {
                                    id: editButton

                                    text: "Edit"
                                    onClicked: page.edit(card.modelData)

                                    ThemedToolTip {
                                        visible: editButton.hovered
                                        delay: 500
                                        text: "Into the form: save the current configuration under this name or change the description"
                                    }
                                }

                                ThemedButton {
                                    id: deleteButton

                                    implicitWidth: 36
                                    text: "✕"
                                    onClicked: page.remove(card.modelData.name)

                                    ThemedToolTip {
                                        visible: deleteButton.hovered
                                        delay: 500
                                        text: "Delete the profile"
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ConfirmDialog {
        id: confirm
    }

    // Bottom right over the page
    Notifications {
        id: notifications

        parent: page
        z: 100
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 24
        anchors.bottomMargin: 24
        width: 380
        height: parent.height / 2
    }
}
