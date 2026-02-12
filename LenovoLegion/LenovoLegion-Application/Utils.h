// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../LenovoLegion-PrepareBuild/CPUTopology.pb.h"
#include "../LenovoLegion-PrepareBuild/CPUFrequency.pb.h"

#include <QString>
#include <QTabWidget>
#include <WidgetMessage.h>
#include <QBoxLayout>



#include <functional>


namespace LenovoLegionGui {


namespace Utils
{
    namespace ProtoBuf
    {
            void forAllCpuTopologyRange(const std::function<bool (const int)> &func, const ::google::protobuf::RepeatedPtrField<legion::messages::CPUTopology::ActiveCPUsRange> &cpuRange);
            void forAllCpuTopologyRange(const std::function<bool (const int)> &func, const legion::messages::CPUTopology::ActiveCPUsRange &cpuRange);
            void forAllCpu(const std::function<bool (const legion::messages::CPUFrequency::CPUX &, const size_t)> &func, const ::google::protobuf::RepeatedPtrField<legion::messages::CPUFrequency::CPUX> &cpus);
    }

    namespace Task
    {

            /*
             * Task list helpers, independent of the container type
             */
            template<class T>
            static void insertTasksBack(T &tasks, const std::list<std::function<void ()> > &tasksToInsert)
            {
                tasks.insert(tasks.end(),tasksToInsert.begin(),tasksToInsert.end());
            }

            template<class T>
            static void insertTasksFront(T &tasks, const std::list<std::function<void ()> > &tasksToInsert)
            {
                tasks.insert(tasks.begin(),tasksToInsert.begin(),tasksToInsert.end());
            }

            template<class T>
            static void executeAndClearTasks(T &tasks)
            {
                for(auto& task : tasks)
                    task();

                tasks.clear();
            }

            template<class T>
            static void executeTasks(T &tasks)
            {
                for(auto& task : tasks)
                    task();
            }

    }

    namespace Layout
    {

        void deleteLayoutItem(QLayoutItem *layout);
        void forAllLayoutsDo(QLayout &layout, const std::function<void (QLayoutItem &)>& func);
        void removeAllLayoutWidgets(QLayout &layout);
    }
}

}
