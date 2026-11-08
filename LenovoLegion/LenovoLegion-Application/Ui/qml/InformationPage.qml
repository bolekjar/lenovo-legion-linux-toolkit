// SPDX-License-Identifier: GPL-2.0
// Page of the generic GUI (toolbar page "Information"): the machine at a glance (model, processor, graphics, BIOS)
// and groups of the details (InfoSection). Values can be selected and copied, "Copy All" copies a text report
// (e.g. for a bug report). Read only, data of MACHINE_INFORMATION, CPU_INFO, CPU_TOPOLOGY and NVIDIA_NWML.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

SettingsPage {
    id: page

    title: ""
    changeBar: false
    columnWidth: 400

    DataSource {
        id: machine

        dataType: "MACHINE_INFORMATION"
    }

    DataSource {
        id: cpu

        dataType: "CPU_INFO"
    }

    DataSource {
        id: topology

        dataType: "CPU_TOPOLOGY"
    }

    DataSource {
        id: gpu

        dataType: "NVIDIA_NWML"
    }

    // Values of the sources (empty string when missing)
    function text(source, path) {
        const value = source.available ? source.value(path, source.revision) : undefined
        return value !== undefined && value !== null ? String(value).trim() : ""
    }

    function number(source, path) {
        const value = source.available ? source.value(path, source.revision) : undefined
        return value !== undefined && value !== null ? Number(value) : 0
    }

    // "LENOVO" -> "Lenovo" (DMI strings in capitals), other strings unchanged
    function vendorName(value) {
        return value.length > 1 && value === value.toUpperCase() ? value.charAt(0) + value.slice(1).toLowerCase() : value
    }

    // "Intel(R) Core(TM) Ultra 9 275HX" -> "Intel Core Ultra 9 275HX"
    function cpuName(value) {
        return value.replace(/\((R|TM|C)\)/gi, "").replace(/\s+/g, " ").trim()
    }

    // "NVIDIA GeForce RTX 5090 Laptop GPU" -> "GeForce RTX 5090 Laptop GPU"
    function gpuName(value) {
        return value.replace(/^NVIDIA\s+/i, "")
    }

    // SMBIOS chassis type (DMI chassis_type is a number)
    readonly property var chassisTypes: ({
        "1": "Other", "2": "Unknown", "3": "Desktop", "4": "Low Profile Desktop", "5": "Pizza Box", "6": "Mini Tower",
        "7": "Tower", "8": "Portable", "9": "Laptop", "10": "Notebook", "11": "Hand Held", "12": "Docking Station",
        "13": "All in One", "14": "Sub Notebook", "15": "Space-saving", "16": "Lunch Box", "17": "Main Server Chassis",
        "18": "Expansion Chassis", "19": "SubChassis", "20": "Bus Expansion Chassis", "21": "Peripheral Chassis",
        "22": "RAID Chassis", "23": "Rack Mount Chassis", "24": "Sealed-case PC", "25": "Multi-system Chassis",
        "26": "Compact PCI", "27": "Advanced TCA", "28": "Blade", "29": "Blade Enclosure", "30": "Tablet",
        "31": "Convertible", "32": "Detachable", "33": "IoT Gateway", "34": "Embedded PC", "35": "Mini PC", "36": "Stick PC"
    })

    function chassisName(value) {
        return chassisTypes[value] !== undefined ? chassisTypes[value] : value
    }

    // BIOS date of DMI (MM/DD/YYYY) in a readable form
    function biosDate(value) {
        const parts = value.split("/")
        if (parts.length !== 3)
            return value

        const date = new Date(Number(parts[2]), Number(parts[0]) - 1, Number(parts[1]))
        return isNaN(date.getTime()) ? value : Qt.formatDate(date, "d MMMM yyyy")
    }

    // Cache size of sysfs ("48K", "36864K") -> "48 KB", "36 MB"
    function cacheSize(value) {
        const match = /^(\d+)\s*([KMG]?)/i.exec(value)
        if (match === null)
            return value

        let kilobytes = Number(match[1]) * ({ "": 1 / 1024, "K": 1, "M": 1024, "G": 1024 * 1024 })[match[2].toUpperCase()]
        return kilobytes >= 1024 && kilobytes % 1024 === 0 ? (kilobytes / 1024) + " MB" : kilobytes + " KB"
    }

    function cacheName(level, type) {
        return "L" + level + (type === "Data" ? "d" : type === "Instruction" ? "i" : "")
    }

    // Number of CPUs in the ranges of the topology (min..max)
    function cpuCount(path) {
        let count = 0

        for (let i = 0; i < topology.count(path, topology.revision); ++i)
            count += number(topology, path + "[" + i + "].cpu_max") - number(topology, path + "[" + i + "].cpu_min") + 1

        return count
    }

    function bytes(value) {
        return value <= 0 ? "" : Math.round(value / (1024 * 1024 * 1024)) + " GB"
    }

    function watts(milliwatts) {
        return milliwatts > 0 ? Math.round(milliwatts / 1000) + " W" : ""
    }

    // Model of the machine: the product version of Lenovo (the name), the product name is the machine type
    readonly property string modelName: text(machine, "product_version").length > 0 ? text(machine, "product_version")
                                                                                    : text(machine, "product_name")

    readonly property var systemRows: [
        { label: "Manufacturer", value: vendorName(text(machine, "sys_vendor")) },
        { label: "Model", value: text(machine, "product_version") },
        { label: "Machine type", value: text(machine, "product_name") },
        { label: "Product family", value: text(machine, "product_family") },
        { label: "SKU", value: text(machine, "product_sku") }
    ]

    readonly property var biosRows: [
        { label: "Vendor", value: vendorName(text(machine, "bios_vendor")) },
        { label: "Version", value: text(machine, "bios_version") },
        { label: "Release", value: text(machine, "bios_release") },
        { label: "Date", value: biosDate(text(machine, "bios_date")), hint: text(machine, "bios_date") }
    ]

    readonly property var boardRows: [
        { label: "Board", value: text(machine, "board_name") },
        { label: "Board vendor", value: vendorName(text(machine, "board_vendor")) },
        { label: "Board version", value: text(machine, "board_version") },
        { label: "Chassis type", value: chassisName(text(machine, "chassis_type")), hint: "SMBIOS chassis type " + text(machine, "chassis_type") },
        { label: "Chassis vendor", value: vendorName(text(machine, "chassis_vendor")) },
        { label: "Chassis version", value: text(machine, "chassis_version") }
    ]

    readonly property int performanceCpus: cpuCount("active_cpus_core")
    readonly property int efficientCpus:   cpuCount("active_cpus_atom")
    readonly property int activeCpus:      cpuCount("active_cpus")

    readonly property var caches: {
        const result = []

        for (let i = 0; i < cpu.count("caches", cpu.revision); ++i)
            result.push({ name: cacheName(number(cpu, "caches[" + i + "].level"), text(cpu, "caches[" + i + "].type")),
                          size: cacheSize(text(cpu, "caches[" + i + "].size")),
                          type: text(cpu, "caches[" + i + "].type") })

        return result
    }

    readonly property var processorRows: [
        { label: "Model", value: cpuName(text(cpu, "cpu_model")), hint: text(cpu, "cpu_model") },
        { label: "Generation", value: text(cpu, "cpu_generation") },
        { label: "Architecture", value: text(cpu, "architecture") },
        { label: "Logical CPUs", value: activeCpus <= 0 ? ""
                : activeCpus + (performanceCpus > 0 && efficientCpus > 0 ? " (" + performanceCpus + " performance, " + efficientCpus + " efficient)" : "") },
        { label: "Family / model", value: number(cpu, "family") > 0 ? number(cpu, "family") + " / " + number(cpu, "model")
                                                                     + " (0x" + number(cpu, "model").toString(16).toUpperCase() + ")" : "" },
        { label: "Cache", value: caches.map(cache => cache.name + " " + cache.size).join(", "), reportOnly: true }
    ]

    readonly property bool gpuSuspended: gpu.available && gpu.value("gpu_suspended", gpu.revision) === true

    readonly property var graphicsRows: {
        const power = "hardware_monitor.power."
        const pcie = "hardware_monitor.pcie."
        const generation = number(gpu, pcie + "generation"), width = number(gpu, pcie + "width")
        const generationMax = number(gpu, pcie + "generation_max"), widthMax = number(gpu, pcie + "width_max")

        return [
            { label: "Name", value: gpuName(text(gpu, "name")), hint: text(gpu, "name") },
            { label: "Video memory", value: bytes(number(gpu, "hardware_monitor.memory_use.total")) },
            { label: "Default power limit", value: watts(number(gpu, power + "default_value")) },
            { label: "Power limit range", value: number(gpu, power + "max_value") > 0
                    ? watts(number(gpu, power + "min_value")) + " – " + watts(number(gpu, power + "max_value")) : "" },
            { label: "Enforced power limit", value: watts(number(gpu, power + "enforced_value")) },
            { label: "PCIe link", value: generation > 0 ? "Gen " + generation + " ×" + width
                    + (generationMax !== generation || widthMax !== width ? " (max Gen " + generationMax + " ×" + widthMax + ")" : "") : "" },
            { label: "Max GPU clock", value: number(gpu, "hardware_monitor.gpu_clock.max_value") > 0 ? number(gpu, "hardware_monitor.gpu_clock.max_value") + " MHz" : "" },
            { label: "Max memory clock", value: number(gpu, "hardware_monitor.memory_clock.max_value") > 0 ? number(gpu, "hardware_monitor.memory_clock.max_value") + " MHz" : "" }
        ]
    }

    // Text report of all groups (Copy All)
    function report() {
        const groups = [
            { title: "System", rows: systemRows }, { title: "Processor", rows: processorRows },
            { title: "Graphics", rows: graphicsRows }, { title: "BIOS", rows: biosRows },
            { title: "Board & Chassis", rows: boardRows }
        ]
        let lines = [vendorName(text(machine, "sys_vendor")) + " " + modelName]

        for (const group of groups) {
            const rows = group.rows.filter(row => String(row.value).length > 0)
            if (rows.length === 0)
                continue

            lines.push("", group.title)
            for (const row of rows)
                lines.push("  " + row.label + ": " + row.value)
        }

        return lines.join("\n").trim()
    }

    // Clipboard: a hidden text edit (QML has no clipboard API)
    TextEdit {
        id: clipboard

        visible: false
    }

    function copy(value, message) {
        clipboard.text = value
        clipboard.selectAll()
        clipboard.copy()
        clipboard.text = ""
        notifications.notify("Copied", message, "info")
    }

    // The machine at a glance: model, maker and type, the main parts as chips, Copy All
    Rectangle {
        id: hero

        Layout.fillWidth: true
        Layout.columnSpan: page.columns
        implicitHeight: heroLayout.implicitHeight + 40

        radius: theme.radius
        border.width: theme.styled ? 2 : 1
        border.color: theme.groupBorder
        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.groupBackground }
            GradientStop { position: 1.0; color: theme.styled ? theme.window : theme.groupBackground }
        }
        clip: true

        // Accent light from the left (like the light bars of the notebook)
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
            anchors.margins: 20
            anchors.leftMargin: 28
            spacing: 20

            LegionLogo {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 64
                Layout.preferredHeight: 64
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    Layout.fillWidth: true
                    text: page.modelName.length > 0 ? page.modelName : "Unknown machine"
                    font.pixelSize: theme.fontPixelSize + 12
                    font.bold: true
                    color: theme.text
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: [page.vendorName(page.text(machine, "sys_vendor")),
                           page.text(machine, "product_name").length > 0 ? "Machine type " + page.text(machine, "product_name") : "",
                           page.chassisName(page.text(machine, "chassis_type"))].filter(part => part.length > 0).join("  ·  ")
                    color: theme.disabledText
                    elide: Text.ElideRight
                }

                // Main parts
                Flow {
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    spacing: 8

                    Repeater {
                        model: [
                            { icon: "cpu",    text: page.cpuName(page.text(cpu, "cpu_model")) },
                            { icon: "gpu",    text: page.gpuName(page.text(gpu, "name")) },
                            { icon: "limits", text: page.bytes(page.number(gpu, "hardware_monitor.memory_use.total")).length > 0
                                                    ? page.bytes(page.number(gpu, "hardware_monitor.memory_use.total")) + " VRAM" : "" },
                            { icon: "device", text: page.text(machine, "bios_version").length > 0 ? "BIOS " + page.text(machine, "bios_version") : "" }
                        ].filter(chip => chip.text.length > 0)

                        delegate: Rectangle {
                            id: chip

                            required property var modelData

                            implicitWidth: chipRow.implicitWidth + 20
                            implicitHeight: chipRow.implicitHeight + 10
                            radius: height / 2
                            color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.14)
                            border.width: 1
                            border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.55)

                            Row {
                                id: chipRow

                                anchors.centerIn: parent
                                spacing: 6

                                TintedIcon {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 16
                                    height: 16
                                    source: "qrc:/ui/icons/" + chip.modelData.icon + ".svg"
                                    color: theme.text
                                }

                                Label {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: chip.modelData.text
                                    font.bold: true
                                    color: theme.text
                                }
                            }
                        }
                    }
                }
            }

            ThemedButton {
                id: copyAll

                Layout.alignment: Qt.AlignTop
                text: "Copy All"
                enabled: page.modelName.length > 0 || page.text(cpu, "cpu_model").length > 0
                onClicked: page.copy(page.report(), "All information copied to the clipboard")

                ThemedToolTip {
                    visible: copyAll.hovered
                    delay: 500
                    text: "Copy all information as text (e.g. for a bug report)"
                }
            }
        }
    }

    InfoSection {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        title: "Processor"
        source: cpu
        rows: page.processorRows
        emptyText: "Processor information is not available"
        onCopyRequested: (label, value) => page.copy(value, label + " copied to the clipboard")

        // Caches as chips
        Flow {
            Layout.fillWidth: true
            Layout.topMargin: 4
            Layout.leftMargin: 6
            visible: page.caches.length > 0
            spacing: 6

            Repeater {
                model: page.caches

                delegate: Rectangle {
                    id: cacheChip

                    required property var modelData

                    implicitWidth: cacheRow.implicitWidth + 16
                    implicitHeight: cacheRow.implicitHeight + 8
                    radius: theme.inputRadius
                    color: theme.window
                    border.width: 1
                    border.color: theme.groupBorder

                    Row {
                        id: cacheRow

                        anchors.centerIn: parent
                        spacing: 6

                        Label {
                            text: cacheChip.modelData.name
                            color: theme.accentHover
                            font.bold: true
                        }

                        Label {
                            text: cacheChip.modelData.size
                            color: theme.text
                        }
                    }

                    HoverHandler {
                        id: cacheHover
                    }

                    ThemedToolTip {
                        visible: cacheHover.hovered
                        delay: 500
                        text: cacheChip.modelData.name.substring(0, 2) + " " + cacheChip.modelData.type.toLowerCase() + " cache, " + cacheChip.modelData.size
                    }
                }
            }
        }
    }

    InfoSection {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        title: "Graphics"
        source: gpu
        rows: page.graphicsRows
        emptyText: "NVIDIA GPU information is not available"
        onCopyRequested: (label, value) => page.copy(value, label + " copied to the clipboard")

        // Runtime suspended GPU: NVML is not asked (the GPU would wake up), the values are from the last query
        Label {
            Layout.fillWidth: true
            Layout.leftMargin: 6
            visible: page.gpuSuspended
            text: "The GPU is sleeping (runtime suspend), the values are from the last query."
            color: theme.disabledText
            font.italic: true
            wrapMode: Text.Wrap
        }
    }

    InfoSection {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        title: "System"
        source: machine
        rows: page.systemRows
        emptyText: "System information is not available"
        onCopyRequested: (label, value) => page.copy(value, label + " copied to the clipboard")
    }

    InfoSection {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        title: "BIOS"
        source: machine
        rows: page.biosRows
        emptyText: "BIOS information is not available"
        onCopyRequested: (label, value) => page.copy(value, label + " copied to the clipboard")
    }

    InfoSection {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        title: "Board & Chassis"
        source: machine
        rows: page.boardRows
        emptyText: "Board information is not available"
        onCopyRequested: (label, value) => page.copy(value, label + " copied to the clipboard")
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
        width: 360
        height: parent.height / 2
    }
}
