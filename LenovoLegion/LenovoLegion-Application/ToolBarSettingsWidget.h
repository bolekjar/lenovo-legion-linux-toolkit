// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ToolBarWidget.h"
#include "Settings.h"

namespace Ui {
class ToolBarSettingsWidget;
}


namespace LenovoLegionGui {


class WidgetMessage;

class ToolBarSettingsWidget : public ToolBarWidget
{
    Q_OBJECT

public:

    explicit ToolBarSettingsWidget(DataProvider* dataProvider,QWidget *parent = nullptr);

    ~ToolBarSettingsWidget();

   virtual void dataProviderEvent(const legion::messages::Notification &event) override;
   virtual void cleanup() override;
protected slots:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);
    void onResetToDefaults();
    void onApplicationSettingChanged();
    void onDaemonSettingChanged();
    void onSaveCurrentConfiguration();
    void onReconnectDaemon();
    void onStylesheetThemeChanged(int index);

private:

    void resetAllSettingsToDefaults();
    void loadSettings();
    void saveApplicationSettings();
    void loadDaemonSettings();
    void sendDaemonSettingsToDaemon();
    void connectSignals();


private:

    static QString getWhiteStylesheet();
    static QString getGrayStylesheet();
    static QString getDarkStylesheet();

public:

    static QString getStylesheetTheme(ApplicationSettings::ThemeType theme);

private:
    Ui::ToolBarSettingsWidget *ui;

    /*
     * Default Action Map, Defined by user
     */
    std::map<std::string,std::list<std::function< void ()>>> m_defaultActionsMap;
};

}
