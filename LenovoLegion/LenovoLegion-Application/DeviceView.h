// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../LenovoLegion-Daemon/RGBControllerInterface.h"

#include <QWidget>

namespace LenovoLegionGui {
typedef struct
{
    float matrix_x;
    float matrix_y;
    float matrix_w;
    float matrix_h;
} matrix_pos_size_type;

class DeviceView : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceView(QWidget *parent = 0);
    ~DeviceView();

    virtual QSize sizeHint () const         override;
    virtual QSize minimumSizeHint () const  override;

    void setController(LenovoLegionDaemon::RGBControllerInterface * controller_ptr,const uint32_t vendorId,const uint32_t productId);
    void setNumericalLabels(bool enable);
    void setPerLED(bool per_led_mode);
    void markLeds(const QMap<int,QColor> &leds);
    void cleanup();

protected:
    void mousePressEvent(QMouseEvent *event)    override;
    void mouseMoveEvent(QMouseEvent *event)     override;
    void mouseReleaseEvent(QMouseEvent *)       override;
    void resizeEvent(QResizeEvent *event)       override;
    void paintEvent(QPaintEvent *)              override;

    void timerEvent(QTimerEvent *event) override;

private:
    QSize initSize;
    bool mouseDown;
    bool ctrlDown;
    bool mouseMoved;
    int size;
    int offset_x;
    QRect selectionRect;
    QPoint lastMousePos;
    QVector<int> previousSelection;
    QVector<int> selectedLeds;
    QMap<int,QColor>    marketLeds;
    QVector<bool> selectionFlags;
    QVector<bool> previousFlags;
    bool per_led;

    std::vector<matrix_pos_size_type>           zone_pos;
    std::vector<matrix_pos_size_type>           segment_pos;
    std::vector<matrix_pos_size_type>           led_pos;
    std::vector<QString>                        led_labels;
    std::map<int,QColor>                        led_to_color_map; //selected leds to color
    std::vector<LenovoLegionDaemon::RGBColor>   led_colors;

    float                               matrix_h;

    bool                                numerical_labels;

    LenovoLegionDaemon::RGBControllerInterface* controller;

    int                                 m_timerId;

    QColor posColor(const QPoint &point);
    void InitDeviceView();
    void updateSelection();

    std::string m_keyboardBackgroundImage;

signals:
    void selectionChanged(QVector<int>);

public slots:
    bool selectLed(int);
    bool selectLeds(QVector<int>);
    bool selectZone(int zone, bool add = false);
    void clearSelection(); // Same as selecting the entire device
    void setSelectionColor(LenovoLegionDaemon::RGBColor);
};

}
