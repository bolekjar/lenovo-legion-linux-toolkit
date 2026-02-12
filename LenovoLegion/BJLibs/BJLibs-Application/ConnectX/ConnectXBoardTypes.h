// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include <QString>

namespace bj { namespace connectX {


enum class BoardItem {
    EMPTY   = 0,
    X       = 1,
    O       = 2
};

struct BoardItemData {
    BoardItem m_item;
    bool      m_analyze;
    bool      m_processing;
};

inline QString boardItemtoString(const BoardItem& item)
{
    switch (item) {
    case BoardItem::EMPTY:
        return "EMPTY";
        break;
    case BoardItem::O:
        return "O";
        break;
    case BoardItem::X:
        return "X";
        break;
    }

    return "ERROR";
}

}}
