// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include "ConnectXBoardTypes.h"


#include <map>
#include <bits/stl_pair.h>

namespace bj { namespace connectX {


class ConnectXBoardTrackerBase
{
    struct Data {
        std::pair<int,int> m_position;
        BoardItem          m_item;
    };

public:

    using TrackerData = Data;

public:

    ConnectXBoardTrackerBase();

    void enable();
    void disable();

    virtual void store(const TrackerData& data) = 0;

protected:

    bool                     m_isEnabled;
    std::map<BoardItem,bool> m_filter;
};


}}
