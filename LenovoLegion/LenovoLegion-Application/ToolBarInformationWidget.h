// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ToolBarWidget.h"


#include "../LenovoLegion-PrepareBuild/ComputerInfo.pb.h"
#include "../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h"

namespace Ui {
class ToolBarInformationWidget;
}


namespace LenovoLegionGui {


class WidgetMessage;

class ToolBarInformationWidget : public ToolBarWidget
{
    Q_OBJECT

public:

    explicit ToolBarInformationWidget(DataProvider* dataProvider,QWidget *parent = nullptr);

    ~ToolBarInformationWidget();

    virtual void dataProviderEvent(const legion::messages::Notification &event) override;
    virtual void cleanup() override;


protected slots:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private:

    void loadMachineInformation();
    void displayMachineInformation();
    void loadCPUInformation();
    void displayCPUInformation();
    void loadGPUInformation();
    void displayGPUInformation();

    Ui::ToolBarInformationWidget *ui;

    legion::messages::MachineInformation m_machineInfo;
    legion::messages::CPUInfo m_cpuInfo;
    legion::messages::NvidiaNvml m_gpuInfo;

    /*
     * Default Action Map, Defined by user
     */
    std::map<std::string,std::list<std::function< void ()>>> m_defaultActionsMap;
};

}
