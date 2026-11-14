// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (toolbar page "Help"): live system check (daemon, drivers, NVIDIA, RGB keyboard) and the help
// topics, a list of the topics on the left and the selected topic on the right (Markdown).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    id: page

    title: ""
    changeBar: false

    readonly property string projectUrl: "https://github.com/bolekjar/lenovo-legion-linux-toolkit"

    // Sources of the system check: a source is available when the daemon answers and its module has the data
    DataSource {
        id: daemonCheck

        dataType: "CPU_TOPOLOGY"
    }

    DataSource {
        id: lenovoCheck

        dataType: "POWER_PROFILE"
    }

    DataSource {
        id: msrCheck

        dataType: "CPU_INTEL_MSR"
    }

    DataSource {
        id: nvidiaCheck

        dataType: "NVIDIA_NWML"
    }

    // The same test of a keyboard as KeyboardPage: the module answers also without a keyboard (no brightness)
    DataSource {
        id: rgbCheck

        dataType: "RGB_CONTROLLER"
        request: ({ "request_flags": 0xFFF })
    }

    readonly property bool keyboardFound: rgbCheck.available && rgbCheck.value("britness.max", rgbCheck.revision) > 0

    // Data providers of the daemon: modules (state 0 running, 1 failed, 2 not loaded) and providers of the framework
    DataSource {
        id: modulesCheck

        dataType: "MODULES"
    }

    readonly property var modules: modulesCheck.data("modules", modulesCheck.revision) ?? []
    readonly property var brokenModules: modules.filter(module => module.state !== 0)

    // Connection of the GUI (a missing module of the data of daemonCheck is not a missing connection)
    readonly property bool daemonConnected: typeof appState !== "undefined" && appState !== null ? appState.connected : daemonCheck.available

    // Reason of a failed check: the daemon module of the source when it does not run, otherwise the hint of the check
    function failReason(source, text) {
        return source.moduleError.length > 0 ? source.moduleError : text
    }

    readonly property var checks: [
        { name: "Daemon", ok: page.daemonConnected, required: true,
          okText: "Connected",
          failText: "Not connected. Start the service: sudo systemctl enable --now lenovo-legion-daemon.service. "
                  + "Your user must be in the group legion (log in again after adding it)." },
        { name: "Lenovo Legion driver", ok: lenovoCheck.available, required: true,
          okText: "lenovo_legion loaded",
          failText: page.daemonConnected ? page.failReason(lenovoCheck, "Kernel module lenovo_legion is not loaded or the notebook is not supported. "
                                                                    + "Check the output of: sudo dmesg | grep lenovo-legion")
                                         : "Unknown, the daemon is not connected." },
        { name: "Intel CPU voltage", ok: msrCheck.available, required: false,
          okText: "legion_intel_msr loaded",
          failText: page.daemonConnected ? page.failReason(msrCheck, "Kernel module legion_intel_msr is not loaded (Intel CPUs only). "
                                                                 + "It is loaded by /etc/modules-load.d/lenovo-legion.conf.")
                                         : "Unknown, the daemon is not connected." },
        { name: "NVIDIA GPU", ok: nvidiaCheck.available, required: false,
          // the module answers also without a GPU: available only with the GPU name ((legion.ui.available_from))
          okText: nvidiaCheck.value("name", nvidiaCheck.revision) + " (NVML)",
          failText: page.daemonConnected ? page.failReason(nvidiaCheck, "No NVIDIA GPU or the proprietary NVIDIA driver (NVML) is not installed.")
                                         : "Unknown, the daemon is not connected." },
        { name: "RGB keyboard", ok: page.keyboardFound, required: false,
          okText: "Keyboard found",
          failText: page.daemonConnected ? page.failReason(rgbCheck, "No supported RGB keyboard (USB 048d:c197, 048d:c9xx) was found.")
                                         : "Unknown, the daemon is not connected." },
        { name: "Daemon modules", ok: modulesCheck.available && page.brokenModules.length === 0, required: false,
          // list of the modules after a click on the row
          details: page.modules.map(module => ({
              name: module.name,
              state: module.state,
              text: (module.framework ? "daemon " : "") + (module.version.length > 0 ? "v" + module.version : "")
                    + (module.state === 1 ? " – failed: " + module.error : module.state === 2 ? "not loaded: " + module.error : "")
          })),
          okText: page.modules.filter(module => !module.framework).length + " modules running",
          failText: !page.daemonConnected ? "Unknown, the daemon is not connected."
                  : !modulesCheck.available ? "The daemon does not report its modules (older version of the daemon)."
                  : page.brokenModules.map(module => module.name + (module.state === 1 ? " failed: " : " not loaded: ") + module.error).join("\n")
                    + "\nDetails are in the log of the daemon." }
    ]

    // Topics of the help (Markdown)
    readonly property var topics: [
        { title: "Getting Started", icon: "device", text:
"## Getting Started

Lenovo Legion Linux Toolkit controls the features of Lenovo Legion notebooks on Linux: power profiles, power limits, \
fan curve, CPU and GPU settings, voltage offsets and the RGB keyboard. It is an unofficial replacement of Lenovo Vantage \
and Legion Space and it is **not affiliated with Lenovo**.

The toolbar at the top opens the pages:

- **Home** – hardware monitor, power profiles and all hardware settings in tabs
- **Profiles** – named profiles of the whole configuration, saved by the daemon
- **Keyboard** – RGB keyboard: effects, colors, brightness and logo
- **Settings** – the application and the daemon
- **Information** – the machine, processor, graphics and BIOS
- **Help** – this page

The status bar at the bottom shows the connection to the daemon (with **Reconnect**), the power profile and the \
battery: the charge, charging or the time left; hover it for the details (energy, health, power, cycles). Most pages \
need the daemon, without it they show what to check; Settings and Help work also without it. The logo at the top left \
opens **About**.

### Applying changes

Pages with several values show a bar **N changes not applied · Cancel · Apply** at the bottom. Nothing is written \
until you press **Apply**. Changed values are marked with the change color. Switches like the power profile, the \
touchpad or the keyboard brightness are written immediately." },

        { title: "Home", icon: "cpu", text:
"## Home

**HW Monitoring** at the top shows temperatures, frequencies, power and utilization of the CPU and the GPU and the fan \
speeds. The check box in the title of the CPU and GPU groups opens a window with the details (frequency of every CPU, \
GPU clocks and memory).

**Power Profiles** on the left switch the thermal mode of the notebook: *Quiet*, *Balanced*, *Performance*, *Extreme* \
and *Custom*. Fn+Q switches them too. The power limits and the fan curve belong to the profile. **Saved Profiles** \
below apply a profile saved on the Profiles page at once.

The tabs:

- **CPU Control** – SMT (Hyper-Threading), cores on/off and the governor of every CPU; P-cores and E-cores are \
shown separately on hybrid Intel CPUs
- **CPU Freq Control** – minimum and maximum frequency of every CPU
- **CPU Power** – power state of the CPU (intel_pstate limits, turbo) and the CPU voltage offsets
- **GPU** – NVIDIA GPU and memory clock offsets with presets
- **Power** – CPU and GPU power limits and temperature limits of the current power profile
- **Fan Curve** – fan curve with 10 points (drag the points, the curve stays rising), fan full speed
- **Device** – touchpad, Windows key, Fn lock, camera power, battery conservation mode (charging stops at about 80 %), rapid charge, USB charging when off and the GPU mode (hybrid, iGPU only, dGPU only; needs a restart)

Tabs with the Lenovo icon need the Lenovo Legion kernel driver; without it they are shown grayed with the reason." },

        { title: "Keyboard", icon: "device", text:
"## Keyboard

The drawing shows the notebook in three views: **Keyboard** (from above), **Front** (front light bar) and **Back** \
(rear light bar around the vents and the logo). The keys and the light bars show their current colors.

- **Click** selects a key or a light segment, **Ctrl+click** adds or removes it, **drag** selects all in the rectangle
- **Select all** selects the LEDs of all views, **Clear** nothing; the views show the number of their selected LEDs

**Effect** adds a new effect: choose the mode, its speed, direction and colors (per LED, mode specific or random) \
and press **Add Effect**. A new effect takes its LEDs from the older effects. Modes marked **Not on Linux** can be \
selected, but the keyboard does not show them (e.g. the audio modes need the sound driver).

**Effects** lists the effects of the keyboard: hovering highlights their LEDs, a click selects them, ✕ removes the \
effect. **Reset To Default** restores the effects of the keyboard. **Control** sets the brightness, the keyboard \
profile and the logo light." },

        { title: "Profiles", icon: "limits", text:
"## Profiles

A profile is a named snapshot of the current hardware configuration (power profile, power limits, fan curve, CPU \
and GPU settings). The profiles are saved and applied by the daemon, so they work without the GUI too.

- **Save Current Configuration** saves the current settings under a name; an existing name is overwritten after \
confirmation and keeps its description
- **Load** applies the profile after confirmation, all pages read the data again
- **Edit** puts the name and the description into the form (to save the current settings under this name)
- **✕** deletes the profile after confirmation

**Saved Profiles** on the Home page applies a profile with one click.

In **Settings** you can let the daemon apply the saved settings when it starts and save them when it exits." },

        { title: "Settings", icon: "power", text:
"## Settings

All changes are saved at once. The page opens also without the daemon (settings of the application, Reconnect).

**Appearance** – the theme: No Theme (colors of the system), White, Gray, Dark; the cards show a preview and the \
theme is applied at once to all pages.

**Application**

- Start minimized to the system tray, minimize to the tray on close
- Debug and trace logging of the GUI

**Daemon**

- Apply the saved settings when the daemon starts, save the settings when it exits
- Debug and trace logging of the daemon
- **Save Current Configuration** saves the current hardware settings to the configuration of the daemon
- **Reconnect** connects to the daemon again (e.g. after a restart of the service)

**Reset to Defaults** resets the settings of the application, the configuration of the daemon is not changed." },

        { title: "Information", icon: "gpu", text:
"## Information

The machine at a glance: model, machine type, processor, graphics, BIOS, board and chassis.

- Every value can be selected by the mouse, the button on the right of a row copies it
- **Copy All** copies all the information as text – please attach it to bug reports" },

        { title: "How It Works", icon: "frequency", text:
"## How It Works

The toolkit has four parts:

1. **lenovo_legion** – kernel driver of the notebook (WMI, embedded controller). It provides the standard Linux \
interfaces (sysfs, hwmon, firmware attributes) for the power profiles, power limits and fans.
2. **legion_intel_msr** – kernel driver of the Intel CPU voltage offsets (OC mailbox). It does not depend on the \
notebook.
3. **LenovoLegion-Daemon** – service running as root (`lenovo-legion-daemon.service`). It reads and writes the \
hardware through its modules (drivers, NVIDIA NVML, the RGB keyboard over USB HID) and saves the settings and \
profiles.
4. **LenovoLegion** – this application. It talks only to the daemon over the socket \
`/run/lenovo-legion/lenovo_legion_daemon.socket`, members of the group `legion` may use it.

The GUI never touches the hardware directly: every value is read from and written by the daemon." },

        { title: "Troubleshooting", icon: "fan", text:
"## Troubleshooting

**The GUI does not connect to the daemon**

- Check the service: `systemctl status lenovo-legion-daemon.service`
- Your user must be in the group `legion`: `groups` – after adding the group log in again
- Settings → **Reconnect**

**Tabs are grayed: Requires the Lenovo Legion kernel driver**

- Check `sudo dmesg | grep lenovo-legion`: the driver prints `Legion platform driver was loaded`
- With DKMS the module is rebuilt for a new kernel automatically; otherwise rebuild it after a kernel update
- Secure Boot needs a signed module (MOK)

**CPU voltage offsets are not applied**

- Many BIOS versions lock undervolting (the offsets read back as 0 mV). The page reports when a value was not applied.

**NVIDIA values are missing**

- NVML needs the proprietary NVIDIA driver. A sleeping GPU (runtime suspend) is not woken up, the last values are shown.

**Bug reports**

Attach **Information → Copy All** and the output of `journalctl -u lenovo-legion-daemon.service`." },

        { title: "Tips & Safety", icon: "limits", text:
"## Tips & Safety

- **Change one thing at a time** and test it under load before the next change.
- **Voltage offsets**: start small (e.g. −25 mV), test the stability (e.g. stress-ng, a game), go down in small steps. \
An unstable undervolt can freeze the notebook; the offsets are not kept after a reboot unless the daemon applies \
the saved settings.
- **Fan curve**: quiet curves raise the temperatures and the CPU and GPU can lower their clocks; the tool tip of a \
point shows the default curves of the power profile for comparison.
- **Power limits**: lower limits mean less heat and noise with a small loss of performance; *Quiet* or *Balanced* \
with lower limits extends the battery life.
- **GPU clock offsets**: use the presets first, artifacts or crashes mean too high offsets.
- Save a working configuration as a **profile** before experimenting." },

        { title: "About", icon: "device", text:
"## About

**Lenovo Legion Linux Toolkit** – unofficial control application for Lenovo Legion notebooks, tested on Legion Pro 7 \
Gen 9 and Gen 10. This project is not affiliated with Lenovo in any way.

License: GPL-2.0

Source code, issues and releases: [github.com/bolekjar/lenovo-legion-linux-toolkit](" + projectUrl + ")" }
    ]

    property int currentTopic: 0

    // Markdown of the topics -> HTML with the spacing and colors of the theme (the Markdown of Qt has no spacing
    // between the blocks): ## / ### headings, "- " and "1. " lists, paragraphs, **bold**, *italic*, `command`, [text](url)
    function inline(text) {
        const code = "font-family: monospace; color: " + theme.accentHover + ";"
        return text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
                   .replace(/`([^`]+)`/g, "<span style=\"" + code + "\">$1</span>")
                   .replace(/\*\*([^*]+)\*\*/g, "<b>$1</b>")
                   .replace(/\*([^*]+)\*/g, "<i>$1</i>")
                   .replace(/\[([^\]]+)\]\(([^)]+)\)/g, "<a href=\"$2\" style=\"color: " + theme.accentHover + ";\">$1</a>")
    }

    function toHtml(markdown) {
        const blocks = markdown.split(/\n\s*\n/)
        let html = ""

        for (const block of blocks) {
            const lines = block.split("\n").filter(line => line.trim().length > 0)
            if (lines.length === 0)
                continue

            if (lines[0].startsWith("## ")) {
                html += "<p style=\"margin-top: 0; margin-bottom: 14px; font-size: " + (theme.fontPixelSize + 8) + "px;\"><b>"
                      + inline(lines[0].substring(3)) + "</b></p>"
                lines.shift()
            } else if (lines[0].startsWith("### ")) {
                html += "<p style=\"margin-top: 10px; margin-bottom: 6px; font-size: " + (theme.fontPixelSize + 2) + "px; color: "
                      + theme.accentHover + ";\"><b>" + inline(lines[0].substring(4)) + "</b></p>"
                lines.shift()
            }

            if (lines.length === 0)
                continue

            const bullet = /^- /, numbered = /^\d+\. /
            if (bullet.test(lines[0]) || numbered.test(lines[0])) {
                const tag = bullet.test(lines[0]) ? "ul" : "ol"
                html += "<" + tag + " style=\"margin-top: 0; margin-bottom: 12px; -qt-list-indent: 1;\">"
                for (const line of lines)
                    html += "<li style=\"margin-bottom: 6px;\">" + inline(line.replace(bullet, "").replace(numbered, "")) + "</li>"
                html += "</" + tag + ">"
            } else {
                const bold = lines.length === 1 && /^\*\*[^*]+\*\*$/.test(lines[0])
                html += "<p style=\"margin-top: " + (bold ? 8 : 0) + "px; margin-bottom: " + (bold ? 6 : 12) + "px; line-height: 130%;\">"
                      + inline(lines.join(" ")) + "</p>"
            }
        }

        return html
    }

    // Title of the page with the system check
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
            spacing: 24

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 4

                Label {
                    text: "Help"
                    font.pixelSize: theme.fontPixelSize + 12
                    font.bold: true
                    color: theme.text
                }

                Label {
                    Layout.fillWidth: true
                    text: "Lenovo Legion Linux Toolkit – how the pages work, what to check when something is missing."
                    color: theme.disabledText
                    wrapMode: Text.Wrap
                }
            }

            // System check: the parts of the toolkit and their state, the hint how to fix a missing part
            Rectangle {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: Math.min(520, hero.width * 0.55)
                implicitHeight: checkColumn.implicitHeight + 20
                radius: theme.radius
                color: theme.window
                border.width: 1
                border.color: theme.groupBorder

                ColumnLayout {
                    id: checkColumn

                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 2

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.bottomMargin: 4

                        Label {
                            Layout.fillWidth: true
                            text: "System Check"
                            font.bold: true
                            color: theme.text
                        }

                        ThemedButton {
                            text: "Check Again"
                            onClicked: {
                                daemonCheck.reload()
                                lenovoCheck.reload()
                                msrCheck.reload()
                                nvidiaCheck.reload()
                                rgbCheck.reload()
                                modulesCheck.reload()
                            }
                        }
                    }

                    Repeater {
                        model: page.checks

                        delegate: Rectangle {
                            id: checkRow

                            required property var modelData

                            Layout.fillWidth: true
                            implicitHeight: checkColumnLayout.implicitHeight + 8
                            radius: theme.inputRadius
                            color: checkHover.hovered ? theme.itemHoverBackground : "transparent"

                            // Missing required part: red, missing optional part: gray
                            readonly property color stateColor: modelData.ok ? "#3fb950" : modelData.required ? "#e5534b" : theme.disabledText

                            // Row with details (Daemon modules): opened / closed by a click
                            readonly property bool hasDetails: modelData.details !== undefined && modelData.details.length > 0
                            property bool expanded: false

                            HoverHandler {
                                id: checkHover

                                cursorShape: checkRow.hasDetails ? Qt.PointingHandCursor : Qt.ArrowCursor
                            }

                            TapHandler {
                                enabled: checkRow.hasDetails
                                onTapped: checkRow.expanded = !checkRow.expanded
                            }

                            ColumnLayout {
                                id: checkColumnLayout

                                anchors.fill: parent
                                anchors.leftMargin: 6
                                anchors.rightMargin: 6
                                spacing: 4

                                RowLayout {
                                    id: checkLayout

                                    Layout.fillWidth: true
                                    spacing: 10

                                    // State: check mark / cross in a circle
                                    Rectangle {
                                        Layout.alignment: Qt.AlignTop
                                        Layout.topMargin: 2
                                        implicitWidth: 18
                                        implicitHeight: 18
                                        radius: 9
                                        color: Qt.rgba(checkRow.stateColor.r, checkRow.stateColor.g, checkRow.stateColor.b, 0.18)
                                        border.width: 1.5
                                        border.color: checkRow.stateColor

                                        Text {
                                            anchors.centerIn: parent
                                            text: checkRow.modelData.ok ? "✓" : "✕"
                                            font.pixelSize: 11
                                            font.bold: true
                                            color: checkRow.stateColor
                                        }
                                    }

                                    Label {
                                        Layout.alignment: Qt.AlignTop
                                        Layout.preferredWidth: 150
                                        text: checkRow.modelData.name
                                        font.bold: true
                                        color: checkHover.hovered ? theme.itemHoverText : theme.text
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        Layout.alignment: Qt.AlignTop
                                        text: checkRow.modelData.ok ? checkRow.modelData.okText : checkRow.modelData.failText
                                        color: checkRow.modelData.ok ? (checkHover.hovered ? theme.itemHoverText : theme.disabledText)
                                                                     : (checkHover.hovered ? theme.itemHoverText : theme.text)
                                        wrapMode: Text.Wrap
                                    }

                                    // Open / closed details
                                    Label {
                                        Layout.alignment: Qt.AlignTop
                                        visible: checkRow.hasDetails
                                        text: checkRow.expanded ? "▾" : "▸"
                                        color: checkHover.hovered ? theme.itemHoverText : theme.disabledText
                                    }
                                }

                                // Details: name, state mark, version / reason
                                Repeater {
                                    model: checkRow.expanded ? checkRow.modelData.details : []

                                    delegate: RowLayout {
                                        id: detailRow

                                        required property var modelData

                                        Layout.fillWidth: true
                                        Layout.leftMargin: 28
                                        spacing: 10

                                        readonly property color detailColor: modelData.state === 0 ? "#3fb950" : "#e5534b"

                                        Text {
                                            Layout.alignment: Qt.AlignTop
                                            text: detailRow.modelData.state === 0 ? "✓" : "✕"
                                            font.pixelSize: 11
                                            font.bold: true
                                            color: detailRow.detailColor
                                        }

                                        Label {
                                            Layout.alignment: Qt.AlignTop
                                            Layout.preferredWidth: 132
                                            text: detailRow.modelData.name
                                            elide: Text.ElideRight
                                            color: checkHover.hovered ? theme.itemHoverText : theme.text
                                        }

                                        Label {
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignTop
                                            text: detailRow.modelData.text
                                            color: detailRow.modelData.state === 0 ? (checkHover.hovered ? theme.itemHoverText : theme.disabledText)
                                                                                   : (checkHover.hovered ? theme.itemHoverText : theme.text)
                                            wrapMode: Text.Wrap
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Topics: list on the left, the selected topic on the right
    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Rectangle {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 210
            implicitHeight: topicList.implicitHeight + 12
            radius: theme.radius
            color: theme.groupBackground
            border.width: theme.styled ? 2 : 1
            border.color: theme.groupBorder

            ColumnLayout {
                id: topicList

                anchors.fill: parent
                anchors.margins: 6
                spacing: 2

                Repeater {
                    model: page.topics

                    delegate: AbstractButton {
                        id: topicButton

                        required property var modelData
                        required property int index

                        readonly property bool current: page.currentTopic === index

                        Layout.fillWidth: true
                        implicitHeight: 34
                        hoverEnabled: true
                        onClicked: page.currentTopic = index

                        background: Rectangle {
                            radius: theme.inputRadius
                            color: topicButton.current ? theme.accent
                                 : topicButton.hovered ? theme.itemHoverBackground : "transparent"

                            Rectangle {
                                anchors.fill: parent
                                visible: topicButton.current && theme.styled
                                radius: parent.radius
                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: theme.accent }
                                    GradientStop { position: 1.0; color: theme.accentHover }
                                }
                            }
                        }

                        contentItem: Row {
                            leftPadding: 10
                            spacing: 10

                            TintedIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 16
                                height: 16
                                source: "qrc:/ui/icons/" + topicButton.modelData.icon + ".svg"
                                color: topicButton.current ? theme.accentText
                                     : topicButton.hovered ? theme.itemHoverText : theme.text
                            }

                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: topicButton.modelData.title
                                font.bold: topicButton.current
                                color: topicButton.current ? theme.accentText
                                     : topicButton.hovered ? theme.itemHoverText : theme.text
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            implicitHeight: topicText.implicitHeight + 40
            radius: theme.radius
            color: theme.groupBackground
            border.width: theme.styled ? 2 : 1
            border.color: theme.groupBorder

            TextEdit {
                id: topicText

                x: 24
                y: 16
                width: parent.width - 48
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.RichText
                text: page.toHtml(page.topics[page.currentTopic].text)
                font.pixelSize: theme.fontPixelSize
                color: theme.text
                selectionColor: theme.accent
                selectedTextColor: theme.accentText

                onLinkActivated: link => Qt.openUrlExternally(link)

                HoverHandler {
                    cursorShape: topicText.hoveredLink.length > 0 ? Qt.PointingHandCursor : Qt.IBeamCursor
                }
            }
        }
    }
}
