// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include "RGBController.h"

#include "../LenovoLegion-PrepareBuild/Notification.pb.h"

#include <QFrame>

#include <memory>

namespace Ui
{
    class RGBKeyboardDevice;
}

class QTableWidgetItem;
namespace LenovoLegionGui {

class RGBKeyboardDevice : public QFrame
{
    Q_OBJECT

public:
    explicit RGBKeyboardDevice(RGBController *dev, QWidget *parent = nullptr);
    ~RGBKeyboardDevice();

    void dataProviderEvent(const legion::messages::Notification &notification);
    void cleanup();

private:

    /*
     * Profile/Brithness/Logo
     */
    void UpdateProfileUi();
    void UpdateBrightnessUi();
    void UpdateLogoUi();


    /*
     * Add Efects
     */
    void UpdateZoneUi();
    void UpdateLEDsUI(const QVector<int>& indices = QVector<int>());
    void UpdateModeUi(const int selectModeIdx = 0);
    void UpdatePerLEDUI(const std::optional<bool>& enabled = std::nullopt);
    void UpdateModeSpecificUi(const std::optional<bool>& enabled = std::nullopt);
    void UpdateRandomUi(const std::optional<bool>& enabled = std::nullopt);
    void UpdateModeSpecificColorsUi(const int selectModeIdx = -1);
    void UpdateModeSpecificColorCountUi(const int value = 1);
    void UpdateSpeedUi();
    void UpdateDirectionUi();
    void UpdateAddButtomUi();


    /*
     * Color selection
     */
    void UpdateColorFrameUi();



    /*
     * Current Effects
     */
    void UpdateClearAllEffectsButtomUi();
    void UpdateResetEffectsButtomUi();
    void UpdateEffectColorsUi();
    void UpdateEffectUi();
    void UpdateTableEffectsUi();


    /*
     * Enable/Disable Keyboard View
     */
    void ShowDeviceView();
    void HideDeviceView();

private slots:

    bool eventFilter(QObject* watched, QEvent* event);

    /*
     * Color selection changed event handlers
     */
    void on_ColorWheelBox_colorChanged(const QColor color);
    void on_SwatchBox_swatchChanged(const QColor color);
    void on_RedSpinBox_valueChanged(int red);
    void on_HueSpinBox_valueChanged(int hue);
    void on_GreenSpinBox_valueChanged(int green);
    void on_SatSpinBox_valueChanged(int sat);
    void on_BlueSpinBox_valueChanged(int blue);
    void on_ValSpinBox_valueChanged(int val);
    void on_HexLineEdit_textChanged(const QString &arg1);



    /*
     * Device view
     */
    void on_DeviceViewBox_selectionChanged(QVector<int>);
    void on_pushButtonToggleLEDView_clicked();



    /*
     * Brightness changed event handler
     */
    void on_BrightnessSlider_valueChanged(int value);

    /*
     * Profile changed event handler
     */
    void on_ProfileBox_currentIndexChanged(int index);

    /*
     * Add/Remove effects event handlers
     */
    void on_tableWidgetEffects_currentItemChanged(QTableWidgetItem *, QTableWidgetItem *previous);
    void on_tableWidgetEffects_cellEntered(int row, int column);
    void on_comboBox_EffectsColors_currentIndexChanged(int index);
    void on_pushButton_EffectsClearAll_clicked();
    void on_pushButton_AddEffects_clicked();
    void on_pushButton_EffectsDefault_clicked();

    /*
     * Mode change event handlers
     */
    void on_ModeBox_currentIndexChanged(int index);
    void on_ModeSpecificCheck_clicked();
    void on_PerLEDCheck_clicked();
    void on_RandomCheck_clicked();
    void on_comboBox_modeSpecificColor_currentIndexChanged(int index);
    void on_spinBox_modeSpecificColorCount_valueChanged(int value);
    void on_comboBox_logo_currentIndexChanged(int index);

private:
    Ui::RGBKeyboardDevice           *ui;
    std::unique_ptr<RGBController>   device;

    bool HexFormatRGB       = true;

    QColor current_color;
    void updateColorUi(const bool UpdateHex = true);
    void colorChanged(const bool UpdateHex  = true);


    /*
     * Tooltip Description
     */
    QString ModeDescription(const LenovoLegionDaemon::mode& m);
};

}
