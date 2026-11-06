// SPDX-License-Identifier: GPL-2.0
// Monochrome icon (SVG) in a given color: the image is drawn and colored by "source-in" composition
// (Canvas, works also with the software renderer, no shader effects)
import QtQuick

Canvas {
    id: icon

    property url   source
    property color color: "black"

    implicitWidth: 16
    implicitHeight: 16

    onSourceChanged: if (source.toString().length > 0) loadImage(source)
    onColorChanged: requestPaint()
    onImageLoaded: requestPaint()
    Component.onCompleted: if (source.toString().length > 0) loadImage(source)

    onPaint: {
        const context = getContext("2d")

        context.clearRect(0, 0, width, height)

        if (!isImageLoaded(source))
            return

        context.globalCompositeOperation = "source-over"
        context.drawImage(source, 0, 0, width, height)
        context.globalCompositeOperation = "source-in"
        context.fillStyle = icon.color
        context.fillRect(0, 0, width, height)
        context.globalCompositeOperation = "source-over"
    }
}
