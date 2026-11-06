// SPDX-License-Identifier: GPL-2.0
// CPU threads the same as CPUControl of the widget pages:
//  - grid of the threads (8 rows), P-threads, E-threads (CPU topology) and disabled threads (offline in the data),
//  - "Show threads" filters the grid, the governor and Enable/Disable are applied to the shown threads,
//  - per thread: Enabled (CPU0 can not be switched off), governor of the thread,
//  - "Disabled SMP": SMT (written immediately, "on" switches all sibling threads on, see the CPUSMT daemon module),
//  - Apply sends the changed threads only (elements of cpus by index, the daemon skips empty ones).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0
import "CellLayout.js" as CellLayout

Section {
    id: grid

    property DataSource topology    // CPU_TOPOLOGY
    property DataSource smt         // CPU_SMT

    title: "CPU Control"
    applyBar: true

    source: DataSource {
        id: options

        dataType: "CPU_OPTIONS"
    }

    // Colors of the widget page (ThreadControl, not part of the themes)
    readonly property var  kindBackground: ({ "P": "#3D2B1F", "E": "#393939", "D": "transparent" })
    readonly property var  kindName:       ({ "P": "P-Thread", "E": "E-Thread", "D": "D-Thread" })

    readonly property int  cpuCount:    (options.count("cpus", options.revision))
    readonly property var  performance: (ranges("active_cpus_core"))
    readonly property var  efficient:   (ranges("active_cpus_atom"))
    readonly property bool hybrid:      performance.length > 0 && efficient.length > 0

    readonly property var  filters: hybrid ? ["ALL", "Enabled", "Disabled", "Performance", "Efficiency"] : ["ALL", "Enabled", "Disabled"]
    property string        filter:  "ALL"

    readonly property var  kinds:       (threadKinds())
    readonly property var  titleColors: (threadTitleColors())

    function ranges(field) {
        const cpus = []

        for (let range = 0; range < topology.count(field, topology.revision); ++range) {
            const min = topology.value(field + "[" + range + "].cpu_min", topology.revision)
            const max = topology.value(field + "[" + range + "].cpu_max", topology.revision)

            for (let cpu = min; cpu <= max; ++cpu)
                cpus.push(cpu)
        }

        return cpus
    }

    /*
     * Kind of the threads by the data from the daemon: D = offline, P/E = CPU topology
     */
    function threadKinds() {
        const result = []

        for (let cpu = 0; cpu < cpuCount; ++cpu) {
            if (!options.daemonValue("cpus[" + cpu + "].cpu_online", options.revision))
                result.push("D")
            else if (efficient.includes(cpu))
                result.push("E")
            else
                result.push("P")
        }

        return result
    }

    /*
     * Title color changes with the core (white/black), threads in the order P, E, D (the same as CPUControl)
     */
    function threadTitleColors() {
        const colors = []
        const order = []
        let color = 0
        let core = 0

        for (const kind of ["P", "E", "D"])
            for (let cpu = 0; cpu < cpuCount; ++cpu)
                if (kinds[cpu] === kind)
                    order.push(cpu)

        for (const cpu of order) {
            const coreId = options.daemonValue("cpus[" + cpu + "].cpu_core_id", options.revision)

            if (coreId !== core) {
                color = (color + 1) % 2
                core = coreId
            }

            colors[cpu] = kinds[cpu] === "D" ? "#333066" : (color === 0 ? "white" : "black")
        }

        return colors
    }

    function shownKinds() {
        switch (filter) {
        case "Enabled":     return ["P", "E"]
        case "Disabled":    return ["D"]
        case "Performance": return ["P"]
        case "Efficiency":  return ["E"]
        default:            return ["P", "E", "D"]
        }
    }

    function shown(cpu) {
        return shownKinds().includes(kinds[cpu])
    }

    /*
     * After new data (Apply of Disable/Enable, daemon notification) the filter can show nothing: "Efficiency"
     * after the E-threads were disabled (the topology has no active E-threads, the filter is not offered),
     * "Disabled" after all threads were enabled. Back to ALL the same as the widget page (refresh starts with ALL).
     */
    function checkFilter() {
        if (filter === "ALL")
            return

        if (!filters.includes(filter)) {
            filter = "ALL"
            return
        }

        for (let cpu = 0; cpu < cpuCount; ++cpu)
            if (shown(cpu))
                return

        filter = "ALL"
    }

    onKindsChanged: checkFilter()
    onFiltersChanged: checkFilter()

    function setOnline(enabled) {
        options.beginChanges()   // one notification for all threads
        for (let cpu = 1; cpu < cpuCount; ++cpu)
            if (shown(cpu))
                options.field("cpus[" + cpu + "].cpu_online", options.revision).value = enabled
        options.endChanges()
    }

    // Governor of the shown online threads (not of the disabled threads)
    function setGovernor(governor) {
        options.beginChanges()
        for (let cpu = 0; cpu < cpuCount; ++cpu)
            if (shown(cpu) && kinds[cpu] !== "D")
                options.field("cpus[" + cpu + "].governor", options.revision).value = governor
        options.endChanges()
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        ThemedCheckBox {
            text: "Disabled SMP"
            visible: grid.smt !== null && (grid.smt.count("settable_controls", grid.smt.revision)) > 0
            checked: grid.smt !== null && (grid.smt.value("active", grid.smt.revision)) === false
            // "on"/"off" always written (applyValues): "on" switches the sibling threads on also when the control is already "on"
            onToggled: grid.smt.applyValues({ "control": checked ? "off" : "on" })
        }

        Item {
            Layout.fillWidth: true
        }

        Label {
            text: "Show threads:"
        }

        ThemedComboBox {
            implicitWidth: 150
            model: grid.filters
            currentIndex: grid.filters.indexOf(grid.filter)
            onActivated: index => grid.filter = grid.filters[index]
        }

        Label {
            text: "Governor:"
        }

        ThemedComboBox {
            implicitWidth: 150
            model: (options.repeatedValues("cpus[0].available_governors", options.revision))
            currentIndex: count > 0 ? find((options.daemonValue("cpus[0].governor", options.revision))) : -1
            onActivated: index => grid.setGovernor(textAt(index))
        }

        ThemedButton {
            text: "Enable"
            onClicked: grid.setOnline(true)
        }

        ThemedButton {
            text: "Disable"
            onClicked: grid.setOnline(false)
        }
    }

    // Shown threads in the order of the CPU numbers
    readonly property var shownThreads: {
        const result = []

        for (let cpu = 0; cpu < cpuCount; ++cpu)
            if (shown(cpu))
                result.push(cpu)

        return result
    }

    // Width of a thread cell (the whole title "P-Thread: 15, CoreId: 11, DieId: 0" fits)
    readonly property int cellWidth: 290

    // Shown cells column by column, the kinds of the threads in their own columns, columns by the width (up to 4)
    GridLayout {
        id: cells

        readonly property int cellColumns: Math.max(1, Math.min(4, Math.floor((width + columnSpacing) / (grid.cellWidth + columnSpacing))))

        // P, E and disabled threads strictly in their own columns
        readonly property var placement: CellLayout.place(["P", "E", "D"].map(kind => grid.shownThreads.filter(cpu => grid.kinds[cpu] === kind)), cellColumns)

        Layout.fillWidth: true

        rowSpacing: 6
        columnSpacing: 6

        Repeater {
            model: grid.cpuCount

            // Cell of a thread: group with the title tab like ThreadControl (QGroupBox, title color by the core,
            // background by the kind of the thread)
            delegate: Section {
                id: thread

                required property int index

                readonly property FieldModel onlineField:   (options.field("cpus[" + index + "].cpu_online", options.revision))
                readonly property FieldModel governorField: (options.field("cpus[" + index + "].governor", options.revision))

                // Fixed dark background of P/E-threads: light text in all themes, disabled threads use the theme
                readonly property color textColor: grid.kinds[index] === "D" ? theme.text : "#e0e0e0"

                readonly property var position: cells.placement.positions[index] !== undefined ? cells.placement.positions[index] : [0, 0]

                Layout.row: position[0]
                Layout.column: position[1]
                Layout.fillWidth: true
                Layout.preferredWidth: grid.cellWidth

                visible: grid.shown(index)

                title: grid.kindName[grid.kinds[index]] + ": " + index
                       + ", CoreId: " + (options.daemonValue("cpus[" + index + "].cpu_core_id", options.revision))
                       + ", DieId: " + options.daemonValue("cpus[" + index + "].die_id", options.revision)
                titleColor: grid.titleColors[index]
                backgroundColor: grid.kindBackground[grid.kinds[index]]

                // Enabled and the governor in one row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    // Marks of changed values in a space reserved always (the row does not move when a value is changed)
                    Label {
                        text: "●"
                        color: theme.modified
                        opacity: thread.onlineField !== null && thread.onlineField.modified ? 1 : 0
                    }

                    ThemedCheckBox {
                        Layout.leftMargin: -10
                        text: "Enabled"
                        checked: thread.onlineField !== null && thread.onlineField.value === true
                        enabled: thread.index !== 0
                        onToggled: thread.onlineField.value = checked
                        textColor: thread.onlineField !== null && thread.onlineField.modified ? theme.modified
                                 : enabled ? thread.textColor : theme.disabledText
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        visible: thread.governorField !== null && thread.governorField.visible
                        text: "●"
                        color: theme.modified
                        opacity: thread.governorField !== null && thread.governorField.modified ? 1 : 0
                    }

                    Label {
                        Layout.leftMargin: -8
                        visible: thread.governorField !== null && thread.governorField.visible
                        text: "Governor: " + (thread.governorField !== null ? thread.governorField.value : "")
                        color: thread.governorField !== null && thread.governorField.modified ? theme.modified : thread.textColor
                    }
                }
            }
        }
    }
}
