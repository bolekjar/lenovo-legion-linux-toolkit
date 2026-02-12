// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ConnectXRender.h"

#include <QGraphicsItem>

namespace {

void renderA(int X1, int Y1, int X2, int Y2, Qt::GlobalColor color,QGraphicsScene &scene, int gridSize)
{
    scene.addLine(X1,Y1,X2,Y2,QPen(color));
    scene.addLine(X1+gridSize,Y1,X2-gridSize,Y2,QPen(color));
    scene.addEllipse(X1+1,Y1+1,gridSize -2 ,gridSize - 2,QPen(color),QBrush());
}

void renderX(int X1, int Y1,int X2,int Y2,Qt::GlobalColor color,int width,QGraphicsScene &scene, int gridSize)
{
    QPen  pen(color);
    pen.setWidth(width);

    scene.addLine(X1 + width,Y1 + width,X2 - width,Y2 - width,pen);
    scene.addLine(X1+gridSize - width ,Y1 + width,X2-gridSize + width,Y2 - width,pen);
}

void renderO(int X, int Y,Qt::GlobalColor color,int width,QGraphicsScene &scene, int gridSize)
{
    QPen  pen(color);
    pen.setWidth(width);

    scene.addEllipse(X+1,Y+1,gridSize - width,gridSize - width,pen,QBrush());
}

}


void bj::connectX::ConnectXRender::renderBoard(const ConnectXBoardTrackLastPosition &board, QGraphicsScene &scene, int gridSize)
{
    QPen  pen(Qt::darkYellow);

    scene.clear();
    scene.addRect(0,0,board.m_xMax* gridSize,board.m_yMax* gridSize,pen);


    for (int X = 0; X < board.m_xMax; ++X) {
        scene.addText(QString::number(X))->setPos(X * gridSize + 10,-25);
    }

    for (int Y = 0; Y < board.m_yMax; ++Y) {
        scene.addText(QString::number(Y))->setPos(-25,Y * gridSize + 10);
    }


    for (int curX = 0;curX < board.m_xMax;curX++) {
        scene.addLine(curX*gridSize,0,curX*gridSize,board.m_yMax*gridSize,pen);

        for (int curY = 0;curY < board.m_yMax; curY++) {
            scene.addLine(0,curY*gridSize,board.m_xMax*gridSize,curY*gridSize,pen);

            switch (board.get(std::make_pair(curX,curY)).m_item ) {
            case bj::connectX::BoardItem::O:
                renderO(curX*gridSize,curY*gridSize,Qt::blue,1,scene,gridSize);
                break;
            case bj::connectX::BoardItem::X:
                renderX(curX*gridSize,curY*gridSize,curX*gridSize+gridSize,curY*gridSize+gridSize,Qt::red,1,scene,gridSize);
                break;
            case bj::connectX::BoardItem::EMPTY:
                break;
            }

            if(board.get(std::make_pair(curX,curY)).m_analyze)
            {
                renderA(curX*gridSize,curY*gridSize,curX*gridSize+gridSize,curY*gridSize+gridSize,Qt::green,scene,gridSize);
            }
        }
    }


    if(board.tracker().isDefined())
    {
        switch (board.tracker().last().m_item) {
        case ConnectXBoardTrackLastPosition::BoardItem::O:
            renderO(board.tracker().last().m_position.first*gridSize,board.tracker().last().m_position.second*gridSize,Qt::blue,2,scene,gridSize);
            break;
        case ConnectXBoardTrackLastPosition::BoardItem::X:
            renderX(board.tracker().last().m_position.first*gridSize,board.tracker().last().m_position.second*gridSize,board.tracker().last().m_position.first*gridSize+gridSize,board.tracker().last().m_position.second*gridSize+gridSize,Qt::red,2,scene,gridSize);
            break;
        case ConnectXBoardTrackLastPosition::BoardItem::EMPTY:
            break;
        }
    }
}

void bj::connectX::ConnectXRender::renderScore(const ConnectXBoardBase &board, QGraphicsScene &scene)
{
    QPen  pen(Qt::darkYellow);

    scene.clear();
    scene.addRect(0,0,board.m_xMax*150,board.m_yMax*100,pen);

    for (int X = 0; X < board.m_xMax; ++X) {
        scene.addText(QString::number(X))->setPos(X * 150 + 10,-25);
    }

    for (int Y = 0; Y < board.m_yMax; ++Y) {
        scene.addText(QString::number(Y))->setPos(-25,Y * 100 + 10);
    }

    for (int curX = 0;curX < board.m_xMax;curX++) {
        scene.addLine(curX*150,0,curX*150, board.m_yMax*100,pen);
        for (int curY = 0;curY < board.m_yMax; curY++) {
            scene.addLine(0,curY*100,board.m_xMax*150,curY*100,pen);

            scene.addText(QString("O:").append(bj::connectX::ConnectXScore::Attack::SignedNumber::toString(board.dataScore()[curX][curY].attack().getOAttack()).c_str()))->setPos(curX*150,curY*100);
            scene.addText(QString("X:").append(bj::connectX::ConnectXScore::Attack::SignedNumber::toString(board.dataScore()[curX][curY].attack().getXAttack()).c_str()))->setPos(curX*150,curY*100 + 20);
        }
    }
}
