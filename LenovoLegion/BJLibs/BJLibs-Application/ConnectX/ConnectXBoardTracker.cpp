// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ConnectXBoardTracker.h"

namespace bj { namespace connectX {



ConnectXBoardTracker::ConnectXBoardTracker() : m_last{std::make_pair(-1,-1),BoardItem::EMPTY}
{}


void ConnectXBoardTracker::store(const TrackerData& data)
{
    if(m_isEnabled && m_filter[data.m_item])
    {
        m_last = data;
    }
}

const ConnectXBoardTrackerBase::TrackerData &ConnectXBoardTracker::last() const
{
    return  m_last;
}

bool ConnectXBoardTracker::isDefined() const
{
    return (m_last.m_position.first > -1) && (m_last.m_position.second > -1);
}

}}
