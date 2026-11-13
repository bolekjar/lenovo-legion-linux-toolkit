// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "PageSources.h"
#include "DataSource.h"

#include <QTimer>

namespace LenovoLegionGui {

PageSources::PageSources(QObject *parent) : QObject(parent)
{}

QObject *PageSources::page() const
{
    return m_page;
}

void PageSources::setPage(QObject *page)
{
    if(page == m_page)
    {
        return;
    }

    m_page = page;
    emit pageChanged();

    scan();

    /*
     * Objects of the page created after the binding (the page is completed later)
     */
    QTimer::singleShot(0,this,&PageSources::scan);
}

bool PageSources::available() const
{
    /*
     * A page without own data sources (detail windows of the HW monitor use the sources of the monitor page)
     * is always available
     */
    bool any = false;

    for(const auto& source : m_sources)
    {
        any = any || source != nullptr;
    }

    if(!any)
    {
        return true;
    }

    for(const auto& source : m_sources)
    {
        if(source != nullptr && source->available())
        {
            return true;
        }
    }

    return false;
}

QString PageSources::platform() const
{
    for(const auto& source : m_sources)
    {
        if(source != nullptr && source->platform() == "lenovo")
        {
            return "lenovo";
        }
    }

    return "generic";
}

QString PageSources::reason() const
{
    if(available())
    {
        return {};
    }

    if(platform() == "lenovo")
    {
        return "Requires the Lenovo Legion kernel driver";
    }

    /*
     * Reason of the data source (e.g. no NVIDIA GPU, (legion.ui.unavailable_reason))
     */
    for(const auto& source : m_sources)
    {
        if(source != nullptr && !source->lastError().isEmpty())
        {
            return source->lastError();
        }
    }

    return "The daemon provides no data for this page";
}

QString PageSources::description() const
{
    return platform() == "lenovo" ? "Lenovo Legion notebook: controlled by the Lenovo Legion kernel driver (legion)"
                                  : "Standard Linux interfaces: works on any notebook";
}

int PageSources::changeCount() const
{
    int count = 0;

    for(const auto& source : m_sources)
    {
        if(source != nullptr)
        {
            count += source->changeCount();
        }
    }

    return count;
}

void PageSources::apply()
{
    /*
     * Copy: apply reloads the sources, the list must not change during the loop
     */
    const auto sources = m_sources;

    for(const auto& source : sources)
    {
        if(source != nullptr && source->dirty())
        {
            source->apply();
        }
    }
}

void PageSources::cancel()
{
    const auto sources = m_sources;

    for(const auto& source : sources)
    {
        if(source != nullptr && source->dirty())
        {
            source->cancel();
        }
    }
}

void PageSources::scan()
{
    for(const auto& source : std::as_const(m_sources))
    {
        if(source != nullptr)
        {
            disconnect(source,nullptr,this,nullptr);
        }
    }

    m_sources.clear();

    if(m_page != nullptr)
    {
        for(auto* source : m_page->findChildren<DataSource*>())
        {
            m_sources.append(source);
            connect(source,&DataSource::messageChanged,this,&PageSources::stateChanged);
            connect(source,&DataSource::dataTypeChanged,this,&PageSources::stateChanged);
        }
    }

    emit stateChanged();
}

}
