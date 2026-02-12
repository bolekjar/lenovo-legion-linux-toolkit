// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include <QSlider>

namespace LenovoLegionGui {
class QTooltipedSlider : public QSlider
{
    Q_OBJECT

public:
    explicit QTooltipedSlider(QWidget *parent = nullptr);
};
}
