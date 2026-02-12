// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ConnectXBoardTrackerBase.h"


namespace bj { namespace connectX {

ConnectXBoardTrackerBase::ConnectXBoardTrackerBase() :
    m_isEnabled(true)
{
    m_filter[BoardItem::O]      = true;
    m_filter[BoardItem::X]      = true;
    m_filter[BoardItem::EMPTY]  = true;
}

void ConnectXBoardTrackerBase::enable()
{
    m_isEnabled = true;
}

void ConnectXBoardTrackerBase::disable()
{
    m_isEnabled = false;
}

}}
