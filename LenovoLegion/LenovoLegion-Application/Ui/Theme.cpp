// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Theme.h"

#include <QFont>
#include <QGuiApplication>
#include <QPalette>

namespace LenovoLegionGui {

Theme::Theme(ApplicationSettings::ThemeType theme, QObject *parent) :
    QObject(parent)
{
    setTheme(theme);
}

void Theme::setTheme(ApplicationSettings::ThemeType theme)
{
    /*
     * Colors of the stylesheets in ToolBarSettingsWidget (QWidget, QGroupBox, QGroupBox::title, QPushButton, QSpinBox, ...)
     */
    m_styled        = theme != ApplicationSettings::ThemeType::NoTheme;
    m_radius        = 12;
    m_inputRadius   = 10;
    m_fontPixelSize = 13;

    switch(theme)
    {
    case ApplicationSettings::ThemeType::WhiteTheme:
        m_window             = QColor("#ffffff");
        m_text               = QColor("#2d2d30");
        m_base               = QColor("#ffffff");
        m_groupBackground    = QColor("#fafafa");
        m_groupBorder        = QColor("#e0e0e0");
        m_titleBackground    = QColor("#007acc");
        m_titleText          = QColor("#ffffff");
        m_accent             = QColor("#007acc");
        m_accentText         = QColor("#ffffff");
        m_buttonBackground   = QColor("#ffffff");
        m_buttonText         = QColor("#007acc");
        m_disabledBackground = QColor("#e0e0e0");
        m_disabledText       = QColor("#999999");
        m_accentHover             = QColor("#00a8ff");
        m_inputDisabledBackground = QColor("#f0f0f0");
        m_inputDisabledBorder     = QColor("#d0d0d0");
        m_buttonChecked           = QColor("#007acc");
        m_popupBackground         = QColor("#ffffff");
        m_itemHoverBackground     = QColor("#f0f0f0");
        m_itemHoverText           = QColor("#007acc");
        m_accentPressed             = QColor("#005a9e");
        m_trackBackground           = QColor("#f5f5f5");
        m_sliderGroove              = QColor("#e0e0e0");
        m_sliderHandle              = QColor("#ffffff");
        m_tabBackground             = QColor("#f0f0f0");
        m_paneBorder                = QColor("#e0e0e0");
        m_tabText                   = QColor("#2d2d30");
        m_listBackground            = QColor("#dadada");
        m_listItemBackground        = QColor("#f5f5f5");
        m_listItemBorder            = QColor("#fbfbfb");
        m_listItemHover             = QColor("#e8e8e8");
        m_listItemSelected          = QColor("#d8d8d8");
        m_listText                  = QColor("#000000");
        m_listSelectedText          = QColor("#000000");
        break;

    case ApplicationSettings::ThemeType::GrayTheme:
        m_window             = QColor("#808080");
        m_text               = QColor("#e0e0e0");
        m_base               = QColor("#808080");
        m_groupBackground    = QColor("#787878");
        m_groupBorder        = QColor("#606060");
        m_titleBackground    = QColor("#3c6844");
        m_titleText          = QColor("#808080");
        m_accent             = QColor("#3c6844");
        m_accentText         = QColor("#808080");
        m_buttonBackground   = QColor("#808080");
        m_buttonText         = QColor("#3c6844");
        m_disabledBackground = QColor("#606060");
        m_disabledText       = QColor("#a0a0a0");
        m_accentHover             = QColor("#4d8a5a");
        m_inputDisabledBackground = QColor("#686868");
        m_inputDisabledBorder     = QColor("#505050");
        m_buttonChecked           = QColor("#3c6844");
        m_popupBackground         = QColor("#787878");
        m_itemHoverBackground     = QColor("#686868");
        m_itemHoverText           = QColor("#3c6844");
        m_accentPressed             = QColor("#2d4f33");
        m_trackBackground           = QColor("#707070");
        m_sliderGroove              = QColor("#606060");
        m_sliderHandle              = QColor("#808080");
        m_tabBackground             = QColor("#686868");
        m_paneBorder                = QColor("#606060");
        m_tabText                   = QColor("#e0e0e0");
        m_listBackground            = QColor("#808080");
        m_listItemBackground        = QColor("#585858");
        m_listItemBorder            = QColor("#585858");
        m_listItemHover             = QColor("#474747");
        m_listItemSelected          = QColor("#434343");
        m_listText                  = QColor("#ffffff");
        m_listSelectedText          = QColor("#ffffff");
        break;

    case ApplicationSettings::ThemeType::DarkTheme:
        m_window             = QColor("#2d2d30");
        m_text               = QColor("#e0e0e0");
        m_base               = QColor("#3a3a3d");
        m_groupBackground    = QColor("#3a3a3d");
        m_groupBorder        = QColor("#3e3e42");
        m_titleBackground    = QColor("#ff8800");
        m_titleText          = QColor("#ffffff");
        m_accent             = QColor("#ff8800");
        m_accentText         = QColor("#ffffff");
        m_buttonBackground   = QColor("#ff8800");
        m_buttonText         = QColor("#ffffff");
        m_disabledBackground = QColor("#3e3e42");
        m_disabledText       = QColor("#6e6e73");
        m_accentHover             = QColor("#ffaa33");
        m_inputDisabledBackground = QColor("#252526");
        m_inputDisabledBorder     = QColor("#3e3e42");
        m_buttonChecked           = QColor("#ffaa33");
        m_popupBackground         = QColor("#2a2a2d");
        m_itemHoverBackground     = QColor("#3e3e42");
        m_itemHoverText           = QColor("#ffffff");
        m_accentPressed             = QColor("#cc6600");
        m_trackBackground           = QColor("#1e1e1e");
        m_sliderGroove              = QColor("#3e3e42");
        m_sliderHandle              = QColor("#e0e0e0");
        m_tabBackground             = QColor("#252526");
        m_paneBorder                = QColor("#ff8800");
        m_tabText                   = QColor("#b0b0b0");
        m_listBackground            = QColor("#3a3b3a");
        m_listItemBackground        = QColor("#313131");
        m_listItemBorder            = QColor("#454545");
        m_listItemHover             = QColor("#474747");
        m_listItemSelected          = QColor("#434343");
        m_listText                  = QColor("#d5d5d5");
        m_listSelectedText          = QColor("#ffffff");
        break;

    case ApplicationSettings::ThemeType::NoTheme:
    default:
    {
        /*
         * System look, the same as the widgets without a stylesheet
         */
        const QPalette palette = QGuiApplication::palette();

        /*
         * Border visible on light and dark system palettes (Mid is close to the background on dark palettes)
         */
        auto mix = [](const QColor& a,const QColor& b,double ratio) {
            return QColor::fromRgbF(a.redF()   * ratio + b.redF()   * (1.0 - ratio),
                                    a.greenF() * ratio + b.greenF() * (1.0 - ratio),
                                    a.blueF()  * ratio + b.blueF()  * (1.0 - ratio));
        };

        const QColor border = mix(palette.color(QPalette::Text),palette.color(QPalette::Base),0.35);

        m_window             = palette.color(QPalette::Window);
        m_text               = palette.color(QPalette::WindowText);
        m_base               = palette.color(QPalette::Base);
        m_groupBackground    = palette.color(QPalette::Window);
        m_groupBorder        = border;
        m_titleBackground    = palette.color(QPalette::Window);
        m_titleText          = palette.color(QPalette::WindowText);
        m_accent             = palette.color(QPalette::Highlight);
        m_accentText         = palette.color(QPalette::HighlightedText);
        m_buttonBackground   = palette.color(QPalette::Button);
        m_buttonText         = palette.color(QPalette::ButtonText);
        m_disabledBackground = palette.color(QPalette::Disabled,QPalette::Button);
        m_disabledText       = palette.color(QPalette::Disabled,QPalette::WindowText);
        m_accentHover             = palette.color(QPalette::Highlight);
        m_inputDisabledBackground = palette.color(QPalette::Disabled,QPalette::Base);
        m_inputDisabledBorder     = mix(palette.color(QPalette::Text),palette.color(QPalette::Base),0.2);
        m_buttonChecked           = palette.color(QPalette::Highlight);
        m_popupBackground         = palette.color(QPalette::Base);
        m_itemHoverBackground     = palette.color(QPalette::Midlight);
        m_itemHoverText           = palette.color(QPalette::Text);
        m_accentPressed             = palette.color(QPalette::Highlight).darker(130);
        m_trackBackground           = palette.color(QPalette::Window).darker(110);
        m_sliderGroove              = palette.color(QPalette::Mid);
        m_sliderHandle              = palette.color(QPalette::Button);
        m_tabBackground             = palette.color(QPalette::Button);
        m_paneBorder                = m_groupBorder;
        m_tabText                   = palette.color(QPalette::ButtonText);
        m_listBackground            = palette.color(QPalette::Base);
        m_listItemBackground        = palette.color(QPalette::Base);
        m_listItemBorder            = border;
        m_listItemHover             = palette.color(QPalette::Midlight);
        m_listItemSelected          = palette.color(QPalette::Highlight);
        m_listText                  = palette.color(QPalette::Text);
        m_listSelectedText          = palette.color(QPalette::HighlightedText);
        m_radius             = 4;
        m_inputRadius        = 2;
        m_fontPixelSize      = QGuiApplication::font().pixelSize() > 0 ? QGuiApplication::font().pixelSize() : 13;
        break;
    }
    }

    emit changed();
}

}
