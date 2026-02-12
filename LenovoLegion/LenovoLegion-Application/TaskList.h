// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include <list>
#include <functional>


namespace LenovoLegionGui {

    class TaskList
    {

    public:

        TaskList()  = default;
        ~TaskList() = default;


        void addTasks(const std::list<std::function<void ()>>& tasks);
        void runAll();
        void clearTasks();

    private:
        std::list<std::function< void ()>> m_asyncTasks;
    };

}
