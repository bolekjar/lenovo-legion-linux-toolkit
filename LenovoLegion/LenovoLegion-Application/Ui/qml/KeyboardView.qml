// SPDX-License-Identifier: GPL-2.0
// RGB keyboard: every LED in its current color, drawn (no photos). Layout by KeyboardLayout: on the Lenovo Legion
// keyboards three views of the notebook in tabs (Keyboard: open lid with the display, keys, front light bar from above;
// Front: front light bar; Back: logo on the lid and the rear light bar around the vents, switched by a switcher in
// the style of the notebook), otherwise the keys of the
// matrix zones on a deck. Colors from the state source (RGB_CONTROLLER with REQUEST_STATE_FOR_ALL_LEDS, read
// periodically). Click selects the LEDs of a key / segment, Ctrl+click adds / removes them, dragging with the left
// button selects all in the rectangle (Ctrl: added), the selection is the same in all views (the tabs show how many
// of their keys / segments are selected).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import LenovoLegion.Ui 1.0

Item {
    id: view

    required property DataSource source         // RGB_CONTROLLER: LEDs, zones, device info
    required property DataSource stateSource    // RGB_CONTROLLER: colors of all LEDs (0x00BBGGRR)

    property var selected: []                   // indexes of the selected LEDs
    property var marked: []                     // indexes of highlighted LEDs (LEDs of the effect under the mouse)

    // Indexes of the LEDs of the keys (effects of the whole keyboard)
    readonly property var keyLeds: layout.keyLeds

    readonly property var colors: stateSource.repeatedValues("colors", stateSource.revision)

    function ledColor(index) {
        const rgb = index < colors.length ? colors[index] : 0
        return Qt.rgba((rgb & 0xFF) / 255, ((rgb >> 8) & 0xFF) / 255, ((rgb >> 16) & 0xFF) / 255, 1)
    }

    // The selection as an argument: bindings are evaluated again when it changes
    function isSelected(indexes, selection) {
        return indexes.some(index => selection.indexOf(index) >= 0)
    }

    // Click: only this key / segment (again: nothing), Ctrl+click: add / remove it
    function select(indexes, add) {
        const all = indexes.every(index => selected.indexOf(index) >= 0)

        if (!add) {
            selected = all && selected.length === indexes.length ? [] : indexes.slice()
            return
        }

        selected = all ? selected.filter(index => indexes.indexOf(index) < 0)
                       : selected.concat(indexes.filter(index => selected.indexOf(index) < 0))
    }

    function viewKeys(keys, name) {
        return keys.filter(key => key.view === name)
    }

    // Number of keys / segments of a view with a selected LED
    function selectedIn(name, selection) {
        return viewKeys(layout.keys, name).filter(key => isSelected(key.indexes, selection)).length
    }

    // All LEDs of all views (keys, light bars, logo)
    function selectAll() {
        let indexes = []

        for (const key of layout.keys)
            indexes = union(indexes, key.indexes)

        selected = indexes
    }

    function union(first, second) {
        return first.concat(second.filter(index => first.indexOf(index) < 0))
    }

    // Distance of a point (px of the view) from the line of a segment along a path (rear light bar)
    function pathDistance(key, point) {
        let best = Infinity

        for (let i = 0; i + 1 < key.path.length; ++i) {
            const ax = (key.x + key.path[i].x) * area.width, ay = (key.y + key.path[i].y) * area.width
            const bx = (key.x + key.path[i + 1].x) * area.width, by = (key.y + key.path[i + 1].y) * area.width
            const length = (bx - ax) * (bx - ax) + (by - ay) * (by - ay)
            const t = length > 0 ? Math.max(0, Math.min(1, ((point.x - ax) * (bx - ax) + (point.y - ay) * (by - ay)) / length)) : 0
            best = Math.min(best, Math.hypot(point.x - (ax + t * (bx - ax)), point.y - (ay + t * (by - ay))))
        }

        return best
    }

    // Key / segment under a point of the view (px), null outside of the LEDs
    function keyAt(point) {
        const keys = viewKeys(layout.keys, currentName)

        for (let i = keys.length - 1; i >= 0; --i) {
            const key = keys[i]

            if (key.path !== undefined) {
                if (pathDistance(key, point) <= Math.max(key.thickness * area.width, 8))
                    return key
                continue
            }

            if (point.x >= key.x * area.width && point.x <= (key.x + key.w) * area.width
                    && point.y >= key.y * area.width && point.y <= (key.y + key.h) * area.width)
                return key
        }

        return null
    }

    // LED indexes of the keys / segments in a rectangle of the view (px): segments along a path by the points of the line
    function keysInRect(x, y, width, height) {
        let indexes = []
        const inside = (px, py) => px >= x && px <= x + width && py >= y && py <= y + height

        for (const key of viewKeys(layout.keys, currentName)) {
            let hit = false

            if (key.path !== undefined) {
                for (let i = 0; i < key.path.length && !hit; ++i) {
                    const px = (key.x + key.path[i].x) * area.width, py = (key.y + key.path[i].y) * area.width
                    hit = inside(px, py)

                    if (!hit && i + 1 < key.path.length)
                        hit = inside((px + (key.x + key.path[i + 1].x) * area.width) / 2, (py + (key.y + key.path[i + 1].y) * area.width) / 2)
                }
            } else {
                hit = key.x * area.width < x + width && (key.x + key.w) * area.width > x
                   && key.y * area.width < y + height && (key.y + key.h) * area.width > y
            }

            if (hit)
                indexes = union(indexes, key.indexes)
        }

        return indexes
    }

    KeyboardLayout {
        id: layout

        source: view.source
    }

    readonly property var    currentView: tabs.currentIndex >= 0 && tabs.currentIndex < layout.views.length ? layout.views[tabs.currentIndex] : null
    readonly property string currentName: currentView !== null ? currentView.name : "top"

    // The highest view: the same height in all tabs (no jumping), smaller views vertically centered
    readonly property real maxHeightRatio: Math.max(0, ...layout.views.map(item => item.heightRatio))

    // Width of the drawing: not wider than maxDrawWidth and not higher than maxHeightPart of the window (a large window
    // would make it huge with a scroll bar), centered
    property real maxDrawWidth: 1100
    property real maxHeightPart: 0.55
    readonly property real drawWidth: Math.min(view.width, maxDrawWidth,
                                               Window.height > 0 && maxHeightRatio > 0 ? Window.height * maxHeightPart / maxHeightRatio : view.width)

    implicitHeight: content.implicitHeight

    // Selection on the dark notebook in all themes: the accent of the theme brightened to a luminance of at least
    // minSelectionLuma (a dark accent of the gray / white theme is not visible on the dark chassis), the other LEDs
    // dimmed while something is selected or highlighted (light bars and the logo less: dimmed thin lights look
    // switched off)
    readonly property real  minSelectionLuma: 0.6
    readonly property color selectionColor: {
        const accent = theme.accentHover
        const luma = 0.2126 * accent.r + 0.7152 * accent.g + 0.0722 * accent.b
        return luma >= minSelectionLuma ? accent : Qt.tint(accent, Qt.rgba(1, 1, 1, (minSelectionLuma - luma) / (1 - luma)))
    }
    readonly property real  dimmedOpacity: 0.45       // brightness of the color of the dimmed keys
    readonly property real  dimmedLightOpacity: 0.65  // brightness of the color of the dimmed light bars and the logo

    // Housing of every LED (rim of the keys, rail of the light bars, outline of the logo): a dark / switched off LED
    // is still visible on the dark notebook in all themes
    readonly property color ledRimColor: "#8a8a94"
    readonly property bool  dimOthers: selected.length > 0 || marked.length > 0

    // Active area of the mouse selection outside of the view (padding of the group around it)
    property real selectionMargin: 0

    // Selection by the mouse over the whole widget (the view, the free space around the drawing and selectionMargin
    // around it, the switcher and the buttons are over it): click selects the key / segment under the cursor (outside
    // of the LEDs: nothing), dragging with the left button selects all in the rectangle, Ctrl adds to the selection
    MouseArea {
        id: selection

        anchors.fill: parent
        anchors.margins: -view.selectionMargin
        z: -1

        // The page scrolls (Flickable): dragging here selects, it does not scroll the page
        preventStealing: true

        property point start
        property bool  dragging: false
        property var   base: []

        function toArea(mouse) {
            return mapToItem(area, mouse.x, mouse.y)
        }

        onPressed: mouse => {
            start = toArea(mouse)
            dragging = false
            base = (mouse.modifiers & Qt.ControlModifier) !== 0 ? view.selected.slice() : []
        }

        onPositionChanged: mouse => {
            const point = toArea(mouse)

            if (!dragging && Math.abs(point.x - start.x) + Math.abs(point.y - start.y) < 6)
                return

            dragging = true
            band.update(start, point)
            view.selected = view.union(base, view.keysInRect(band.x, band.y, band.width, band.height))
        }

        onReleased: mouse => {
            if (dragging) {
                dragging = false
                return
            }

            const key = view.keyAt(toArea(mouse))
            if (key === null) {
                if ((mouse.modifiers & Qt.ControlModifier) === 0)
                    view.selected = []
                return
            }

            view.select(key.indexes, (mouse.modifiers & Qt.ControlModifier) !== 0)
        }

        onCanceled: dragging = false
    }

    ColumnLayout {
        id: content

        x: (view.width - width) / 2
        width: view.drawWidth
        spacing: 0

        // Views of the device, the number of the selected keys / segments, select all / nothing
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Switcher of the views in the style of the notebook: dark chassis, a pictogram of the view, the selected
            // view lit by a light bar in the accent color, the number of its selected keys / segments in a badge
            Rectangle {
                id: tabs

                objectName: "viewSwitcher"
                property int currentIndex: 0

                visible: layout.views.length > 1
                implicitWidth: switcherRow.implicitWidth + 8
                implicitHeight: switcherRow.implicitHeight + 8
                radius: height / 2
                border.width: 1
                border.color: "#46464c"
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#2c2c31" }
                    GradientStop { position: 1.0; color: "#16161a" }
                }

                Row {
                    id: switcherRow

                    anchors.centerIn: parent
                    spacing: 4

                    Repeater {
                        model: layout.views

                        delegate: AbstractButton {
                            id: viewButton

                            required property var modelData
                            required property int index

                            readonly property int  count: view.selectedIn(modelData.name, view.selected)
                            readonly property bool current: tabs.currentIndex === index

                            hoverEnabled: true
                            implicitHeight: 36
                            implicitWidth: buttonRow.implicitWidth + 28

                            onClicked: tabs.currentIndex = index

                            background: Rectangle {
                                radius: height / 2
                                color: viewButton.current ? "#3b3b44" : viewButton.hovered ? "#2f2f36" : "transparent"
                                border.width: viewButton.current ? 1 : 0
                                border.color: "#55555e"

                                // Light bar of the selected view (like the LED bars of the notebook)
                                Rectangle {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: 3
                                    width: parent.width - 2 * parent.radius
                                    height: 3
                                    radius: 1.5
                                    visible: viewButton.current
                                    gradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: theme.accent }
                                        GradientStop { position: 1.0; color: theme.accentHover }
                                    }
                                }
                            }

                            contentItem: Item {
                                implicitWidth: buttonRow.implicitWidth
                                implicitHeight: buttonRow.implicitHeight

                                Row {
                                    id: buttonRow

                                    anchors.centerIn: parent
                                    spacing: 8

                                    // Pictogram of the view: lid and body from above / the front / the back
                                    Canvas {
                                        id: pictogram

                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 24
                                        height: 16

                                        readonly property color lineColor: viewButton.current ? "white" : "#a0a0a8"
                                        readonly property color lightColor: viewButton.current ? theme.accentHover : "#6a6a72"
                                        onLineColorChanged: requestPaint()
                                        onLightColorChanged: requestPaint()

                                        onPaint: {
                                            const ctx = getContext("2d")
                                            ctx.reset()
                                            ctx.lineWidth = 1.2
                                            ctx.strokeStyle = lineColor

                                            if (viewButton.modelData.name === "top") {
                                                // Open lid (trapezoid) and the body with rows of keys
                                                ctx.beginPath()
                                                ctx.moveTo(1, 1); ctx.lineTo(23, 1); ctx.lineTo(21, 6); ctx.lineTo(3, 6); ctx.closePath()
                                                ctx.stroke()
                                                ctx.strokeRect(2.5, 7.5, 19, 8)
                                                ctx.fillStyle = lineColor
                                                for (let row = 0; row < 2; ++row)
                                                    for (let col = 0; col < 6; ++col)
                                                        ctx.fillRect(4.5 + col * 2.6, 9.5 + row * 2.6, 1.6, 1.6)
                                            } else {
                                                // Lid (display or back of the lid) and the edge of the body with its light
                                                ctx.strokeRect(3.5, 0.5, 17, 10)
                                                ctx.strokeRect(0.5, 11.5, 23, 4)
                                                ctx.fillStyle = lightColor
                                                if (viewButton.modelData.name === "front")
                                                    ctx.fillRect(2, 13.2, 20, 1.6)
                                                else {
                                                    ctx.fillRect(2, 12.6, 8, 1.2); ctx.fillRect(14, 12.6, 8, 1.2)
                                                    ctx.fillRect(10, 13.6, 4, 1)
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: viewButton.modelData.title
                                        font.pixelSize: theme.fontPixelSize
                                        font.bold: viewButton.current
                                        color: viewButton.current ? "white" : "#a0a0a8"
                                    }

                                    // Number of the selected keys / segments of the view
                                    Rectangle {
                                        anchors.verticalCenter: parent.verticalCenter
                                        visible: viewButton.count > 0
                                        width: Math.max(height, badgeText.implicitWidth + 10)
                                        height: 18
                                        radius: 9
                                        color: theme.accent

                                        Text {
                                            id: badgeText

                                            anchors.centerIn: parent
                                            text: viewButton.count
                                            font.pixelSize: theme.fontPixelSize - 2
                                            font.bold: true
                                            color: theme.accentText
                                        }
                                    }
                                }
                            }

                            ThemedToolTip {
                                visible: viewButton.hovered && viewButton.modelData.hint.length > 0
                                delay: 500
                                text: viewButton.modelData.hint
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: view.selected.length > 0 ? view.selected.length + " LEDs selected" : "Click or drag: select, Ctrl: add"
                color: theme.disabledText
            }

            ThemedButton {
                text: "Select all"
                onClicked: view.selectAll()
            }

            ThemedButton {
                text: "Clear"
                enabled: view.selected.length > 0
                onClicked: view.selected = []
            }
        }

        Rectangle {
            id: stage

            Layout.fillWidth: true
            Layout.topMargin: 8
            implicitHeight: width > 0 ? (width - 2 * padding) * view.maxHeightRatio + 2 * padding : 0

            readonly property real padding: Math.max(10, width / 70)

            // Background of the theme like the tab pane of the home page (QTabWidget::pane: gradient, radius), no border
            radius: theme.styled ? 12 : 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: theme.groupBackground }
                GradientStop { position: 1.0; color: theme.window }
            }

            Item {
                id: area

                readonly property real viewHeight: view.currentView !== null ? view.currentView.heightRatio * width : 0

                x: stage.padding
                width: stage.width - 2 * stage.padding
                y: (stage.height - viewHeight) / 2
                height: viewHeight

                function px(rect) {
                    return rect !== undefined ? Qt.rect(rect.x * width, rect.y * width, rect.w * width, rect.h * width) : Qt.rect(0, 0, 0, 0)
                }

                readonly property rect lid:  px(view.currentView !== null ? view.currentView.lid : undefined)
                readonly property rect body: px(view.currentView !== null ? view.currentView.body : undefined)
                readonly property rect deck: px(view.currentView !== null ? view.currentView.deck : undefined)

                // Top view: open lid with the display (wider at the top, perspective), hinge
                Canvas {
                    id: openLid

                    visible: view.currentName === "top" && area.lid.width > 0
                    x: area.lid.x
                    y: area.lid.y
                    width: area.lid.width
                    height: area.lid.height

                    onWidthChanged: requestPaint()
                    onHeightChanged: requestPaint()
                    onVisibleChanged: requestPaint()

                    onPaint: {
                        const ctx = getContext("2d")
                        const inset = width * 0.03
                        ctx.reset()

                        // Bezel
                        ctx.beginPath()
                        ctx.moveTo(0, 0)
                        ctx.lineTo(width, 0)
                        ctx.lineTo(width - inset, height)
                        ctx.lineTo(inset, height)
                        ctx.closePath()
                        ctx.fillStyle = "#121215"
                        ctx.fill()
                        ctx.strokeStyle = "#3a3a40"
                        ctx.lineWidth = 1
                        ctx.stroke()

                        // Display
                        const b = height * 0.07
                        const gradient = ctx.createLinearGradient(0, b, 0, height - b)
                        gradient.addColorStop(0.0, "#0d1330")
                        gradient.addColorStop(0.6, "#2a1f5c")
                        gradient.addColorStop(1.0, "#6a2a6e")
                        ctx.beginPath()
                        ctx.moveTo(b * 1.2, b)
                        ctx.lineTo(width - b * 1.2, b)
                        ctx.lineTo(width - inset - b * 0.8, height - b * 0.6)
                        ctx.lineTo(inset + b * 0.8, height - b * 0.6)
                        ctx.closePath()
                        ctx.fillStyle = gradient
                        ctx.fill()
                    }
                }

                // Front / back view: the lid as a rectangle (display or the back of the lid)
                Rectangle {
                    visible: view.currentName !== "top" && area.lid.width > 0
                    x: area.lid.x
                    y: area.lid.y
                    width: area.lid.width
                    height: area.lid.height
                    radius: stage.padding / 2
                    border.width: 1
                    border.color: "#3a3a40"
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: view.currentName === "back" ? "#34343a" : "#121215" }
                        GradientStop { position: 1.0; color: view.currentName === "back" ? "#1c1c20" : "#121215" }
                    }

                    // Display of the front view
                    Rectangle {
                        visible: view.currentName === "front"
                        anchors.fill: parent
                        anchors.margins: parent.height * 0.05
                        radius: 2
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#0d1330" }
                            GradientStop { position: 0.6; color: "#2a1f5c" }
                            GradientStop { position: 1.0; color: "#6a2a6e" }
                        }
                    }
                }

                // Chassis
                Rectangle {
                    visible: area.body.width > 0
                    x: area.body.x
                    y: area.body.y
                    width: area.body.width
                    height: area.body.height
                    radius: Math.min(stage.padding, height / 3)
                    border.width: 1
                    border.color: "#46464c"
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#2c2c31" }
                        GradientStop { position: 1.0; color: "#16161a" }
                    }
                }

                // Back view: vents of the rear and the hinge
                Repeater {
                    model: view.currentName === "back" && view.currentView.vents !== undefined ? view.currentView.vents : []

                    delegate: Rectangle {
                        required property var modelData

                        x: modelData.x * area.width
                        y: modelData.y * area.width
                        width: modelData.w * area.width
                        height: modelData.h * area.width
                        radius: 2
                        color: "#0c0c0e"
                        border.width: 1
                        border.color: "#26262b"
                    }
                }

                // Top view: plate of the keys (whole area of a device without the chassis)
                Rectangle {
                    visible: area.deck.width > 0
                    x: area.deck.x - stage.padding / 2
                    y: area.deck.y - stage.padding / 2
                    width: area.deck.width + stage.padding
                    height: area.deck.height + stage.padding
                    radius: stage.padding / 2
                    color: "#101013"
                    border.width: 1
                    border.color: "#050506"
                }

                // Gap between the keys (the layout of the keys can touch), at least 3 px
                readonly property real keyGap: Math.max(3, width / 180)

                Repeater {
                    model: view.viewKeys(layout.keys, view.currentName)

                    delegate: Loader {
                        id: led

                        required property var modelData

                        // Color of the LED, darker while other LEDs are selected / highlighted (the housing stays visible);
                        // light bars and the logo less than the keys
                        readonly property bool  dimmed: view.dimOthers && !isSelected && !isMarked
                        readonly property real  dimFactor: modelData.kind === "key" ? view.dimmedOpacity : view.dimmedLightOpacity
                        readonly property color rawColor: view.ledColor(modelData.index)
                        readonly property color ledColor: dimmed ? Qt.rgba(rawColor.r * dimFactor, rawColor.g * dimFactor, rawColor.b * dimFactor, 1)
                                                                 : rawColor
                        readonly property bool  isSelected: view.isSelected(modelData.indexes, view.selected)
                        readonly property bool  isMarked: view.isSelected(modelData.indexes, view.marked)
                        // Outline: white for the highlighted LEDs (effect under the mouse), the selection color for the
                        // selection, the same width for all kinds of LEDs
                        readonly property color outlineColor: isMarked ? "white" : view.selectionColor
                        readonly property real  outlineWidth: 3

                        x: modelData.x * area.width
                        y: modelData.y * area.width
                        width: modelData.w * area.width
                        height: modelData.h * area.width

                        sourceComponent: modelData.kind === "key" ? keyCap : modelData.kind === "logo" ? logo
                                       : modelData.path !== undefined ? lightLine : lightSegment

                        HoverHandler {
                            id: ledHover

                            cursorShape: Qt.PointingHandCursor
                        }

                        ThemedToolTip {
                            visible: ledHover.hovered && !selection.dragging
                            delay: 600
                            text: led.modelData.name + "\nLED " + led.modelData.indexes.join(", ")
                        }

                        // Key cap in the color of the LED: darker side visible at the bottom, brighter top, label
                        Component {
                            id: keyCap

                            Item {
                                id: keyItem

                                readonly property real corner: Math.min(width, height) * 0.16
                                readonly property real luma: 0.2126 * led.ledColor.r + 0.7152 * led.ledColor.g + 0.0722 * led.ledColor.b

                                Item {
                                    anchors.fill: parent
                                    anchors.margins: area.keyGap / 2

                                    Rectangle {
                                        anchors.fill: parent
                                        radius: keyItem.corner
                                        color: Qt.darker(led.ledColor, 2.2)
                                    }

                                    Rectangle {
                                        id: cap

                                        anchors.fill: parent
                                        anchors.bottomMargin: Math.max(2, parent.height * 0.08)
                                        radius: keyItem.corner
                                        color: led.ledColor
                                        border.width: led.isSelected ? 2 : 1
                                        border.color: led.isSelected ? "white" : view.ledRimColor

                                        Rectangle {
                                            anchors.fill: parent
                                            anchors.margins: cap.border.width
                                            radius: cap.radius
                                            gradient: Gradient {
                                                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.25) }
                                                GradientStop { position: 0.55; color: Qt.rgba(1, 1, 1, 0.0) }
                                            }
                                        }

                                        Text {
                                            anchors.centerIn: parent
                                            width: parent.width - 4
                                            height: parent.height
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            // Long labels (Num, Copilot on the narrow keys) smaller instead of cut
                                            fontSizeMode: Text.HorizontalFit
                                            minimumPixelSize: 6
                                            text: led.modelData.label
                                            font.pixelSize: Math.max(7, Math.min(cap.height * 0.38, 14))
                                            color: keyItem.luma > 0.5 ? "black" : "white"
                                        }
                                    }

                                    // Selection / highlight: outline around the key
                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.margins: -led.outlineWidth
                                        visible: led.isSelected || led.isMarked
                                        radius: keyItem.corner + led.outlineWidth
                                        color: "transparent"
                                        border.width: led.outlineWidth
                                        border.color: led.outlineColor
                                    }
                                }
                            }
                        }

                        // Segment of a light bar: glow around, the light itself, a small gap to the next segment
                        Component {
                            id: lightSegment

                            Item {
                                id: segment

                                readonly property real thickness: Math.min(width, height)

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.leftMargin: 1
                                    anchors.rightMargin: 1
                                    anchors.topMargin: -segment.thickness * 0.6
                                    anchors.bottomMargin: -segment.thickness * 0.6
                                    radius: segment.thickness
                                    color: led.ledColor
                                    opacity: 0.25
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.leftMargin: 1
                                    anchors.rightMargin: 1
                                    radius: segment.thickness / 2
                                    color: led.ledColor
                                    // Thin white edge when selected (the color of the light stays visible)
                                    border.width: 1
                                    border.color: led.isSelected ? "white" : view.ledRimColor
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: -led.outlineWidth
                                    visible: led.isSelected || led.isMarked
                                    radius: segment.thickness / 2 + led.outlineWidth
                                    color: "transparent"
                                    border.width: led.outlineWidth
                                    border.color: led.outlineColor
                                }
                            }
                        }

                        // Segment of a light bar along a line (rear bar around the vents): the light, selection
                        Component {
                            id: lightLine

                            Item {
                            // Larger than the segment: the selection outline is wider than the line
                            Canvas {
                                readonly property real  margin: 6

                                x: -margin
                                y: -margin
                                width: parent.width + 2 * margin
                                height: parent.height + 2 * margin

                                readonly property color lineColor: led.ledColor
                                readonly property bool  selectedLine: led.isSelected
                                readonly property bool  markedLine: led.isMarked
                                readonly property color outlineColor: led.outlineColor
                                readonly property real  thickness: led.modelData.thickness * area.width

                                onLineColorChanged: requestPaint()
                                onSelectedLineChanged: requestPaint()
                                onMarkedLineChanged: requestPaint()
                                onOutlineColorChanged: requestPaint()
                                onWidthChanged: requestPaint()
                                onHeightChanged: requestPaint()

                                function stroke(ctx, width, color) {
                                    ctx.beginPath()

                                    for (let i = 0; i < led.modelData.path.length; ++i) {
                                        const point = led.modelData.path[i]
                                        if (i === 0)
                                            ctx.moveTo(point.x * area.width + margin, point.y * area.width + margin)
                                        else
                                            ctx.lineTo(point.x * area.width + margin, point.y * area.width + margin)
                                    }

                                    ctx.lineWidth = width
                                    ctx.strokeStyle = color
                                    ctx.stroke()
                                }

                                onPaint: {
                                    const ctx = getContext("2d")
                                    ctx.reset()
                                    ctx.lineCap = "butt"
                                    ctx.lineJoin = "round"

                                    // Selected / highlighted: outline, then a thin white edge (like the selected keys), the
                                    // light in its color over it; otherwise the rail of the housing
                                    if (selectedLine || markedLine) {
                                        stroke(ctx, thickness + 2 + 2 * led.outlineWidth, outlineColor)
                                        stroke(ctx, thickness + 2, "white")
                                    } else {
                                        stroke(ctx, thickness + 3, view.ledRimColor)
                                    }

                                    stroke(ctx, thickness, lineColor)
                                }
                            }
                            }
                        }

                        // Logo on the lid in the color of its LED
                        Component {
                            id: logo

                            Item {
                                Text {
                                    anchors.centerIn: parent
                                    text: led.modelData.label
                                    font.pixelSize: parent.height * 0.85
                                    font.bold: true
                                    font.letterSpacing: parent.height * 0.12
                                    color: led.ledColor
                                    // Selected: white outline of the letters like the border of the selected keys
                                    // not selected: the rim of the LEDs (a dark logo is still visible)
                                    style: Text.Outline
                                    styleColor: led.isSelected ? "white" : view.ledRimColor
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: -2
                                    visible: led.isSelected
                                    radius: 4
                                    color: "transparent"
                                    border.width: 2
                                    border.color: "white"
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: -2 - led.outlineWidth
                                    visible: led.isSelected || led.isMarked
                                    radius: 4 + led.outlineWidth
                                    color: "transparent"
                                    border.width: led.outlineWidth
                                    border.color: led.outlineColor
                                }
                            }
                        }
                    }
                }

                // Rectangle of the selection by dragging
                Rectangle {
                    id: band

                    function update(from, to) {
                        x = Math.min(from.x, to.x)
                        y = Math.min(from.y, to.y)
                        width = Math.abs(to.x - from.x)
                        height = Math.abs(to.y - from.y)
                    }

                    visible: selection.dragging
                    color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.2)
                    border.width: 1
                    border.color: theme.accentHover
                }
            }
        }
    }
}
