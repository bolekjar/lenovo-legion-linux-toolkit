// SPDX-License-Identifier: GPL-2.0
// CPU frequency limits of the threads the same as CPUFrequencyControl of the widget pages:
//  - grid of the active threads (8 rows), P-threads (active_cpus_core, all active threads without it) and E-threads,
//  - per thread: Min and Max in MHz in the range of the thread (annotations of CPUFrequency.proto), Min <= Max,
//  - global Min/Max in the range of all threads, applied to the threads of "Show threads" (clamped to their range),
//  - preset buttons Min/X, X/X and Default set the global Min/Max,
//  - Apply sends the changed threads only.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LenovoLegion.Ui 1.0
import "CellLayout.js" as CellLayout

Section {
    id: grid

    property DataSource topology    // CPU_TOPOLOGY

    title: "CPU Frequency Control"
    applyBar: true

    source: DataSource {
        id: frequency

        dataType: "CPU_FREQUENCY"
    }

    // Colors of the widget page (ThreadFrequencyControl, not part of the themes)
    readonly property var  kindBackground: ({ "P": "#3D2B1F", "E": "#393939" })
    readonly property var  kindName:       ({ "P": "P-Thread", "E": "E-Thread" })

    readonly property var  performance: (topology.count("active_cpus_core", topology.revision) > 0 ? ranges("active_cpus_core") : ranges("active_cpus"))
    readonly property var  efficient:   (ranges("active_cpus_atom"))
    readonly property var  threads:     performance.concat(efficient)
    readonly property bool hybrid:      topology.count("active_cpus_core", topology.revision) > 0 && efficient.length > 0

    readonly property var  filters: hybrid ? ["All", "Performance", "Efficiency"] : ["All"]
    property string        filter:  "All"

    // The filter is not offered any more (E-threads disabled, no hybrid topology): back to All
    onFiltersChanged: if (!filters.includes(filter)) filter = "All"

    // Range of the threads of the filter (MHz), the global values are clamped to it
    readonly property int  rangeMin: (threadsRange(targets(), true))
    readonly property int  rangeMax: (threadsRange(targets(), false))

    // Global Min/Max, initial values from the first active thread
    property int globalMin: 0
    property int globalMax: 0

    readonly property var presets: [2500, 3000, 3500, 4000, 4500, 5000]

    // Width of a thread cell (the whole title "P-Thread: 15, CoreId: 11, DieId: 0" fits)
    readonly property int cellWidth: 290

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

    function minField(cpu) { return frequency.field("cpus[" + cpu + "].scaling_min_freq", frequency.revision) }
    function maxField(cpu) { return frequency.field("cpus[" + cpu + "].scaling_max_freq", frequency.revision) }

    function threadsRange(cpus, minimum) {
        let result = minimum ? Number.MAX_VALUE : 0

        for (const cpu of cpus) {
            const field = minField(cpu)

            if (field === null || field.min === undefined)
                continue

            result = minimum ? Math.min(result, field.min) : Math.max(result, field.max)
        }

        return result === Number.MAX_VALUE ? 0 : result
    }

    // Value inside the range of the threads of the filter, computed directly (the bindings of rangeMin/rangeMax
    // may not be updated yet in onFilterChanged)
    function clampToRange(value) {
        const cpus = targets()

        return Math.max(threadsRange(cpus, true), Math.min(threadsRange(cpus, false), value))
    }

    function kind(cpu) {
        return efficient.includes(cpu) ? "E" : "P"
    }

    function targets() {
        switch (filter) {
        case "Performance": return performance
        case "Efficiency":  return efficient
        default:            return threads
        }
    }

    // Min of a thread, clamped to its range, Max raised to Min (ThreadFrequencyControl)
    function setThreadMin(cpu, value) {
        const min = minField(cpu)
        const max = maxField(cpu)
        const level = Math.max(min.min, Math.min(min.max, value))

        frequency.beginChanges()
        min.value = level

        if (max.value < level)
            max.value = level
        frequency.endChanges()
    }

    // Max of a thread, clamped to its range, Min lowered to Max
    function setThreadMax(cpu, value) {
        const min = minField(cpu)
        const max = maxField(cpu)
        const level = Math.max(max.min, Math.min(max.max, value))

        frequency.beginChanges()
        max.value = level

        if (min.value > level)
            min.value = level
        frequency.endChanges()
    }

    // Changes of all threads in one batch: one notification instead of one per field (many threads, every
    // notification re-evaluates the whole grid)
    function setGlobalMin(value) {
        value = clampToRange(value)
        globalMin = value

        frequency.beginChanges()

        if (globalMax < value)
            setGlobalMax(value)

        for (const cpu of targets())
            setThreadMin(cpu, value)

        frequency.endChanges()
    }

    function setGlobalMax(value) {
        value = clampToRange(value)
        globalMax = value

        frequency.beginChanges()

        if (globalMin > value)
            setGlobalMin(value)

        for (const cpu of targets())
            setThreadMax(cpu, value)

        frequency.endChanges()
    }

    // Preset buttons: Min and Max in one batch
    function setGlobal(min, max) {
        frequency.beginChanges()
        setGlobalMin(min)
        setGlobalMax(max)
        frequency.endChanges()
    }

    function resetGlobal() {
        if (threads.length > 0 && minField(threads[0]) !== null) {
            globalMin = minField(threads[0]).value
            globalMax = maxField(threads[0]).value
        }
    }

    Component.onCompleted: resetGlobal()

    // Shown global values inside the range of the new filter, the threads are not changed
    onFilterChanged: {
        globalMin = clampToRange(globalMin)
        globalMax = clampToRange(globalMax)
    }

    Connections {
        target: frequency

        // New data from the daemon without changes of the user (after Apply, Cancel): the global values follow the first thread
        function onMessageChanged() {
            if (!frequency.dirty)
                grid.resetGlobal()
        }
    }

    // Width of the longest global value ("5400 MHz")
    TextMetrics {
        id: valueWidth

        font.pixelSize: theme.fontPixelSize
        text: grid.rangeMax + " MHz"
    }

    GridLayout {
        Layout.fillWidth: true

        columns: 4
        columnSpacing: 12

        Label { text: "Min" }

        ThemedSlider {
            Layout.fillWidth: true
            from: grid.rangeMin
            to: grid.rangeMax
            stepSize: 1
            value: grid.globalMin
            onMoved: grid.setGlobalMin(Math.round(value))
        }

        Label {
            Layout.preferredWidth: valueWidth.advanceWidth + 8   // the longest value, the text does not run into the slider
            text: grid.globalMin + " MHz"
            horizontalAlignment: Text.AlignRight
        }

        RowLayout {
            Label { text: "Show threads" }

            ThemedComboBox {
                implicitWidth: 150
                model: grid.filters
                currentIndex: grid.filters.indexOf(grid.filter)
                onActivated: index => grid.filter = grid.filters[index]
            }
        }

        Label { text: "Max" }

        ThemedSlider {
            Layout.fillWidth: true
            from: grid.rangeMin
            to: grid.rangeMax
            stepSize: 1
            value: grid.globalMax
            onMoved: grid.setGlobalMax(Math.round(value))
        }

        Label {
            Layout.preferredWidth: valueWidth.advanceWidth + 8   // the longest value, the text does not run into the slider
            text: grid.globalMax + " MHz"
            horizontalAlignment: Text.AlignRight
        }

        Item {}
    }

    // Presets (buttons of CPUFrequencyControl)
    Flow {
        Layout.fillWidth: true
        spacing: 6

        Repeater {
            model: grid.presets

            delegate: Row {
                spacing: 6

                ThemedButton {
                    text: "Min/" + modelData
                    onClicked: grid.setGlobal(grid.rangeMin, modelData)
                }

                ThemedButton {
                    text: modelData + "/" + modelData
                    onClicked: grid.setGlobal(modelData, modelData)
                }
            }
        }

        ThemedButton {
            text: "Default"
            onClicked: grid.setGlobal(grid.rangeMin, grid.rangeMax)
        }
    }

    // Cells column by column, P and E threads in their own columns, columns by the width (up to 4)
    GridLayout {
        id: cells

        readonly property int cellColumns: Math.max(1, Math.min(4, Math.floor((width + columnSpacing) / (grid.cellWidth + columnSpacing))))

        // P and E threads strictly in their own columns
        readonly property var placement: CellLayout.place([grid.performance, grid.efficient], cellColumns)

        Layout.fillWidth: true

        rowSpacing: 6
        columnSpacing: 6

        Repeater {
            model: grid.threads

            // Cell of a thread: group with the title tab like ThreadFrequencyControl (QGroupBox, background by the kind)
            delegate: Section {
                id: thread

                required property int modelData

                readonly property int        cpu:      modelData
                readonly property FieldModel minModel: (grid.minField(cpu))
                readonly property FieldModel maxModel: (grid.maxField(cpu))

                readonly property var position: cells.placement.positions[cpu] !== undefined ? cells.placement.positions[cpu] : [0, 0]

                Layout.row: position[0]
                Layout.column: position[1]
                Layout.fillWidth: true
                Layout.preferredWidth: grid.cellWidth

                title: grid.kindName[grid.kind(cpu)] + ": " + cpu
                       + ", CoreId: " + (frequency.daemonValue("cpus[" + cpu + "].core_id", frequency.revision))
                       + ", DieId: " + frequency.daemonValue("cpus[" + cpu + "].die_id", frequency.revision)
                backgroundColor: grid.kindBackground[grid.kind(cpu)]

                // Min and Max of the thread in one range slider (Min raises Max, Max lowers Min: setThreadMin/Max)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    ThemedRangeSlider {
                        id: range

                        Layout.fillWidth: true
                        enabled: thread.minModel !== null && thread.maxModel !== null
                        from: thread.minModel !== null ? thread.minModel.min : 0
                        to: thread.maxModel !== null ? thread.maxModel.max : 1
                        stepSize: 1
                        snapMode: RangeSlider.SnapAlways
                        first.value: thread.minModel !== null ? thread.minModel.value : 0
                        second.value: thread.maxModel !== null ? thread.maxModel.value : 1

                        // The model values are shown again after the limits and the coupling (the binding is restored)
                        first.onMoved: {
                            grid.setThreadMin(thread.cpu, Math.round(first.value))
                            first.value = Qt.binding(() => thread.minModel !== null ? thread.minModel.value : 0)
                        }
                        second.onMoved: {
                            grid.setThreadMax(thread.cpu, Math.round(second.value))
                            second.value = Qt.binding(() => thread.maxModel !== null ? thread.maxModel.value : 1)
                        }
                    }

                    // "Min – Max MHz", a changed value not applied yet in the change color; the width of the longest
                    // text and the mark reserved always: the slider does not change its width with the values
                    Row {
                        Layout.preferredWidth: rangeText.advanceWidth

                        TextMetrics {
                            id: rangeText

                            font.pixelSize: theme.fontPixelSize
                            text: "● " + grid.rangeMax + " – " + grid.rangeMax + " MHz"
                        }

                        // Mark of a changed value (besides the color)
                        Label {
                            text: "● "
                            color: theme.modified
                            opacity: (thread.minModel !== null && thread.minModel.modified) || (thread.maxModel !== null && thread.maxModel.modified) ? 1 : 0
                        }

                        Label {
                            text: thread.minModel !== null ? thread.minModel.value : ""
                            color: thread.minModel !== null && thread.minModel.modified ? theme.modified : "#e0e0e0"
                        }

                        Label {
                            text: " – "
                            color: "#e0e0e0"
                        }

                        Label {
                            text: thread.maxModel !== null ? thread.maxModel.value + " MHz" : ""
                            color: thread.maxModel !== null && thread.maxModel.modified ? theme.modified : "#e0e0e0"
                        }
                    }
                }
            }
        }
    }
}
