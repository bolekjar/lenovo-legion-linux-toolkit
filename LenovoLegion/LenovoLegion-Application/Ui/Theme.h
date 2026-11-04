// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../Settings.h"

#include <QColor>
#include <QObject>

namespace LenovoLegionGui {

/*
 * Colors of the application theme for QML pages (UiDesign.md). QML does not use the Qt Widgets stylesheets
 * (ToolBarSettingsWidget::getStylesheetTheme), so the main colors of every stylesheet are here.
 * NoTheme takes the colors from the system palette, the same as the widgets without a stylesheet.
 */
class Theme : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool    styled           READ styled           NOTIFY changed)   // false = NoTheme, system look
    Q_PROPERTY(QColor  window           READ window           NOTIFY changed)
    Q_PROPERTY(QColor  text             READ text             NOTIFY changed)
    Q_PROPERTY(QColor  base             READ base             NOTIFY changed)   // background of inputs
    Q_PROPERTY(QColor  groupBackground  READ groupBackground  NOTIFY changed)
    Q_PROPERTY(QColor  groupBorder      READ groupBorder      NOTIFY changed)
    Q_PROPERTY(QColor  titleBackground  READ titleBackground  NOTIFY changed)
    Q_PROPERTY(QColor  titleText        READ titleText        NOTIFY changed)
    Q_PROPERTY(QColor  accent           READ accent           NOTIFY changed)   // borders of inputs and buttons, selection
    Q_PROPERTY(QColor  accentText       READ accentText       NOTIFY changed)   // text on the accent color
    Q_PROPERTY(QColor  buttonBackground READ buttonBackground NOTIFY changed)
    Q_PROPERTY(QColor  buttonText       READ buttonText       NOTIFY changed)
    Q_PROPERTY(QColor  disabledBackground READ disabledBackground NOTIFY changed)
    Q_PROPERTY(QColor  disabledText     READ disabledText     NOTIFY changed)
    Q_PROPERTY(QColor  accentHover      READ accentHover      NOTIFY changed)   // border of inputs on hover and focus
    Q_PROPERTY(QColor  inputDisabledBackground READ inputDisabledBackground NOTIFY changed)
    Q_PROPERTY(QColor  inputDisabledBorder     READ inputDisabledBorder     NOTIFY changed)
    Q_PROPERTY(QColor  popupBackground  READ popupBackground  NOTIFY changed)   // list of a combo box
    Q_PROPERTY(QColor  itemHoverBackground READ itemHoverBackground NOTIFY changed)
    Q_PROPERTY(QColor  itemHoverText    READ itemHoverText    NOTIFY changed)
    Q_PROPERTY(QColor  accentPressed READ accentPressed NOTIFY changed)   // accent of a pressed control
    Q_PROPERTY(QColor  trackBackground READ trackBackground NOTIFY changed)   // track of a scroll bar
    Q_PROPERTY(QColor  sliderGroove READ sliderGroove NOTIFY changed)   // groove of a slider
    Q_PROPERTY(QColor  sliderHandle READ sliderHandle NOTIFY changed)   // handle of a slider
    Q_PROPERTY(QColor  tabBackground READ tabBackground NOTIFY changed)   // not selected tab
    Q_PROPERTY(QColor  paneBorder    READ paneBorder    NOTIFY changed)   // border of the tab widget (QTabWidget::pane)
    Q_PROPERTY(QColor  tabText READ tabText NOTIFY changed)   // text of a not selected tab
    Q_PROPERTY(QColor  listBackground READ listBackground NOTIFY changed)   // list
    Q_PROPERTY(QColor  listItemBackground READ listItemBackground NOTIFY changed)   // item of a list
    Q_PROPERTY(QColor  listItemBorder READ listItemBorder NOTIFY changed)   // border of an item of a list
    Q_PROPERTY(QColor  listItemHover READ listItemHover NOTIFY changed)   // item of a list under the mouse
    Q_PROPERTY(QColor  listItemSelected READ listItemSelected NOTIFY changed)   // selected item of a list
    Q_PROPERTY(QColor  listText READ listText NOTIFY changed)   // text of a list
    Q_PROPERTY(QColor  listSelectedText READ listSelectedText NOTIFY changed)   // text of a selected item
    Q_PROPERTY(QColor  buttonChecked    READ buttonChecked    NOTIFY changed)   // checked push button (QPushButton:checked)
    Q_PROPERTY(QColor  modified         READ modified         CONSTANT)         // label of a changed value not applied yet
    Q_PROPERTY(int     radius           READ radius           NOTIFY changed)
    Q_PROPERTY(int     inputRadius      READ inputRadius      NOTIFY changed)
    Q_PROPERTY(int     fontPixelSize    READ fontPixelSize    NOTIFY changed)

public:

    explicit Theme(ApplicationSettings::ThemeType theme,QObject* parent = nullptr);

    void setTheme(ApplicationSettings::ThemeType theme);

    bool   styled() const             { return m_styled; }
    QColor window() const             { return m_window; }
    QColor text() const               { return m_text; }
    QColor base() const               { return m_base; }
    QColor groupBackground() const    { return m_groupBackground; }
    QColor groupBorder() const        { return m_groupBorder; }
    QColor titleBackground() const    { return m_titleBackground; }
    QColor titleText() const          { return m_titleText; }
    QColor accent() const             { return m_accent; }
    QColor accentText() const         { return m_accentText; }
    QColor buttonBackground() const   { return m_buttonBackground; }
    QColor buttonText() const         { return m_buttonText; }
    QColor disabledBackground() const { return m_disabledBackground; }
    QColor disabledText() const       { return m_disabledText; }
    QColor accentHover() const        { return m_accentHover; }
    QColor inputDisabledBackground() const { return m_inputDisabledBackground; }
    QColor inputDisabledBorder() const     { return m_inputDisabledBorder; }
    QColor buttonChecked() const      { return m_buttonChecked; }
    QColor popupBackground() const    { return m_popupBackground; }
    QColor itemHoverBackground() const { return m_itemHoverBackground; }
    QColor itemHoverText() const      { return m_itemHoverText; }
    QColor accentPressed() const { return m_accentPressed; }
    QColor trackBackground() const { return m_trackBackground; }
    QColor sliderGroove() const { return m_sliderGroove; }
    QColor sliderHandle() const { return m_sliderHandle; }
    QColor tabBackground() const { return m_tabBackground; }
    QColor paneBorder() const { return m_paneBorder; }
    QColor tabText() const { return m_tabText; }
    QColor listBackground() const { return m_listBackground; }
    QColor listItemBackground() const { return m_listItemBackground; }
    QColor listItemBorder() const { return m_listItemBorder; }
    QColor listItemHover() const { return m_listItemHover; }
    QColor listItemSelected() const { return m_listItemSelected; }
    QColor listText() const { return m_listText; }
    QColor listSelectedText() const { return m_listSelectedText; }
    /*
     * The same color as MainWindow::VALUE_CHANGE_COLOR of the widget pages, for all themes
     */
    QColor modified() const           { return QColor("#995e5e"); }
    int    radius() const             { return m_radius; }
    int    inputRadius() const        { return m_inputRadius; }
    int    fontPixelSize() const      { return m_fontPixelSize; }

signals:

    void changed();

private:

    bool   m_styled;
    QColor m_window;
    QColor m_text;
    QColor m_base;
    QColor m_groupBackground;
    QColor m_groupBorder;
    QColor m_titleBackground;
    QColor m_titleText;
    QColor m_accent;
    QColor m_accentText;
    QColor m_buttonBackground;
    QColor m_buttonText;
    QColor m_disabledBackground;
    QColor m_disabledText;
    QColor m_accentHover;
    QColor m_inputDisabledBackground;
    QColor m_inputDisabledBorder;
    QColor m_buttonChecked;
    QColor m_popupBackground;
    QColor m_itemHoverBackground;
    QColor m_itemHoverText;
    QColor m_accentPressed;
    QColor m_trackBackground;
    QColor m_sliderGroove;
    QColor m_sliderHandle;
    QColor m_tabBackground;
    QColor m_paneBorder;
    QColor m_tabText;
    QColor m_listBackground;
    QColor m_listItemBackground;
    QColor m_listItemBorder;
    QColor m_listItemHover;
    QColor m_listItemSelected;
    QColor m_listText;
    QColor m_listSelectedText;
    int    m_radius;
    int    m_inputRadius;
    int    m_fontPixelSize;
};

}
