// SPDX-License-Identifier: GPL-2.0
// Logo of Lenovo Legion cut to its circle (the image has a white square background) with a ring in the accent color
import QtQuick

Canvas {
    id: logo

    readonly property url image: "qrc:/images/icons/LenovoLegionIco.png"
    property color ring: theme.accentHover
    property real  ringWidth: Math.max(2, width / 21)

    implicitWidth: 64
    implicitHeight: 64

    onRingChanged: requestPaint()
    onWidthChanged: requestPaint()
    onImageLoaded: requestPaint()
    Component.onCompleted: loadImage(image)

    onPaint: {
        const ctx = getContext("2d")
        const r = width / 2
        ctx.reset()

        if (isImageLoaded(image)) {
            ctx.save()
            ctx.beginPath()
            ctx.arc(r, r, r - ringWidth - 0.5, 0, 2 * Math.PI)
            ctx.closePath()
            ctx.clip()
            ctx.drawImage(image, ringWidth / 2, ringWidth / 2, width - ringWidth, height - ringWidth)
            ctx.restore()
        }

        ctx.beginPath()
        ctx.arc(r, r, r - ringWidth / 2 - 0.5, 0, 2 * Math.PI)
        ctx.lineWidth = ringWidth
        ctx.strokeStyle = ring
        ctx.stroke()
    }
}
