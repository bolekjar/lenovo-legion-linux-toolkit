// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ToolBarWidget.h"

#include "../LenovoLegion-PrepareBuild/Profiles.pb.h"

#include <map>


namespace Ui {
class ToolBarProfilesWidget;
}


namespace LenovoLegionGui {


class WidgetMessage;

class ToolBarProfilesWidget : public ToolBarWidget
{
    Q_OBJECT

public:

    explicit ToolBarProfilesWidget(DataProvider* dataProvider,QWidget *parent = nullptr);

    ~ToolBarProfilesWidget();

    virtual void dataProviderEvent(const legion::messages::Notification &event) override;
    virtual void cleanup() override;


protected slots:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);
    
    // Profile management slots
    void onSaveProfile();
    void onLoadProfile();
    void onDeleteProfile();
    void onProfileSelected();

private:
    
    void connectSignals();
    void loadProfilesList();
    void showProfiles(const legion::messages::Profiles& profiles);
    void updateProfileDetails(const QString& profileName);

    /*
     * Profiles are saved and applied by the daemon, response is the list of the profiles after the command
     */
    void profileCommand(legion::messages::ProfileCommand::Action action,const QString& name,const QString& description = {});

    Ui::ToolBarProfilesWidget *ui;

    /*
     * Default Action Map, Defined by user
     */
    std::map<std::string,std::list<std::function< void ()>>> m_defaultActionsMap;

    /*
     * Profiles from the daemon, name -> description
     */
    std::map<QString,QString> m_profiles;
};

}
