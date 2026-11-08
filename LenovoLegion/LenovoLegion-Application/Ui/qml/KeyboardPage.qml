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
            Layout.fillWidth: true
            source: rgb
            stateSource: rgbState
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
