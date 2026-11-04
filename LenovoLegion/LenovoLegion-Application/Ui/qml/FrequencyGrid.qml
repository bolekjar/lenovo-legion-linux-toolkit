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

    readonly property var  performance: (topology.revision, topology.count("active_cpus_core") > 0 ? ranges("active_cpus_core") : ranges("active_cpus"))
    readonly property var  efficient:   (topology.revision, ranges("active_cpus_atom"))
    readonly property var  threads:     performance.concat(efficient)
    readonly property bool hybrid:      topology.count("active_cpus_core") > 0 && efficient.length > 0

    readonly property var  filters: hybrid ? ["All", "Performance", "Efficiency"] : ["All"]
    property string        filter:  "All"

    // Range of the threads of the filter (MHz), the global values are clamped to it
    readonly property int  rangeMin: (frequency.revision, threadsRange(targets(), true))
    readonly property int  rangeMax: (frequency.revision, threadsRange(targets(), false))

    // Global Min/Max, initial values from the first active thread
    property int globalMin: 0
    property int globalMax: 0

    readonly property var presets: [2500, 3000, 3500, 4000, 4500, 5000]

    function ranges(field) {
        const cpus = []

        for (let range = 0; range < topology.count(field); ++range) {
            const min = topology.value(field + "[" + range + "].cpu_min")
            const max = topology.value(field + "[" + range + "].cpu_max")

            for (let cpu = min; cpu <= max; ++cpu)
                cpus.push(cpu)
        }

        return cpus
    }

    function minField(cpu) { return frequency.field("cpus[" + cpu + "].scaling_min_freq") }
    function maxField(cpu) { return frequency.field("cpus[" + cpu + "].scaling_max_freq") }

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
            Layout.preferredWidth: 60
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
            Layout.preferredWidth: 60
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

    GridLayout {
        Layout.fillWidth: true

        rowSpacing: 6
        columnSpacing: 6

        Repeater {
            model: grid.threads

            delegate: Rectangle {
                id: thread

                required property int modelData

                readonly property int        cpu:      modelData
                readonly property FieldModel minModel: (frequency.revision, grid.minField(cpu))
                readonly property FieldModel maxModel: (frequency.revision, grid.maxField(cpu))

                Layout.row: cpu % 8
                Layout.column: Math.floor(cpu / 8)
                Layout.fillWidth: true

                implicitHeight: content.implicitHeight + 12
                implicitWidth: 260

                radius: theme.radius
                color: grid.kindBackground[grid.kind(cpu)]
                border.width: 1
                border.color: theme.groupBorder

                ColumnLayout {
                    id: content

                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 2

                    Label {
                        text: grid.kindName[grid.kind(thread.cpu)] + ": " + thread.cpu
                              + ", CoreId: " + (frequency.revision, frequency.daemonValue("cpus[" + thread.cpu + "].core_id"))
                              + ", DieId: " + frequency.daemonValue("cpus[" + thread.cpu + "].die_id")
                        color: "#e0e0e0"
                        font.bold: true
                    }

                    Repeater {
                        model: [{ name: "Min", model: thread.minModel, set: grid.setThreadMin }, { name: "Max", model: thread.maxModel, set: grid.setThreadMax }]

                        delegate: RowLayout {
                            Layout.fillWidth: true

                            Label {
                                Layout.preferredWidth: 30
                                text: modelData.name
                                color: modelData.model !== null && modelData.model.modified ? theme.modified : "#e0e0e0"
                            }

                            ThemedSlider {
                                Layout.fillWidth: true
                                from: modelData.model !== null ? modelData.model.min : 0
                                to: modelData.model !== null ? modelData.model.max : 1
                                stepSize: 1
                                value: modelData.model !== null ? modelData.model.value : 0
                                onMoved: modelData.set(thread.cpu, Math.round(value))
                            }

                            Label {
                                Layout.preferredWidth: 40
                                text: modelData.model !== null ? modelData.model.value : ""
                                color: "#e0e0e0"
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }
            }
        }
    }
}
