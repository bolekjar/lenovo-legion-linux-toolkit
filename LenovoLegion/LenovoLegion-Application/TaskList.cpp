#include "TaskList.h"

#include "Utils.h"

namespace LenovoLegionGui {

void TaskList::addTasks(const std::list<std::function<void ()>>& tasks)
{

    Utils::Task::insertTasksBack(m_asyncTasks,tasks);
}

void TaskList::runAll()
{
    Utils::Task::executeAndClearTasks(m_asyncTasks);
}


void TaskList::clearTasks() { m_asyncTasks.clear(); }



}
