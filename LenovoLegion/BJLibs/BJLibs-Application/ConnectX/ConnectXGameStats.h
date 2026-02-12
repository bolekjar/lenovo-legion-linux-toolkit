// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <BigInt/BigUnsigned.h>


namespace bj { namespace connectX {


class ConnectXBoardBase;

struct ConnectXGameStats
{
private:

    struct Stats {

        framework::BigUnsigned m_complOverall;      //Overall complexity                              (count of nodes in tree)
        framework::BigUnsigned m_complDeap;         //Complexity cut by deapness                      (count of nodes in tree)
        framework::BigUnsigned m_complEmpty;        //Complexity of empty possition                   (count of nodes in tree)
        framework::BigUnsigned m_complDeapEmpty;    //Complexity of empty possition cut by deapness   (count of nodes in tree)

    };


public:

    using StatsType = Stats;

    static Stats getStats(const ConnectXBoardBase& board,int deapLimit);
};



}}


