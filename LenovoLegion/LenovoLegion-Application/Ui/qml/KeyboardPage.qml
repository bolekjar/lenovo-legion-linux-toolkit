// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (Lenovo driver): RGB keyboard (toolbar page Keyboard), the same data as RGBKeyboardDevice
// of the widget pages. GET sends RGBControllerRequest (DataSource.request), SET sends RGBControllerSetRequest
// (sendCommand, the daemon answers empty, the data are read again).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    id: page

    // No page title: the keyboard drawing with its view switcher is the first thing on the page
    columnWidth: 540

    // Flags of RGBControllerRequest / RGBControllerSetRequest (RGBController.proto)
    readonly property int requestAll:         0xFFF
    readonly property int setProfile:         0x001
    readonly property int setBrightness:      0x002
    readonly property int setLogo:            0x020
    readonly property int setEffects:         0x004
    readonly property int setResetEffects:    0x010
    readonly property int requestState:       0x080

    DataSource {
        id: rgb

        dataType: "RGB_CONTROLLER"
        request: ({ "request_flags": page.requestAll })
    }

    // Colors of all LEDs, read periodically in the worker thread (effects animate them)
    DataSource {
        id: rgbState

        dataType: "RGB_CONTROLLER"
        request: ({ "request_flags": page.requestState })
        interval: 100
    }

    readonly property bool hasDevice: rgb.revision >= 0 && rgb.value("britness.max", rgb.revision) > 0

    function send(flags, values) {
        const command = Object.assign({ "set_request_flags": flags }, values)
        rgb.sendCommand(command)
    }

    Label {
        Layout.fillWidth: true
        visible: rgb.available && !page.hasDevice
        text: "No RGB keyboard controller found"
        color: theme.disabledText
    }

    // Keyboard with the current colors of the LEDs over the whole width
    Section {
        visible: page.hasDevice && rgb.count("leds", rgb.revision) > 0
        Layout.columnSpan: page.columns

        KeyboardView {
            id: keyboard

            Layout.fillWidth: true
            // Selection by the mouse over the whole group (padding of Section)
            selectionMargin: 8
            source: rgb
            stateSource: rgbState
        }
    }

    /*
     * Effects (LedGroupEffect): the daemon gets always the whole list (SET_REQUEST_LED_GROUP_EFFECTS), the same as
     * RGBController::AddEffect of the widget pages: a new effect with LEDs takes them from the older effects (effects
     * without LEDs left are removed), a new effect without LEDs (whole device) replaces all.
     */
    readonly property var modes:   rgb.data("modes", rgb.revision) ?? []
    readonly property var effects: rgb.data("led_group_effects", rgb.revision) ?? []
    readonly property var leds:    rgb.data("leds", rgb.revision) ?? []
    readonly property int maxEffects: rgb.value("max_effects", rgb.revision) ?? 0

    // Flags of RGBController::Mode (RGBControllerInterface.h MODE_FLAG_*)
    readonly property int flagSpeed:          1 << 0
    readonly property int flagDirectionLR:    1 << 1
    readonly property int flagDirectionUD:    1 << 2
    readonly property int flagDirectionSpin:  1 << 3
    readonly property int flagPerLedColor:    1 << 4
    readonly property int flagModeColor:      1 << 5
    readonly property int flagRandomColor:    1 << 6
    readonly property int flagPerLedSelect:   1 << 9
    readonly property int flagAllLedSelect:   1 << 10
    readonly property int flagAllKeysSelect:  1 << 11

    // RGBController::ModeColorType
    readonly property int colorsNone:         0
    readonly property int colorsPerLed:       1
    readonly property int colorsModeSpecific: 2
    readonly property int colorsRandom:       3

    // Color 0x00BBGGRR of the daemon <-> QML color
    function toColor(value) {
        return Qt.rgba((value & 0xFF) / 255, ((value >> 8) & 0xFF) / 255, ((value >> 16) & 0xFF) / 255, 1)
    }

    function fromColor(color) {
        return Math.round(color.r * 255) | (Math.round(color.g * 255) << 8) | (Math.round(color.b * 255) << 16)
    }

    // Modes not working on Linux (the daemon marks them in the name, e.g. "Aura (Not working)", the audio modes need
    // the sound driver): they can be added, the name is shown without the mark and with a badge
    readonly property string unsupportedBadge: "Not on Linux"
    readonly property string unsupportedHint: "This effect does not work on Linux yet: it can be added, but the keyboard does not show it."

    function isUnsupported(mode) {
        return mode !== undefined && mode !== null && /\((not working|not supported)\)\s*$/i.test(mode.name)
    }

    function displayName(mode) {
        return mode.name.replace(/\s*\((not working|not supported)\)\s*$/i, "")
    }

    function modeOf(value) {
        return modes.find(item => item.value === value)
    }

    function modeName(value) {
        const mode = modeOf(value)
        return mode !== undefined ? displayName(mode) : "Mode " + value
    }

    // Indexes of the LEDs with the given values (codes), a code can be more LEDs (large keys)
    function ledIndexes(values) {
        const indexes = []

        for (let i = 0; i < leds.length; ++i)
            if (values.indexOf(leds[i].value) >= 0)
                indexes.push(i)

        return indexes
    }

    // LEDs {name, value} of the indexes, every code once
    function ledsOf(indexes) {
        const result = []

        for (const index of indexes) {
            const led = leds[index]
            if (led !== undefined && !result.some(item => item.value === led.value))
                result.push({ "name": led.name, "value": led.value })
        }

        return result
    }

    // Effect in the form of the command (LedGroupEffect)
    function effectCommand(effect) {
        return {
            "mode": effect.mode, "speed": effect.speed, "direction": effect.direction, "color_mode": effect.color_mode,
            "colors": effect.colors.slice(),
            "leds": effect.leds.map(led => ({ "name": led.name, "value": led.value }))
        }
    }

    function sendEffects(list) {
        send(setEffects, { "led_group_effects": list })
    }

    function addEffect(effect) {
        if (effect.leds.length === 0) {
            sendEffects([effect])
            return
        }

        const values = effect.leds.map(led => led.value)
        const list = []

        for (const old of effects) {
            const command = effectCommand(old)
            command.leds = command.leds.filter(led => values.indexOf(led.value) < 0)

            if (command.leds.length > 0)
                list.push(command)
        }

        list.push(effect)
        sendEffects(list)
    }

    function removeEffect(index) {
        sendEffects(effects.filter((item, i) => i !== index).map(item => effectCommand(item)))
    }

    // Color picked for the effect: one color (per LED) or the edited color of the mode colors
    Section {
        title: "Color"
        visible: page.hasDevice && page.modes.length > 0
        // Only for the effects with colors (per LED, mode specific)
        enabled: editor.colorType === page.colorsPerLed || editor.colorType === page.colorsModeSpecific

        ColorPicker {
            id: colorPicker

            Layout.fillWidth: true
            color: editor.editedColor
            onColorPicked: newColor => editor.setEditedColor(newColor)
        }
    }

    // New effect for the selected LEDs (groupBox of the effect of RGBKeyboardDevice)
    Section {
        id: editorSection

        title: "Effect"
        source: rgb
        visible: page.hasDevice && page.modes.length > 0

        Item {
            id: editor

            readonly property var  mode: modeBox.currentIndex >= 0 && modeBox.currentIndex < page.modes.length ? page.modes[modeBox.currentIndex] : null
            readonly property int  flags: mode !== null ? mode.flags : 0
            function has(flag) { return (flags & flag) !== 0 }

            // LEDs of the new effect by the selection mode of the mode
            readonly property string selection: has(page.flagPerLedSelect) ? "selected"
                                              : has(page.flagAllLedSelect) ? "all"
                                              : has(page.flagAllKeysSelect) ? "keys" : "device"
            readonly property var ledIndexes: selection === "selected" ? keyboard.selected
                                            : selection === "all" ? page.leds.map((led, index) => index)
                                            : selection === "keys" ? keyboard.keyLeds : []

            // Colors: per LED (one color), mode specific (count of the mode colors), random
            property int  colorType: page.colorsNone
            property color perLedColor: "#ff0000"
            property var  modeColors: []
            property int  editedIndex: 0

            readonly property color editedColor: colorType === page.colorsModeSpecific && editedIndex < modeColors.length ? modeColors[editedIndex] : perLedColor

            function setEditedColor(color) {
                if (colorType === page.colorsModeSpecific && editedIndex < modeColors.length) {
                    const colors = modeColors.slice()
                    colors[editedIndex] = color
                    modeColors = colors
                } else {
                    perLedColor = color
                }
            }

            // A new mode: its default colors, the first color type it supports
            onModeChanged: {
                if (mode === null)
                    return

                // Flags of the new mode (the binding of flags can be evaluated later)
                const modeFlags = mode.flags
                colorType = (modeFlags & page.flagPerLedColor) !== 0 ? page.colorsPerLed
                          : (modeFlags & page.flagModeColor) !== 0 ? page.colorsModeSpecific
                          : (modeFlags & page.flagRandomColor) !== 0 ? page.colorsRandom : page.colorsNone
                modeColors = mode.colors.slice(0, Math.max(1, mode.colors_min)).map(value => page.toColor(value))
                editedIndex = 0
            }

            readonly property bool full: page.effects.length >= page.maxEffects
            readonly property bool canAdd: mode !== null && !full && (selection !== "selected" || ledIndexes.length > 0)

            function effect() {
                return {
                    "mode": mode.value,
                    "speed": has(page.flagSpeed) ? Math.round(speedSlider.value) : 0,
                    "direction": directionBox.count > 0 ? directionBox.currentValue : mode.direction,
                    "color_mode": colorType,
                    "colors": colorType === page.colorsPerLed ? [page.fromColor(perLedColor)]
                            : colorType === page.colorsModeSpecific ? modeColors.map(color => page.fromColor(color)) : [],
                    "leds": page.ledsOf(ledIndexes)
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 8

            Label {
                text: "Mode"
            }

            ThemedComboBox {
                id: modeBox

                Layout.fillWidth: true
                textRole: "text"
                model: page.modes.map(item => page.isUnsupported(item)
                                      ? { "text": page.displayName(item), "badge": page.unsupportedBadge, "tooltip": page.unsupportedHint }
                                      : { "text": page.displayName(item) })
            }

            // The selected mode does not work on Linux
            Item {
                visible: page.isUnsupported(editor.mode)
                implicitWidth: 1
                implicitHeight: 1
            }

            Rectangle {
                Layout.fillWidth: true
                visible: page.isUnsupported(editor.mode)
                implicitHeight: unsupportedRow.implicitHeight + 12
                radius: theme.inputRadius
                color: Qt.rgba(1, 0.70, 0.28, 0.12)
                border.width: 1
                border.color: "#ffb347"

                RowLayout {
                    id: unsupportedRow

                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 8

                    Image {
                        source: "qrc:/images/icons/warning.png"
                        sourceSize: Qt.size(32, 32)
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        Layout.alignment: Qt.AlignTop
                        fillMode: Image.PreserveAspectFit
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        text: page.unsupportedHint
                    }
                }
            }

            Label {
                text: "LEDs"
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: editor.selection === "selected" && editor.ledIndexes.length === 0 ? theme.modified : theme.text
                text: editor.selection === "selected" ? (editor.ledIndexes.length > 0 ? editor.ledIndexes.length + (editor.ledIndexes.length === 1 ? " selected LED" : " selected LEDs")
                                                                                       : "Select LEDs on the keyboard")
                    : editor.selection === "all" ? "All LEDs"
                    : editor.selection === "keys" ? "All keys"
                    : "Whole device"
            }

            Label {
                visible: editor.has(page.flagSpeed)
                text: "Speed"
            }

            ThemedSlider {
                id: speedSlider

                Layout.fillWidth: true
                visible: editor.has(page.flagSpeed)
                from: editor.mode !== null ? editor.mode.speed_min : 0
                to: editor.mode !== null ? editor.mode.speed_max : 0
                stepSize: 1
                snapMode: Slider.SnapAlways
                value: editor.mode !== null ? editor.mode.speed : 0
            }

            Label {
                visible: directionBox.count > 0
                text: "Direction"
            }

            ThemedComboBox {
                id: directionBox

                Layout.fillWidth: true
                visible: count > 0
                textRole: "text"
                valueRole: "value"
                model: {
                    const items = []

                    if (editor.has(page.flagDirectionLR))
                        items.push({ "text": "Left", "value": 0 }, { "text": "Right", "value": 1 })
                    if (editor.has(page.flagDirectionUD))
                        items.push({ "text": "Up", "value": 2 }, { "text": "Down", "value": 3 })
                    if (editor.has(page.flagDirectionSpin))
                        items.push({ "text": "Spin left", "value": 4 }, { "text": "Spin right", "value": 5 })

                    return items
                }
            }

            Label {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 6
                visible: editor.has(page.flagPerLedColor) || editor.has(page.flagModeColor) || editor.has(page.flagRandomColor)
                text: "Colors"
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: editor.has(page.flagPerLedColor) || editor.has(page.flagModeColor) || editor.has(page.flagRandomColor)
                spacing: 4

                RowLayout {
                    spacing: 12

                    ThemedRadioButton {
                        visible: editor.has(page.flagPerLedColor)
                        text: "Per LED"
                        checked: editor.colorType === page.colorsPerLed
                        onClicked: editor.colorType = page.colorsPerLed
                    }

                    ThemedRadioButton {
                        visible: editor.has(page.flagModeColor)
                        text: "Mode specific"
                        checked: editor.colorType === page.colorsModeSpecific
                        onClicked: editor.colorType = page.colorsModeSpecific
                    }

                    ThemedRadioButton {
                        visible: editor.has(page.flagRandomColor)
                        text: "Random"
                        checked: editor.colorType === page.colorsRandom
                        onClicked: editor.colorType = page.colorsRandom
                    }
                }

                // Colors of the effect: click one to edit it in the color picker, + / - changes the count
                RowLayout {
                    visible: editor.colorType === page.colorsPerLed || editor.colorType === page.colorsModeSpecific
                    spacing: 6

                    Repeater {
                        model: editor.colorType === page.colorsModeSpecific ? editor.modeColors : [editor.perLedColor]

                        delegate: Rectangle {
                            required property var modelData
                            required property int index

                            readonly property bool edited: editor.colorType !== page.colorsModeSpecific || editor.editedIndex === index

                            width: 30
                            height: 30
                            radius: 6
                            color: modelData
                            border.width: edited ? 3 : 1
                            border.color: edited ? theme.accentHover : theme.groupBorder

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: editor.editedIndex = parent.index
                            }
                        }
                    }

                    ThemedButton {
                        visible: editor.colorType === page.colorsModeSpecific
                        implicitWidth: 32
                        text: "+"
                        enabled: editor.mode !== null && editor.modeColors.length < editor.mode.colors_max
                        onClicked: {
                            const next = editor.modeColors.length
                            editor.modeColors = editor.modeColors.concat([next < editor.mode.colors.length ? page.toColor(editor.mode.colors[next]) : editor.perLedColor])
                            editor.editedIndex = next
                        }
                    }

                    ThemedButton {
                        visible: editor.colorType === page.colorsModeSpecific
                        implicitWidth: 32
                        text: "−"
                        enabled: editor.mode !== null && editor.modeColors.length > Math.max(1, editor.mode.colors_min)
                        onClicked: {
                            editor.modeColors = editor.modeColors.slice(0, editor.modeColors.length - 1)
                            editor.editedIndex = Math.min(editor.editedIndex, editor.modeColors.length - 1)
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6

            Label {
                Layout.fillWidth: true
                visible: editor.full
                text: "Maximum of " + page.maxEffects + " effects reached"
                color: theme.modified
                wrapMode: Text.Wrap
            }

            Item {
                Layout.fillWidth: true
                visible: !editor.full
            }

            ThemedButton {
                text: "Add Effect"
                enabled: editor.canAdd
                onClicked: page.addEffect(editor.effect())
            }
        }
    }

    // Effects of the device: hover highlights its LEDs on the keyboard, click selects them, the cross removes it
    Section {
        title: "Effects"
        source: rgb
        visible: page.hasDevice && page.modes.length > 0

        Label {
            Layout.fillWidth: true
            visible: page.effects.length === 0
            text: "No effects"
            color: theme.disabledText
        }

        Repeater {
            model: page.effects

            delegate: Rectangle {
                id: effectRow

                required property var modelData
                required property int index

                readonly property var ledIndexes: page.ledIndexes(modelData.leds.map(led => led.value))

                Layout.fillWidth: true
                implicitHeight: 36
                radius: theme.radius / 2
                color: rowMouse.containsMouse ? theme.itemHoverBackground : "transparent"
                border.width: 1
                border.color: theme.groupBorder

                MouseArea {
                    id: rowMouse

                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onContainsMouseChanged: keyboard.marked = containsMouse ? effectRow.ledIndexes : []
                    onClicked: keyboard.selected = effectRow.ledIndexes.slice()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 4
                    spacing: 8

                    // Colors of the effect (random: rainbow, none: gray)
                    Row {
                        spacing: 2

                        Repeater {
                            model: effectRow.modelData.colors.length > 0 ? effectRow.modelData.colors : [-1]

                            delegate: Rectangle {
                                required property var modelData

                                width: 14
                                height: 14
                                radius: 3
                                color: modelData >= 0 ? page.toColor(modelData) : theme.disabledBackground
                                gradient: modelData < 0 && effectRow.modelData.color_mode === page.colorsRandom ? randomGradient : null
                                border.width: 1
                                border.color: theme.groupBorder

                                Gradient {
                                    id: randomGradient

                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: "red" }
                                    GradientStop { position: 0.5; color: "lime" }
                                    GradientStop { position: 1.0; color: "blue" }
                                }
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: page.modeName(effectRow.modelData.mode)
                        color: rowMouse.containsMouse ? theme.itemHoverText : theme.text
                    }

                    // The effect does not work on Linux
                    Rectangle {
                        visible: page.isUnsupported(page.modeOf(effectRow.modelData.mode))
                        implicitWidth: badgeLabel.implicitWidth + 12
                        implicitHeight: badgeLabel.implicitHeight + 4
                        radius: height / 2
                        color: "transparent"
                        border.width: 1
                        border.color: "#ffb347"

                        Label {
                            id: badgeLabel

                            anchors.centerIn: parent
                            text: page.unsupportedBadge
                            font.pixelSize: theme.fontPixelSize - 2
                            color: "#ffb347"
                        }

                        HoverHandler {
                            id: badgeHover
                        }

                        ThemedToolTip {
                            visible: badgeHover.hovered
                            delay: 500
                            text: page.unsupportedHint
                        }
                    }

                    Label {
                        text: effectRow.modelData.leds.length > 0 ? effectRow.ledIndexes.length + (effectRow.ledIndexes.length === 1 ? " LED" : " LEDs") : "Whole device"
                        color: theme.disabledText
                    }

                    ThemedButton {
                        implicitWidth: 28
                        implicitHeight: 26
                        text: "✕"
                        onClicked: page.removeEffect(effectRow.index)

                        ThemedToolTip {
                            visible: parent.hovered
                            delay: 500
                            text: "Remove the effect"
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6

            Label {
                Layout.fillWidth: true
                text: page.effects.length + " of " + page.maxEffects
                color: theme.disabledText
            }

            ThemedButton {
                text: "Clear All"
                enabled: page.effects.length > 0
                onClicked: page.sendEffects([])
            }

            ThemedButton {
                text: "Reset To Default"
                onClicked: page.send(page.setResetEffects, {})
            }
        }
    }

    // Brightness, device profile and logo (groupBox_Control of RGBKeyboardDevice), written immediately
    Section {
        title: "Control"
        source: rgb
        visible: page.hasDevice

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 12

            Label {
                text: "Brightness"
            }

            ThemedSlider {
                id: brightness

                Layout.fillWidth: true
                from: rgb.value("britness.min", rgb.revision)
                to: rgb.value("britness.max", rgb.revision)
                stepSize: 1
                snapMode: Slider.SnapAlways
                value: rgb.value("britness.current", rgb.revision)
                onMoved: {
                    page.send(page.setBrightness, { "britness.current": Math.round(value) })
                    value = Qt.binding(() => rgb.value("britness.current", rgb.revision))
                }
            }

            Label {
                Layout.preferredWidth: 40
                horizontalAlignment: Text.AlignRight
                text: brightness.value === brightness.from ? "Off" : Math.round(brightness.value)
            }

            Label {
                text: "Device profile"
            }

            ThemedComboBox {
                id: profile

                readonly property int minimum: rgb.value("profile.min", rgb.revision)

                Layout.fillWidth: true
                model: {
                    const result = []

                    for (let i = minimum; i <= rgb.value("profile.max", rgb.revision); ++i)
                        result.push("Profile " + i)

                    return result
                }
                currentIndex: rgb.value("profile.current", rgb.revision) - minimum
                onActivated: index => page.send(page.setProfile, { "profile.current": index + minimum })
            }

            Item { implicitWidth: 1 }

            Label {
                visible: rgb.value("logo_status.has_logo", rgb.revision) === true
                text: "Logo"
            }

            ThemedSwitch {
                visible: rgb.value("logo_status.has_logo", rgb.revision) === true
                checked: rgb.value("logo_status.logo_on", rgb.revision) === true
                onToggled: {
                    page.send(page.setLogo, { "logo_status.logo_on": checked })
                    checked = Qt.binding(() => rgb.value("logo_status.logo_on", rgb.revision) === true)
                }
            }

            Item { implicitWidth: 1 }
        }
    }

    // Device information (DeviceTypeInfo)
    Section {
        title: "Device"
        visible: page.hasDevice

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: rgb.value("device_info.name", rgb.revision) + "\n"
                  + "Vendor: " + rgb.value("device_info.vendor", rgb.revision) + "\n"
                  + "USB ID: " + Number(rgb.value("device_info.vendor_id", rgb.revision)).toString(16).padStart(4, "0")
                  + ":" + Number(rgb.value("device_info.product_id", rgb.revision)).toString(16).padStart(4, "0") + "\n"
                  + "Effects: " + rgb.count("led_group_effects", rgb.revision) + " of " + rgb.value("max_effects", rgb.revision)
        }
    }
}
