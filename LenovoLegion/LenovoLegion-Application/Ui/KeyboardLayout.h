// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "DataSource.h"

#include <QObject>
#include <QPointer>
#include <QVariantList>

namespace legion::messages { class RGBControllerResponse; }

namespace LenovoLegionGui {

/*
 * Layout of the LEDs of the RGB keyboard for KeyboardView.qml. Source is the RGB_CONTROLLER data source
 * (RGBControllerResponse with leds and zones). Positions are relative to the width (0..1), heightRatio is the height.
 *
 * Lenovo Legion keyboards (enough LED codes of the template found): the keys at their physical positions by the key
 * codes, the light bars of the chassis (rear around the vents, front along the bottom edge) and the logo on the lid
 * in three views of the notebook: from above (open lid, keys, front bar), from the front, from the back (lid with the logo).
 * Other devices: the same as DeviceView of the widget pages (OpenRGB): matrix zones by their matrix map, large keys
 * (Tab, Shift, Enter, Space, ...) expanded over the empty cells, single/linear zones in rows of 20 LEDs.
 */
class KeyboardLayout : public QObject
{
    Q_OBJECT

    Q_PROPERTY(LenovoLegionGui::DataSource* source READ source WRITE setSource NOTIFY sourceChanged)

    // [{index, indexes, x, y, w, h, label, name, kind, view}] of every key (LEDs with the same name / code are one key),
    // relative to the view (width 1), kind: "key", "rear", "front" (light bars), "logo", "other" (no place known);
    // a LED can be in more views (the front light bar from above and from the front)
    Q_PROPERTY(QVariantList keys        READ keys        NOTIFY layoutChanged)
    // [{name, x, y, w, h}] of every zone (label position)
    Q_PROPERTY(QVariantList zones       READ zones       NOTIFY layoutChanged)
    Q_PROPERTY(qreal        heightRatio READ heightRatio NOTIFY layoutChanged)
    // [{name, title, hint, heightRatio, lid, body, deck, vents}] views of the device ("top", "front", "back"), the keys
    // have the name of their view, positions relative to the width of the view; lid, body, deck {x, y, w, h} (if drawn)
    Q_PROPERTY(QVariantList views       READ views       NOTIFY layoutChanged)

public:

    explicit KeyboardLayout(QObject* parent = nullptr);

    DataSource*  source() const;
    void         setSource(DataSource* source);

    QVariantList keys() const;
    QVariantList zones() const;
    qreal        heightRatio() const;
    QVariantList views() const;

signals:

    void sourceChanged();
    void layoutChanged();

private:

    void update();
    bool updateByTemplate(const legion::messages::RGBControllerResponse& data);

    QPointer<DataSource> m_source;
    QByteArray           m_layoutData;     // serialized leds and zones of the last layout (unchanged data -> no new layout)
    QVariantList         m_keys;
    QVariantList         m_zones;
    qreal                m_heightRatio = 0;
    QVariantList         m_views;
};

}
