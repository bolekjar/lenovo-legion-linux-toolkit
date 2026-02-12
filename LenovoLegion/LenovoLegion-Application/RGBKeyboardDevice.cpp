// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "RGBKeyboardDevice.h"
#include "RGBControllerKeyNames.h"
#include "ui_RGBKeyboardDevice.h"


#include <Core/LoggerHolder.h>


#include <QEvent>
#include <QMouseEvent>
#include <QAbstractItemView>
#include <QScreen>

namespace LenovoLegionGui {

QString RGBKeyboardDevice::ModeDescription(const LenovoLegionDaemon::mode& m)
{
    static const std::unordered_map<std::string, QString> descriptions =
    {
        {"Screw Rainbow",           tr("Screw Rainbow effect.")},
        {"Rainbow Wave",            tr("Rainbow Wave effect.")},
        {"Color Change",            tr("Color Change effect.")},
        {"Color Pulse",             tr("Color Pulse effect.")},
        {"Color Wave",              tr("Color Wave effect.")},
        {"Smooth",                  tr("Smooth effect.")},
        {"Rainbow Wave",            tr("Rainbow Wave effect.")},
        {"Rain",                    tr("Rain Flashes effect.")},
        {"Ripple",                  tr("Ripple Flashes effect.")},
        {"Always",                  tr("Always Flashes effect.")},
        {"Type Lighting",           tr("Type Lighting effect.")}
    };

    /*-----------------------------------------------------------------*\
    | Find the given mode name in the list and return the description   |
    | if it exists, otherwise return an empty string                    |
    \*-----------------------------------------------------------------*/
    std::unordered_map<std::string, QString>::const_iterator it = descriptions.find(m.name);

    if(it != descriptions.end())
    {
        return it->second;
    }

    return "";
}

RGBKeyboardDevice::RGBKeyboardDevice(RGBController *dev, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::RGBKeyboardDevice),
    device(dev)
{
    ui->setupUi(this);

    ui->LEDBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->LEDBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->ModeBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->ModeBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->DirectionBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->DirectionBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->ZoneBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->ZoneBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->ProfileBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->ProfileBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->DirectionEffectsBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->DirectionEffectsBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->LEDEffectsBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->LEDEffectsBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->ModeEffectsBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->ModeEffectsBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->SpeedEffectsComboBox->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->SpeedEffectsComboBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    /*
     * Detect mouse tracking for QListWidget (effects list)
     */
    ui->tableWidgetEffects->setMouseTracking(true);
    ui->tableWidgetEffects->viewport()->setMouseTracking(true);
    ui->tableWidgetEffects->viewport()->installEventFilter(this);
    
    /*
     * Prevent deselection when clicking on already selected item
     * Use ContiguousSelection which doesn't allow deselection by clicking the same item
     * Disable drag to prevent deselection when mouse moves outside while pressed
     */
    ui->tableWidgetEffects->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetEffects->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableWidgetEffects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetEffects->setDragEnabled(false);
    ui->tableWidgetEffects->setDragDropMode(QAbstractItemView::NoDragDrop);

    /*-----------------------------------------------------*\
    | Set up the device view                                |
    \*-----------------------------------------------------*/
    ui->DeviceViewBox->setController(device.get(),device->vendorId(),device->productId());

    connect(ui->DeviceViewBox, &DeviceView::selectionChanged, this, &RGBKeyboardDevice::on_DeviceViewBox_selectionChanged);

    /*-----------------------------------------------------*\
     | The profile selection  box                           |
    \*-----------------------------------------------------*/
    UpdateProfileUi();


    /*-----------------------------------------------------*\
     | Fill the brightness slider                           |
    \*-----------------------------------------------------*/
    UpdateBrightnessUi();

    /*-----------------------------------------------------*\
     | Fill the logo controls                              |
     \*-----------------------------------------------------*/
    UpdateLogoUi();

    /*-----------------------------------------------------*\
    | Update mode user interface elements                   |
    \*-----------------------------------------------------*/
    UpdateModeUi();


    /*-----------------------------------------------------*\
     | Update effect user interface elements                |
    \*-----------------------------------------------------*/
    UpdateTableEffectsUi();


    /*-----------------------------------------------------*\
    | Set initial color to black and update UI              |
    \*-----------------------------------------------------*/
    current_color.setRgb(0, 0, 0);
    updateColorUi();
}

RGBKeyboardDevice::~RGBKeyboardDevice()
{
    delete ui;
}

void RGBKeyboardDevice::dataProviderEvent(const legion::messages::Notification &notification)
{
    if(notification.has_action())
    {
        if(notification.action() == legion::messages::Notification::SPECIAL_KEY_PRESSED && notification.has_special_key())
        {
            switch (notification.special_key()) {
            case legion::messages::Notification::SPECTRUMBACKLIGHTOFF:
            case legion::messages::Notification::SPECTRUMBACKLIGHT1:
            case legion::messages::Notification::SPECTRUMBACKLIGHT2:
            case legion::messages::Notification::SPECTRUMBACKLIGHT3:
                device->RefreshData();
                UpdateBrightnessUi();
                break;

            case legion::messages::Notification::SPECTRUMPRESET1:
            case legion::messages::Notification::SPECTRUMPRESET2:
            case legion::messages::Notification::SPECTRUMPRESET3:
            case legion::messages::Notification::SPECTRUMPRESET4:
            case legion::messages::Notification::SPECTRUMPRESET5:
            case legion::messages::Notification::SPECTRUMPRESET6:
                device->RefreshData();
                UpdateProfileUi();
                break;
            case legion::messages::Notification::LOGOBACKLIGHTOFF:
            case legion::messages::Notification::LOGOBACKLIGHTON:
                device->RefreshData();
                UpdateLogoUi();
                break;
            default:
                break;
            }
        }
    }
}

void RGBKeyboardDevice::cleanup()
{
    ui->DeviceViewBox->cleanup();
}

void RGBKeyboardDevice::on_BrightnessSlider_valueChanged(int value)
{

    /*-----------------------------------------------------*\
    | Set device brightness                                 |
    \*-----------------------------------------------------*/
    device->SetBrightness((unsigned int)value);
    device->ApplyPendingChanges();
}

void RGBKeyboardDevice::UpdateProfileUi()
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * -----------------------------------------------------*/
    ui->ProfileBox->blockSignals(true);
    ui->ProfileBox->clear();
    ui->ProfileBox->setEnabled(false);

    auto profiles = device->GetProfiles();
    if(profiles.max > 0)
    {
        /*-----------------------------------------------------*\
        | Fill in the profile selection box                    |
        \*-----------------------------------------------------*/
        for (std::size_t i = profiles.min; i < profiles.max + 1; i++)
        {
            ui->ProfileBox->addItem(QString("Profile ").append(QString::number(i)),QVariant::fromValue(i));
        }

        ui->ProfileBox->setCurrentIndex(profiles.active - profiles.min);
    }

    /*
     * Dependencies
     */
    UpdateTableEffectsUi();

    /*-----------------------------------------------------*\
     *
     * Enable profile selection box if there are profiles
     * -----------------------------------------------------*/

    if (profiles.max > 0)
    {
        ui->ProfileBox->setEnabled(true);
        ui->ProfileBox->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateBrightnessUi()
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * -----------------------------------------------------*/
    ui->BrightnessSlider->blockSignals(true);
    ui->BrightnessSlider->setMinimum(0);
    ui->BrightnessSlider->setMaximum(0);
    ui->BrightnessSlider->setEnabled(false);

    /*
     * Set min, max, value, enable based on device
     */
    auto brightness = device->GetBrightness();
    if(brightness.min < brightness.max)
    {
        ui->BrightnessSlider->setMinimum(brightness.min);
        ui->BrightnessSlider->setMaximum(brightness.max);
        ui->BrightnessSlider->setValue(brightness.active);
    }


    /*
     * Dependencies
     */
    // N/A


    /*
     * Enable brightness slider if supported
     */
    if(brightness.min < brightness.max)
    {
        ui->BrightnessSlider->setEnabled(true);
        ui->BrightnessSlider->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateLogoUi()
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * -----------------------------------------------------*/
    ui->comboBox_logo->blockSignals(true);
    ui->comboBox_logo->clear();
    ui->comboBox_logo->setEnabled(false);
    ui->comboBox_logo->setVisible(false);
    ui->label_logo->setVisible(false);

    /*
     * Set
     */
    if(device->HasLogo())
    {
        ui->comboBox_logo->addItem(tr("On"), QVariant::fromValue(true));
        ui->comboBox_logo->addItem(tr("Off"), QVariant::fromValue(false));
        ui->comboBox_logo->setCurrentIndex(device->GetLogoState() ? 0 : 1);
        ui->comboBox_logo->setEnabled(true);
        ui->comboBox_logo->blockSignals(false);
        ui->comboBox_logo->setVisible(true);
        ui->label_logo->setVisible(true);
    }

    /*
     * Dependencies
     */
    // N/A

    if(device->HasLogo())
    {
        ui->comboBox_logo->setVisible(true);
        ui->label_logo->setVisible(true);
        ui->comboBox_logo->setEnabled(true);
        ui->comboBox_logo->blockSignals(false);
    }
}

void RGBKeyboardDevice::on_SwatchBox_swatchChanged(const QColor color)
{
    /*-----------------------------------------------------*\
    | Store the swatch color to the current color QColor    |
    \*-----------------------------------------------------*/
    current_color = color;

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_ColorWheelBox_colorChanged(const QColor color)
{
    /*-----------------------------------------------------*\
    | Store the wheel color to the current color QColor     |
    \*-----------------------------------------------------*/
    current_color = color;

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_RedSpinBox_valueChanged(int red)
{
    /*-----------------------------------------------------*\
    | Update the current color QColor red channel           |
    \*-----------------------------------------------------*/
    current_color.setRed(red);

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_HueSpinBox_valueChanged(int hue)
{
    /*-----------------------------------------------------*\
    | Read the saturation and value box values              |
    \*-----------------------------------------------------*/
    int sat = current_color.saturation();
    int val = current_color.value();

    /*-----------------------------------------------------*\
    | Update the current color QColor using HSV             |
    \*-----------------------------------------------------*/
    current_color.setHsv(hue, sat, val);

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_GreenSpinBox_valueChanged(int green)
{
    /*-----------------------------------------------------*\
    | Update the current color QColor green channel         |
    \*-----------------------------------------------------*/
    current_color.setGreen(green);

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_SatSpinBox_valueChanged(int sat)
{
    /*-----------------------------------------------------*\
    | Read the hue and value box values                     |
    \*-----------------------------------------------------*/
    int hue = current_color.hue();
    int val = current_color.value();

    /*-----------------------------------------------------*\
    | Update the current color QColor using HSV             |
    \*-----------------------------------------------------*/
    current_color.setHsv(hue, sat, val);

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_BlueSpinBox_valueChanged(int blue)
{
    /*-----------------------------------------------------*\
    | Update the current color QColor blue channel          |
    \*-----------------------------------------------------*/
    current_color.setBlue(blue);

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_ValSpinBox_valueChanged(int val)
{
    /*-----------------------------------------------------*\
    | Read the hue and saturation box values                |
    \*-----------------------------------------------------*/
    int hue = current_color.hue();
    int sat = current_color.saturation();

    /*-----------------------------------------------------*\
    | Update the current color QColor using HSV             |
    \*-----------------------------------------------------*/
    current_color.setHsv(hue, sat, val);

    /*-----------------------------------------------------*\
    | Update the color UI                                   |
    \*-----------------------------------------------------*/
    colorChanged();
}

void RGBKeyboardDevice::on_HexLineEdit_textChanged(const QString &arg1)
{
    /*-----------------------------------------------------*\
    | Make an editable copy of the string                   |
    \*-----------------------------------------------------*/
    QString temp = arg1;

    /*-----------------------------------------------------*\
    | Remove # character so that #XXXXXX color codes are    |
    | acceptable.  0xXXXXXX codes are already accepted by   |
    | toInt().  Convert into an RGBColor.  Mask off the     |
    | unused bits.                                          |
    \*-----------------------------------------------------*/
    LenovoLegionDaemon::RGBColor color = (LenovoLegionDaemon::RGBColor)(0x00FFFFFF & temp.replace("#", "").toInt(NULL, 16));

    /*-----------------------------------------------------*\
    | Store new color into the current color QColor         |
    | Because RGBColor stores color in BGR format, we have  |
    | to reverse the R and B channels if the hex format is  |
    | RGB.                                                  |
    \*-----------------------------------------------------*/
    if(HexFormatRGB)
    {
        current_color.setRed(RGBGetBValue(color));
        current_color.setGreen(RGBGetGValue(color));
        current_color.setBlue(RGBGetRValue(color));
    }
    else
    {
        current_color.setRed(RGBGetRValue(color));
        current_color.setGreen(RGBGetGValue(color));
        current_color.setBlue(RGBGetBValue(color));
    }

    /*-----------------------------------------------------*\
    | Update the color UI, but set the UpdateHex flag to    |
    | false so the hex edit box isn't updated while the user|
    | is in the middle of typing a value.                   |
    \*-----------------------------------------------------*/
    colorChanged(false);
}

void RGBKeyboardDevice::on_DeviceViewBox_selectionChanged(QVector<int> indices)
{
    UpdateLEDsUI(indices);

    UpdateAddButtomUi();
}

void RGBKeyboardDevice::ShowDeviceView()
{
    /*-----------------------------------------------------*\
     * Show device view                                     *
     * -----------------------------------------------------*/
    ui->DeviceViewBoxFrame->show();
}

void RGBKeyboardDevice::HideDeviceView()
{
    /*-----------------------------------------------------*\
    | Hide device view                                      |
    \*-----------------------------------------------------*/
    ui->DeviceViewBoxFrame->hide();
}

bool RGBKeyboardDevice::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->tableWidgetEffects->viewport()) {

        if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (mouseEvent && !ui->tableWidgetEffects->itemAt(mouseEvent->pos())) {
                ui->DeviceViewBox->markLeds({});
            }
            else
            {
                on_tableWidgetEffects_cellEntered(ui->tableWidgetEffects->indexAt(mouseEvent->pos()).row(), ui->tableWidgetEffects->indexAt(mouseEvent->pos()).column());
            }
            return true;
        }
        else if (event->type() == QEvent::Leave) {
            ui->DeviceViewBox->markLeds({});
        }
        else if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

            if (mouseEvent) {
                QModelIndex index = ui->tableWidgetEffects->indexAt(mouseEvent->pos());
                if (index.isValid() && index.column() == 1) {

                    device->RemoveEffect(static_cast<unsigned int>(index.row()));
                    device->ApplyPendingChanges();

                    UpdateTableEffectsUi();
                } else if (index.isValid() && index.column() == 0)
                {

                    /*
                     * Generate signals,Current Index changed event
                     * doesn't get generated when clicking the same item
                     */
                    ui->tableWidgetEffects->setCurrentCell(index.row(),index.column());
                    ui->tableWidgetEffects->item(index.row(), index.column())->setSelected(true);
               }
                return true;
            }
        }
        else if (event->type() == QEvent::MouseButtonDblClick) {
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

void RGBKeyboardDevice::UpdateZoneUi()
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * -----------------------------------------------------*/
    ui->ZoneBox->blockSignals(true);
    ui->ZoneBox->clear();
    ui->ZoneBox->setEnabled(false);


    /*
     * Dependencies
     */
    // N/A


    /*-----------------------------------------------------*\
     *
     * Fill in the zone selection box
     * ----------------------------------------------------*/
    const auto& zones = device->GetZones();
    for(const auto& zone : zones)
    {
        ui->ZoneBox->addItem(zone.name.c_str());
    }

    ui->ZoneBox->setCurrentIndex(ui->ZoneBox->count() > 0 ? 0 : -1);
    ui->ZoneBox->setEnabled(true);
}

void RGBKeyboardDevice::UpdateLEDsUI(const QVector<int>& indices)
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * -----------------------------------------------------*/
    ui->LEDBox->blockSignals(true);
    ui->LEDBox->clear();
    ui->LEDBox->setEnabled(false);



    /*
     * Dependencies
     */
    // N/A


    /*-----------------------------------------------------*\
     *
     * Fill in the LED selection box
     * ----------------------------------------------------*/
    for (const auto& led : indices)
    {
        ui->LEDBox->addItem(device->GetLEDName(led).c_str(),QVariant::fromValue(device->GetLEDs().at(led)));
    }

    ui->LEDBox->setEnabled(ui->LEDBox->count() > 0);
}

void RGBKeyboardDevice::UpdateModeUi(const int selectModeIdx)
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * ----------------------------------------------------*/
    ui->ModeBox->blockSignals(true);
    ui->ModeBox->clear();
    ui->ModeBox->setEnabled(false);
    ui->DeviceViewBox->setPerLED(false);

    /*
     * Fill in the mode selection box
     */
    const auto& modes = device->GetModes();
    for(std::size_t i = 0; i < modes.size(); i++)
    {
        ui->ModeBox->addItem(device->GetModeNameByIdx(i).c_str());
        ui->ModeBox->setItemData((int)i, ModeDescription(modes[i]), Qt::ToolTipRole);
    }
    ui->ModeBox->setCurrentIndex(ui->ModeBox->count() >selectModeIdx ? selectModeIdx : -1);

    /*
     * Dependencies
     */
    UpdateZoneUi();
    UpdateSpeedUi();
    UpdateDirectionUi();
    UpdatePerLEDUI();
    UpdateModeSpecificUi();
    UpdateRandomUi();

    /*
     * Device View
     */
    const auto& mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_PER_LED_SELECTION)
    {
        ui->DeviceViewBox->selectLeds([this](){
            QVector<int> indices;
            for (int i = 0; i < ui->LEDBox->count(); ++i) {

                auto led = ui->LEDBox->itemData(i).value<LenovoLegionDaemon::led>();

                for(size_t j = 0; j < device->GetLEDs().size(); ++j)
                {
                    if(device->GetLEDs().at(j).value == led.value)
                    {
                        indices.push_back((int)j);
                    }
                }
            }
            return indices;
        }());
        ui->DeviceViewBox->setPerLED(true);
    }
    else if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_ALL_LED_SELECTION)
    {
        ui->DeviceViewBox->selectLeds([this](){
            QVector<int> indices;

            for(size_t i = 0; i < device->GetLEDs().size(); ++i)
            {
                indices.push_back((int)i);
            }

            return indices;
        }());
    }
    else if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_ALL_KB_LED_SELECTION)
    {
        ui->DeviceViewBox->selectLeds([this](){
            QVector<int> indices;

            for(size_t i = 0; i < device->GetLEDs().size(); ++i)
            {
                if(LenovoLegionDaemon::KeyCodesToName.contains(device->GetLEDs().at(i).value))
                {
                    indices.push_back((int)i);
                }
            }

            return indices;
        }());
    }



    /*-----------------------------------------------------*\
     *
     * Enable mode selection box if there are modes
     * ----------------------------------------------------*/
    if(ui->ModeBox->count() > 0)
    {
        ui->ModeBox->setEnabled(true);
        ui->ModeBox->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdatePerLEDUI(const std::optional<bool> &enabled)
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * ----------------------------------------------------*/
    ui->PerLEDCheck->blockSignals(true);
    ui->PerLEDCheck->setAutoExclusive(false);
    ui->PerLEDCheck->setChecked(false);
    ui->PerLEDCheck->setEnabled(false);


    /*
     * Set checked based on mode
     */
    auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_PER_LED_COLOR)
    {
        ui->PerLEDCheck->setAutoExclusive(true);
        ui->PerLEDCheck->setChecked(enabled.has_value() ? enabled.value() : true);
    }


    /*
     * Dependencies
     */
    UpdateModeSpecificColorCountUi();
    UpdateColorFrameUi();


    /*-----------------------------------------------------*\
     *
     * Enable per LED checkbox if supported
     * ----------------------------------------------------*/
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_PER_LED_COLOR)
    {
        ui->PerLEDCheck->setEnabled(true);
        ui->PerLEDCheck->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateModeSpecificUi(const std::optional<bool> &enabled)
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * ----------------------------------------------------*/
    ui->ModeSpecificCheck->blockSignals(true);
    ui->ModeSpecificCheck->setAutoExclusive(false);
    ui->ModeSpecificCheck->setChecked(false);
    ui->ModeSpecificCheck->setEnabled(false);


    /*
     * Set checked based on mode
     */
    auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        ui->ModeSpecificCheck->setAutoExclusive(true);
        ui->ModeSpecificCheck->setChecked(enabled.has_value() ? enabled.value() : true);
    }
    /*
     * Dependencies
     */
    UpdateModeSpecificColorCountUi();
    UpdateColorFrameUi();

    /*-----------------------------------------------------*\
     *
     * Enable mode specific checkbox if supported
     * ----------------------------------------------------*/
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        ui->ModeSpecificCheck->setEnabled(true);
        ui->ModeSpecificCheck->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateRandomUi(const std::optional<bool> &enabled)
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * ----------------------------------------------------*/
    ui->RandomCheck->blockSignals(true);
    ui->RandomCheck->setAutoExclusive(false);
    ui->RandomCheck->setChecked(false);
    ui->RandomCheck->setEnabled(false);


    /*
     * Set checked based on mode
     */
    auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_RANDOM_COLOR)
    {
        ui->RandomCheck->setAutoExclusive(true);
        ui->RandomCheck->setChecked(enabled.has_value() ? enabled.value() : true);
    }

    /*
     * Dependencies
     */
    UpdateModeSpecificColorCountUi();
    UpdateColorFrameUi();

    /*-----------------------------------------------------*\
     *
     * Enable random checkbox if supported
     * ----------------------------------------------------*/
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_RANDOM_COLOR)
    {
        ui->RandomCheck->setEnabled(true);
        ui->RandomCheck->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateModeSpecificColorsUi(const int selectModeIdx)
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * -----------------------------------------------------*/
    ui->comboBox_modeSpecificColor->blockSignals(true);

    const std::vector<QVariant> currentColors = [this]{
        std::vector<QVariant> colors;

        for (int i = 0; i < ui->comboBox_modeSpecificColor->count(); ++i)
        {
            colors.push_back(ui->comboBox_modeSpecificColor->itemData(i).value<QColor>());
        }

        return colors;
    }();

    ui->comboBox_modeSpecificColor->clear();
    ui->comboBox_modeSpecificColor->setEnabled(false);


    /*
     * Set enabled based on mode
     */
    if(ui->spinBox_modeSpecificColorCount->maximum() > 0)
    {
        auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
        for(int i = 0; i < ui->spinBox_modeSpecificColorCount->value() ; ++i)
        {
            ui->comboBox_modeSpecificColor->addItem(QString::asprintf("Mode Color %u",i + 1),static_cast<size_t>(i) < currentColors.size() ? currentColors[i] : QVariant(QColor(QRgb(mode.colors.at(i)))));
        }

        if(selectModeIdx >= 0 && selectModeIdx <  ui->comboBox_modeSpecificColor->count())
        {
            ui->comboBox_modeSpecificColor->setCurrentIndex(selectModeIdx);
        }
        else
        {
             ui->comboBox_modeSpecificColor->setCurrentIndex(ui->comboBox_modeSpecificColor->count() - 1);
        }

        /*
         * Dependencies
         */
        current_color = ui->comboBox_modeSpecificColor->itemData(ui->comboBox_modeSpecificColor->currentIndex()).value<QColor>();
    }

    /*
     * Dependencies
     */
    updateColorUi();


    /*
     * Enable mode specific color combo box if there are colors
     */
    if(ui->comboBox_modeSpecificColor->count() > 0)
    {
        ui->comboBox_modeSpecificColor->setEnabled(true);
        ui->comboBox_modeSpecificColor->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateModeSpecificColorCountUi(const int value)
{
    /*-----------------------------------------------------*\
     *
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     * ----------------------------------------------------*/
    ui->spinBox_modeSpecificColorCount->blockSignals(true);
    ui->spinBox_modeSpecificColorCount->setMinimum(0);
    ui->spinBox_modeSpecificColorCount->setMaximum(0);
    ui->spinBox_modeSpecificColorCount->setValue(0);
    ui->spinBox_modeSpecificColorCount->setEnabled(false);



    /*-----------------------------------------------------*\
     *
     * Set min, max, value, enable based on mode
     * ----------------------------------------------------*/
    if(ui->ModeSpecificCheck->isChecked())
    {
        auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());

        ui->spinBox_modeSpecificColorCount->setMinimum(1);
        ui->spinBox_modeSpecificColorCount->setMaximum(mode.colors.size());

        if(value >= ui->spinBox_modeSpecificColorCount->minimum() && value <= ui->spinBox_modeSpecificColorCount->maximum())
        {
            ui->spinBox_modeSpecificColorCount->setValue(value);
        }
        else
        {
            ui->spinBox_modeSpecificColorCount->setValue(1);
        }
    }


    /*
     * Dependencies
     */
    UpdateModeSpecificColorsUi();


    /*
     * Enable mode specific color count spin box if there are colors
     */
    if(ui->spinBox_modeSpecificColorCount->maximum() > 0)
    {
        ui->spinBox_modeSpecificColorCount->setEnabled(true);
        ui->spinBox_modeSpecificColorCount->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateSpeedUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->SpeedSlider->blockSignals(true);
    ui->SpeedSlider->setEnabled(false);
    ui->SpeedSlider->setMinimum(0);
    ui->SpeedSlider->setMaximum(0);
    ui->SpeedSlider->setValue(0);


    /*
     * Set min, max, value, enable based on mode
     */
    auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_SPEED)
    {
        ui->SpeedSlider->setMinimum(mode.speed_min);
        ui->SpeedSlider->setMaximum(mode.speed_max);
        ui->SpeedSlider->setValue(mode.speed);
    }

    /*
     * Dependencies
     */
    // NA

    /*
     * Enable speed slider if supported
     */
    if(ui->SpeedSlider->minimum() < ui->SpeedSlider->maximum())
    {
        ui->SpeedSlider->setEnabled(true);
        ui->SpeedSlider->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateDirectionUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->DirectionBox->blockSignals(true);
    ui->DirectionBox->clear();
    ui->DirectionBox->setEnabled(false);

    /*
     * Set items, enable based on mode
     */
    auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_DIRECTION_LR)
    {
        ui->DirectionBox->addItem(LenovoLegionDaemon::modeDirectionToString(LenovoLegionDaemon::MODE_DIRECTION_LEFT));
        ui->DirectionBox->addItem(LenovoLegionDaemon::modeDirectionToString(LenovoLegionDaemon::MODE_DIRECTION_RIGHT));
    }
    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_DIRECTION_UD)
    {
        ui->DirectionBox->addItem(LenovoLegionDaemon::modeDirectionToString(LenovoLegionDaemon::MODE_DIRECTION_UP));
        ui->DirectionBox->addItem(LenovoLegionDaemon::modeDirectionToString(LenovoLegionDaemon::MODE_DIRECTION_DOWN));
    }

    if(mode.flags & LenovoLegionDaemon::MODE_FLAG_HAS_DIRECTION_SPINLR)
    {
        ui->DirectionBox->addItem(LenovoLegionDaemon::modeDirectionToString(LenovoLegionDaemon::MODE_DIRECTION_SPINLEFT));
        ui->DirectionBox->addItem(LenovoLegionDaemon::modeDirectionToString(LenovoLegionDaemon::MODE_DIRECTION_SPINRIGHT));
    }


    /*
     * Dependencies
     */
    // NA


    /*
     * Enable direction box if supported
     */
    if(ui->DirectionBox->count() > 0)
    {
        ui->DirectionBox->setEnabled(true);
    }
}

void RGBKeyboardDevice::UpdateAddButtomUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->pushButton_AddEffects->blockSignals(true);
    ui->pushButton_AddEffects->setEnabled(false);

    ui->label_maximumReached->setVisible(false);

    /*
     * Dependencies
     */
    //NA



    /*
     * Set enabled based on effect count
     */
    if(device->GetEffects().size() >= device->GetMaxEffects())
    {
        ui->label_maximumReached->setVisible(true);
    }


    if(device->GetEffects().size() < device->GetMaxEffects())
    {
        auto mode = device->GetModeByIdx(ui->ModeBox->currentIndex());
        if( (mode.flags  & LenovoLegionDaemon::MODE_FLAG_HAS_PER_LED_SELECTION)   ||
            (mode.flags  & LenovoLegionDaemon::MODE_FLAG_HAS_ALL_LED_SELECTION)    ||
            (mode.flags  & LenovoLegionDaemon::MODE_FLAG_HAS_ALL_KB_LED_SELECTION))
        {
            if(ui->LEDBox->count() > 0)
            {
                ui->pushButton_AddEffects->setEnabled(true);
                ui->pushButton_AddEffects->blockSignals(false);
            }
        }
        else
        {
            ui->pushButton_AddEffects->setEnabled(true);
            ui->pushButton_AddEffects->blockSignals(false);
        }
    }

}

void RGBKeyboardDevice::UpdateColorFrameUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->ColorFrame->blockSignals(true);
    ui->ColorFrame->setVisible(false);


    /*
     * Dependencies
     */
    //NA


    /*
     * Enable that have data
     */

    if( ui->PerLEDCheck->isChecked() || ui->ModeSpecificCheck->isChecked())
    {
        ui->ColorFrame->setVisible(true);
        ui->ColorFrame->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateClearAllEffectsButtomUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->pushButton_EffectsClearAll->blockSignals(true);
    ui->pushButton_EffectsClearAll->setEnabled(false);

    /*
     * Dependencies
     */
    //N/A


    /*
     * Set enabled based on effect count
     */
    if(device->GetEffects().size() > 0)
    {
        ui->pushButton_EffectsClearAll->setEnabled(true);
        ui->pushButton_EffectsClearAll->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateResetEffectsButtomUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->pushButton_EffectsDefault->blockSignals(true);
    ui->pushButton_EffectsDefault->setEnabled(false);

    /*
     * Dependencies
     */
    //N/A


    /*
     * Set enabled based on effect count
     */
    ui->pushButton_EffectsDefault->setEnabled(true);
    ui->pushButton_EffectsDefault->blockSignals(false);
}

void RGBKeyboardDevice::UpdateEffectColorsUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->comboBox_EffectsColors->blockSignals(true);
    ui->comboBox_EffectsColors->clear();
    ui->comboBox_EffectsColors->setEnabled(false);


    if(!ui->tableWidgetEffects->selectedItems().empty())
    {
        auto effect = device->GetEffect(ui->tableWidgetEffects->selectedItems().first()->row());

        for (const auto& color : effect.m_colors)
        {
            ui->comboBox_EffectsColors->addItem(QString().asprintf("R:%02X G:%02X B:%02X", RGBGetRValue(color),RGBGetGValue(color),RGBGetBValue(color)));
        }
    }

    /*
     * Dependencies
     */
    //NA


    /*
     * Enable that have data
     */
    if(ui->comboBox_EffectsColors->count() > 0 )
    {
        ui->comboBox_EffectsColors->setEnabled(true);
        ui->comboBox_EffectsColors->blockSignals(false);
    }
}

void RGBKeyboardDevice::UpdateEffectUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */
    ui->SelectedEffectsBox->blockSignals(true);
    ui->LEDEffectsBox->blockSignals(true);
    ui->ModeEffectsBox->blockSignals(true);
    ui->RandomEffectsCheck->blockSignals(true);
    ui->ModeSpecificEffectsCheck->blockSignals(true);
    ui->PerLEDEffectsCheck->blockSignals(true);
    ui->SpeedEffectsComboBox->blockSignals(true);
    ui->DirectionEffectsBox->blockSignals(true);


    ui->SelectedEffectsBox->setEnabled(false);
    ui->LEDEffectsBox->setEnabled(false);
    ui->ModeEffectsBox->setEnabled(false);
    ui->RandomEffectsCheck->setEnabled(false);
    ui->ModeSpecificEffectsCheck->setEnabled(false);
    ui->PerLEDEffectsCheck->setEnabled(false);
    ui->SpeedEffectsComboBox->setEnabled(false);
    ui->DirectionEffectsBox->setEnabled(false);


    ui->SelectedEffectsBox->clear();
    ui->LEDEffectsBox->clear();
    ui->ModeEffectsBox->clear();
    ui->RandomEffectsCheck->setAutoExclusive(false);
    ui->RandomEffectsCheck->setChecked(false);
    ui->ModeSpecificEffectsCheck->setAutoExclusive(false);
    ui->ModeSpecificEffectsCheck->setChecked(false);
    ui->PerLEDEffectsCheck->setAutoExclusive(false);
    ui->PerLEDEffectsCheck->setChecked(false);
    ui->SpeedEffectsComboBox->clear();
    ui->DirectionEffectsBox->clear();


    if(!ui->tableWidgetEffects->selectedItems().empty())
    {
        auto effect = device->GetEffect(ui->tableWidgetEffects->selectedItems().first()->row());

        if(effect.m_color_mode == LenovoLegionDaemon::MODE_COLORS_RANDOM)
        {
            ui->RandomEffectsCheck->setChecked(true);
        }

        if(effect.m_color_mode == LenovoLegionDaemon::MODE_COLORS_PER_LED)
        {
            ui->PerLEDEffectsCheck->setChecked(true);
        }

        if(effect.m_color_mode == LenovoLegionDaemon::MODE_COLORS_MODE_SPECIFIC)
        {
            ui->ModeSpecificEffectsCheck->setChecked(true);
        }

        ui->SelectedEffectsBox->addItem(QString::number(ui->tableWidgetEffects->selectedItems().first()->row()) + " - " + device->GetModeByModeValue(effect.m_mode).name.c_str(),QVariant::fromValue(ui->tableWidgetEffects->selectedItems().first()->row()));

        for (const auto& led_effect : effect.m_leds)
        {
            ui->LEDEffectsBox->addItem(QString(led_effect.name.c_str()));
        }

        ui->DirectionEffectsBox->addItem(LenovoLegionDaemon::modeDirectionToString(LenovoLegionDaemon::valueToModeDirection(effect.m_direction)));


        ui->ModeEffectsBox->addItem(device->GetModeByModeValue(effect.m_mode).name.c_str());

        ui->SpeedEffectsComboBox->addItem(QString::number(effect.m_speed));
    }


    /*
     * Dependencies
     */
    UpdateEffectColorsUi();


    /*
     * Enable that have data
     */
    if(ui->LEDEffectsBox->count() > 0 )
    {
        ui->LEDEffectsBox->setEnabled(true);
    }
}

void RGBKeyboardDevice::UpdateTableEffectsUi()
{
    /*
     * Block signals to avoid recursive updates
     * Clear all controls initially
     * Set enabled false initially
     */

    ui->tableWidgetEffects->blockSignals(true);
    ui->tableWidgetEffects->setEnabled(false);
    ui->tableWidgetEffects->clear();
    ui->tableWidgetEffects->setRowCount(0);


    const auto& effects = device->GetEffects();
    for (size_t i = 0; i < effects.size(); ++i)
    {
        ui->tableWidgetEffects->insertRow(i);
        ui->tableWidgetEffects->setItem(i,0,new QTableWidgetItem(QString::number(i) + " - " + device->GetModeByModeValue(effects.at(i).m_mode).name.c_str()));
        ui->tableWidgetEffects->setItem(i,1,new QTableWidgetItem(QIcon(":/images/icons/cross.png"),""));
    }
    // First column stretches, others fit content
    ui->tableWidgetEffects->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidgetEffects->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->tableWidgetEffects->setColumnWidth(1,20);

    /*
     * Dependencies
     */
    UpdateEffectUi();
    UpdateAddButtomUi();
    UpdateClearAllEffectsButtomUi();
    UpdateResetEffectsButtomUi();


    /*
     * Enable that have data
     */
    if(ui->tableWidgetEffects->rowCount() > 0 )
    {
        ui->tableWidgetEffects->setEnabled(true);
        ui->tableWidgetEffects->blockSignals(false);
    }

}

void RGBKeyboardDevice::on_pushButton_AddEffects_clicked()
{
    device->AddEffect([this](){
        LenovoLegionDaemon::led_group_effect effect;

        effect.m_mode       = device->GetModeByIdx(ui->ModeBox->currentIndex()).value;
        effect.m_speed      = ui->SpeedSlider->isEnabled() ? ui->SpeedSlider->value() : 0;
        effect.m_direction  = LenovoLegionDaemon::stringToModeDirection(ui->DirectionBox->currentText());
        effect.m_color_mode = LenovoLegionDaemon::MODE_COLORS_NONE;

        if(ui->PerLEDCheck->isChecked())
        {
            effect.m_color_mode = LenovoLegionDaemon::MODE_COLORS_PER_LED;
            effect.m_colors.push_back(ToRGBColor(current_color.red(), current_color.green(), current_color.blue()));
        }

        if(ui->ModeSpecificCheck->isChecked())
        {
            effect.m_color_mode = LenovoLegionDaemon::MODE_COLORS_MODE_SPECIFIC;

            for (int i = 0; i < ui->comboBox_modeSpecificColor->count(); ++i)
            {
                QColor color = ui->comboBox_modeSpecificColor->itemData(i).value<QColor>();
                effect.m_colors.push_back(ToRGBColor(color.red(), color.green(), color.blue()));
            }
        }

        if(ui->RandomCheck->isChecked())
        {
            effect.m_color_mode = LenovoLegionDaemon::MODE_COLORS_RANDOM;
        }

        for (int i = 0; i < ui->LEDBox->count(); ++i)
        {
            effect.m_leds.push_back(ui->LEDBox->itemData(i).value<LenovoLegionDaemon::led>());
        }

        return effect;
    }());

    device->ApplyPendingChanges();

    UpdateTableEffectsUi();
}

void RGBKeyboardDevice::colorChanged(const bool UpdateHex)
{
    updateColorUi(UpdateHex);

    UpdatePerLEDUI(ui->PerLEDCheck->isChecked());
    UpdateModeSpecificUi(ui->ModeSpecificCheck->isChecked());
}

void RGBKeyboardDevice::updateColorUi(const bool UpdateHex)
{
    /*-----------------------------------------------------*\
    | Update colorwheel                                     |
    \*-----------------------------------------------------*/
    ui->ColorWheelBox->blockSignals(true);
    ui->ColorWheelBox->setColor(current_color);
    ui->ColorWheelBox->blockSignals(false);

    /*-----------------------------------------------------*\
    | Update RGB spinboxes                                  |
    \*-----------------------------------------------------*/
    ui->RedSpinBox->blockSignals(true);
    ui->RedSpinBox->setValue(current_color.red());
    ui->RedSpinBox->blockSignals(false);

    ui->GreenSpinBox->blockSignals(true);
    ui->GreenSpinBox->setValue(current_color.green());
    ui->GreenSpinBox->blockSignals(false);

    ui->BlueSpinBox->blockSignals(true);
    ui->BlueSpinBox->setValue(current_color.blue());
    ui->BlueSpinBox->blockSignals(false);

    /*-----------------------------------------------------*\
    | Update HSV spinboxes                                  |
    \*-----------------------------------------------------*/
    ui->HueSpinBox->blockSignals(true);
    ui->HueSpinBox->setValue(current_color.hue());
    ui->HueSpinBox->blockSignals(false);

    ui->SatSpinBox->blockSignals(true);
    ui->SatSpinBox->setValue(current_color.saturation());
    ui->SatSpinBox->blockSignals(false);

    ui->ValSpinBox->blockSignals(true);
    ui->ValSpinBox->setValue(current_color.value());
    ui->ValSpinBox->blockSignals(false);

    /*-----------------------------------------------------*\
    | Update Hex edit box                                   |
    \*-----------------------------------------------------*/
    if(UpdateHex)
    {
        LenovoLegionDaemon::RGBColor color = (0x00FFFFFF & current_color.rgb());

        /*-------------------------------------------------*\
        | If the hex format is BGR, swap R and B before     |
        | displaying as hex                                 |
        \*-------------------------------------------------*/
        if(!HexFormatRGB)
        {
            color = RGBGetRValue(color) << 16
                  | RGBGetGValue(color) << 8
                  | RGBGetBValue(color);
        }

        ui->HexLineEdit->blockSignals(true);
        ui->HexLineEdit->setText(QString().asprintf("%06X", color));
        ui->HexLineEdit->blockSignals(false);
    }
}

void RGBKeyboardDevice::on_ProfileBox_currentIndexChanged(int index)
{
    /*-----------------------------------------------------*\
    | Change device profile                                    |
    \*-----------------------------------------------------*/
    device->SetProfile(ui->ProfileBox->itemData(index).toUInt());

    device->ApplyPendingChanges();

    UpdateProfileUi();
}

void RGBKeyboardDevice::on_pushButtonToggleLEDView_clicked()
{
    if(!ui->DeviceViewBoxFrame->isHidden())
    {
        HideDeviceView();
    }
    else
    {
        ShowDeviceView();
    }
}

void RGBKeyboardDevice::on_pushButton_EffectsClearAll_clicked()
{
    device->ClearEffects();
    device->ApplyPendingChanges();

    UpdateTableEffectsUi();
}

void RGBKeyboardDevice::on_ModeBox_currentIndexChanged(int index)
{
    UpdateModeUi(index);
}

void RGBKeyboardDevice::on_PerLEDCheck_clicked()
{
    UpdatePerLEDUI(ui->PerLEDCheck->isChecked());
}

void RGBKeyboardDevice::on_ModeSpecificCheck_clicked()
{
    UpdateModeSpecificUi(ui->ModeSpecificCheck->isChecked());
}

void RGBKeyboardDevice::on_RandomCheck_clicked()
{
    UpdateRandomUi(ui->RandomCheck->isChecked());
}

void RGBKeyboardDevice::on_comboBox_modeSpecificColor_currentIndexChanged(int index)
{
    UpdateModeSpecificColorsUi(index);
}

void RGBKeyboardDevice::on_comboBox_EffectsColors_currentIndexChanged(int)
{
    UpdateEffectColorsUi();
}

void RGBKeyboardDevice::on_pushButton_EffectsDefault_clicked()
{
    device->ResetEffectsToDefault();
    device->ApplyPendingChanges();

    UpdateTableEffectsUi();
}

void RGBKeyboardDevice::on_spinBox_modeSpecificColorCount_valueChanged(int value)
{
    UpdateModeSpecificColorCountUi(value);
}


void RGBKeyboardDevice::on_tableWidgetEffects_currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)
{
    UpdateEffectUi();
}

void RGBKeyboardDevice::on_tableWidgetEffects_cellEntered(int row,int)
{
    auto effect = device->GetEffect(row);

    ui->DeviceViewBox->markLeds([&effect,this](){
        QMap<int,QColor> indices;
        for (const auto& led_effect : effect.m_leds)
        {
            auto ledsIdx = device->GetLedsIndexesByDeviceSpecificValue(led_effect.value);

            for (auto idx : ledsIdx) {
                int colorIdx = ui->comboBox_modeSpecificColor->currentIndex();

                if(colorIdx > 0 && static_cast<size_t>(colorIdx) < effect.m_colors.size())
                {
                    indices[idx] = QColor::fromRgb(RGBGetRValue(effect.m_colors[colorIdx]),RGBGetGValue(effect.m_colors[colorIdx]),RGBGetBValue(effect.m_colors[colorIdx]));
                }
                else
                {
                    indices[idx] = QColor::fromRgb(53,87,0xBB);
                }
            }
        }
        return indices;
    }());

}

void RGBKeyboardDevice::on_comboBox_logo_currentIndexChanged(int index)
{
    /*-----------------------------------------------------*\
    | Set device brightness                                 |
    \*-----------------------------------------------------*/
    device->SetLogoState(ui->comboBox_logo->itemData(index).toBool());
    device->ApplyPendingChanges();
}


}
