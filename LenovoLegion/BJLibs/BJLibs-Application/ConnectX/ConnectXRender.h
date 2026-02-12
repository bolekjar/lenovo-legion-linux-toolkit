// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <ConnectX/ConnectXBoard.h>

#include <QGraphicsScene>

namespace bj { namespace connectX {

struct ConnectXRender
{
    static void renderBoard(const ConnectXBoardTrackLastPosition& board,QGraphicsScene &scene,int gridSize);
    static void renderScore(const ConnectXBoardBase& board,QGraphicsScene &scene);
};

}}
