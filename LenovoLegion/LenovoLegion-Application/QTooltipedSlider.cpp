// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "QTooltipedSlider.h"

#include <QToolTip>


namespace LenovoLegionGui {

QTooltipedSlider::QTooltipedSlider(QWidget *parent) :
    QSlider(parent)
{
    connect(this, &QSlider::sliderMoved,[&](int value) {
        QToolTip::showText(QCursor::pos(), QString("%1").arg(value), this);
    });
}

}
