// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <ConnectX/ConnectXGameStats.h>

#include <ConnectX/ConnectXBoardBase.h>

namespace {

bj::framework::BigUnsigned factorial(int n,int stop = 1)
{
    if(n > stop)
        return bj::framework::BigUnsigned(n) * factorial(n - 1,stop);
    else
        return stop;
}

}

namespace bj::connectX {

ConnectXGameStats::Stats ConnectXGameStats::getStats(const ConnectXBoardBase &board, int deapLimit)
{
    Stats       stats              = { 0, 0, 0, 0};
    int         emptyPossitions    = 0;


    for (auto c = board.cbegin() ; c != board.cend(); ++c)
    {
        if(board.get(c.position()).m_item == bj::connectX::BoardItem::EMPTY)
        {
            ++emptyPossitions;
        }
    }

    for ( int i = 0 ; i < ( board.m_xMax * board.m_yMax) ;  i++ ) {
        auto complexity = factorial(board.m_xMax*board.m_yMax,i+1);

        if(deapLimit < 0 || i >= (board.m_xMax * board.m_yMax - (deapLimit + 1) ))
        {
            stats.m_complDeap += complexity;
        }

        stats.m_complOverall += complexity;
    }

    for(int i = 0 ; i <  emptyPossitions;  i++ )
    {
        auto complexity = factorial(emptyPossitions,i+1);

        if(deapLimit < 0 || i >= (emptyPossitions - (deapLimit + 1)))
        {
            stats.m_complDeapEmpty += complexity;
        }

        stats.m_complEmpty += complexity;
    }

    return stats;
}


}
