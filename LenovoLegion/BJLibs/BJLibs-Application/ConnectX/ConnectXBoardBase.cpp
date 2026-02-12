// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ConnectXBoardBase.h"

#include <BigInt/BigIntegerUtils.h>
#include <BigInt/BigIntegerAlgorithms.h>

#include <QMap>
#include <QtConcurrent/QtConcurrentRun>

namespace bj { namespace connectX {

const int ConnectXBoardBase::XEnd = INT32_MAX;
const int ConnectXBoardBase::YEnd = INT32_MAX;
const std::array<int64_t,6> ConnectXBoardBase::ConnectXScorePowFactor = { 1, 1000, 1000000, 1000000000, 100000000000, 100000000000000 };


const ConnectXScore::Attack::SignedNumber ConnectXBoardBase::minusInf = connectX::ConnectXScore::Attack::SignedNumber::getMinusInf();
const ConnectXScore::Attack::SignedNumber ConnectXBoardBase::plusInf  = connectX::ConnectXScore::Attack::SignedNumber::getPlusInf();
const ConnectXScore::Attack::SignedNumber ConnectXBoardBase::undef    = connectX::ConnectXScore::Attack::SignedNumber::getUndef();



ConnectXBoardBase::ConnectXBoardBase(int xMax, int yMax, int connectX):
    m_data(xMax,QVector<BoardItemData>(yMax,BoardItemData{BoardItem::EMPTY,false,false})),
    m_dataScore(xMax,QVector<ConnectXScore>(yMax)),
    m_overallScore(),
    m_xMax(xMax),
    m_yMax(yMax),
    m_connectX(connectX),
    m_minusWinerScore(-ConnectXScorePowFactor.at(connectX - 1)),
    m_plusWinerScore(ConnectXScorePowFactor.at(connectX - 1))
{
    if(m_xMax >= XEnd || m_xMax < 1)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_PARAMETER,"Invalid parametr xMax !")
    }

    if(m_yMax >= YEnd || m_yMax < 1)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_PARAMETER,"Invalid parametr yMax !")
    }

    if(m_connectX > 6)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_PARAMETER,"Invalid parametr m_connectX !")
    }
}

ConnectXBoardBase::ConnectXBoardBase(const ConnectXBoardBase &o) :
    m_data(o.m_data),
    m_dataScore(o.m_dataScore),
    m_overallScore(o.m_overallScore),
    m_xMax(o.m_xMax),
    m_yMax(o.m_yMax),
    m_connectX(o.m_connectX),
    m_minusWinerScore(o.m_minusWinerScore),
    m_plusWinerScore(o.m_plusWinerScore)
{}

ConnectXBoardBase::~ConnectXBoardBase()
{}

const ConnectXBoardBase::ConstIterator ConnectXBoardBase::cbegin() const
{
    return ConstIterator(m_data,m_xMax,m_yMax,0,0);
}

const ConnectXBoardBase::ConstIterator ConnectXBoardBase::cend() const
{
    return ConstIterator(m_data,m_xMax,m_yMax,XEnd,YEnd);
}

ConnectXBoardBase::ConstIteratorImpl::ConstIteratorImpl(const BoardData &boardData, int xMax, int yMax,int posX, int posY) :
    m_boardData(&boardData),
    m_step(0),
    m_posX(posX),
    m_posY(posY),
    m_xMax(xMax),
    m_yMax(yMax)
{}

ConnectXBoardBase::ConstIteratorImpl::ConstIteratorImpl(const ConstIteratorImpl &o) :
    m_boardData(o.m_boardData),
    m_step(o.m_step),
    m_posX(o.m_posX),
    m_posY(o.m_posY),
    m_xMax(o.m_xMax),
    m_yMax(o.m_yMax)
{}

const ConnectXBoardBase::ConstIteratorImpl &ConnectXBoardBase::ConstIteratorImpl::operator=(const ConstIteratorImpl &o)
{
    m_boardData = o.m_boardData;
    m_step = o.m_step;
    m_posX = o.m_posX;
    m_posY = o.m_posY;
    m_xMax = o.m_xMax;
    m_yMax = o.m_yMax;

    return *this;
}

const ConnectXBoardBase::BoardItemData &ConnectXBoardBase::ConstIteratorImpl::operator*() const
{
    return (*m_boardData)[m_posX][m_posY];
}

const ConnectXBoardBase::BoardItemData &ConnectXBoardBase::ConstIteratorImpl::operator ->() const
{
    return (*m_boardData)[m_posX][m_posY];
}

bool ConnectXBoardBase::ConstIteratorImpl::operator==(const ConstIteratorImpl &o) const
{
    return (m_posX == o.m_posX) && (m_posY == o.m_posY);
}

bool ConnectXBoardBase::ConstIteratorImpl::operator!=(const ConstIteratorImpl &o) const
{
    return (m_posX != o.m_posX) || (m_posY != o.m_posY);
}

const ConnectXBoardBase::ConstIteratorImpl &ConnectXBoardBase::ConstIteratorImpl::operator++()
{
    if(isEnd())
        return *this;

    iterateOneStep();
    return *this;
}

ConnectXBoardBase::Position ConnectXBoardBase::ConstIteratorImpl::position() const
{
    return std::make_pair(m_posX,m_posY);
}

void ConnectXBoardBase::ConstIteratorImpl::iterateOneStep()
{
    ++m_step;

    if(m_step >= m_xMax * m_yMax)
    {
        m_posX = XEnd;
        m_posY = YEnd;

        return;
    }

    m_posX = m_step  % m_xMax;
    m_posY = m_step  / m_yMax;
}

bool ConnectXBoardBase::ConstIteratorImpl::isEnd()
{
    return (m_posX == XEnd) && (m_posY == YEnd);
}

const ConnectXBoardBase::ConstIteratorImpl ConnectXBoardBase::ConstIteratorImpl::operator++(int)
{
    ConstIteratorImpl tmp(*this);

    if(isEnd())
        return tmp;

    iterateOneStep();
    return tmp;
}

void ConnectXBoardBase::validatePositions(const int posX, const int posY) const
{
    if(posX >= m_xMax || posX < 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_PARAMETER,"Invalid parametr posX !")
    }

    if(posY >= m_yMax || posY < 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_PARAMETER,"Invalid parametr posY !")
    }
}

ConnectXBoardBase::BoardItemData ConnectXBoardBase::get(const Position& pos) const
{
    validatePositions(pos.first,pos.second);
    return m_data[pos.first][pos.second];
}

bool ConnectXBoardBase::isEmpty() const
{
    bool ret = true;
    std::for_each(cbegin(),cend(),[&ret](const BoardItemData& value){
        if(value.m_item == BoardItem::O || value.m_item == BoardItem::X)
        {
            ret = false;
            return;
        }
    });
    return ret;
}

void ConnectXBoardBase::score(const Position &position)
{
    m_overallScore.clear();

    validatePositions(position.first,position.second);

    m_dataScore[position.first][position.second].set(scoreXDirection(position),scoreYDirection(position),scoreLeftDownDirection(position),scoreLeftUpDirection(position));


    for(int x = std::max(0,position.first - (m_connectX - 1)); x <  std::min(position.first + m_connectX,m_xMax);++x)
    {
        if(x != position.first)
        {
             m_dataScore[x][position.second].setXDirectionAttack(scoreXDirection(std::make_pair(x,position.second)));
        }
    }

    for(int y = std::max(0,position.second - (m_connectX - 1)); y  < std::min(position.second + m_connectX ,m_yMax);++y)
    {
        if(y != position.second)
        {
            m_dataScore[position.first][y].setYDirectionAttack(scoreYDirection(std::make_pair(position.first,y)));
        }
    }


    for(int x = position.first - (m_connectX - 1),
            y = position.second -(m_connectX - 1);
            x < std::min(x + m_connectX,m_xMax) &&
            y < std::min(y + m_connectX,m_yMax);++x,++y)
    {
        if(x < 0 || y < 0)
            continue;

        if(x == position.first && y == position.second)
            continue;

        m_dataScore[x][y].setLeftDownDirectionAttack(scoreLeftDownDirection(std::make_pair(x,y)));
    }

    for(int x = position.first - (m_connectX - 1),y = position.second + (m_connectX - 1);
            x < std::min(x + m_connectX,m_xMax) &&
            y >=std::max(y - m_connectX + 1,0); ++x,--y)
    {
        if(x < 0 || y >= m_yMax)
            continue;


        if(x == position.first && y == position.second)
            continue;

        m_dataScore[x][y].setLeftUpDirectionAttack(scoreLeftUpDirection(std::make_pair(x,y)));
    }


    for(int i = 0;i < m_xMax * m_yMax;++i)
    {
        m_overallScore += m_dataScore[i % m_xMax][i/m_xMax];
    }
}

const ConnectXBoardBase::BoardDataScore &ConnectXBoardBase::dataScore() const
{
    return m_dataScore;
}

ConnectXScore ConnectXBoardBase::overallScore() const
{
     return m_overallScore;
}

bool ConnectXBoardBase::isScorable(const Position &pos, const BoardItem &move) const
{
    validatePositions(pos.first,pos.second);

    if(m_data[pos.first][pos.second].m_item == BoardItem::EMPTY)
    {
        ConnectXScore::Attack::SignedNumber emptyAttack;

        if(move == BoardItem::O)
        {
            if(scoreXDirection(pos,&emptyAttack).getOAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }

            if(scoreYDirection(pos,&emptyAttack).getOAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }

            if(scoreLeftDownDirection(pos,&emptyAttack).getOAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }

            if(scoreLeftUpDirection(pos,&emptyAttack).getOAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }

        }
        else
        {
            if(scoreXDirection(pos,&emptyAttack).getXAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }

            if(scoreYDirection(pos,&emptyAttack).getXAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }

            if(scoreLeftDownDirection(pos,&emptyAttack).getXAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }

            if(scoreLeftUpDirection(pos,&emptyAttack).getXAttack() > 0 || emptyAttack > 0)
            {
                return true;
            }
        }

    }

    return false;
}

bool ConnectXBoardBase::isRated(const Position &pos) const
{
    validatePositions(pos.first,pos.second);

    return (m_dataScore[pos.first][pos.second].attack().getXAttack() != 0) || (m_dataScore[pos.first][pos.second].attack().getOAttack() != 0);
}


bool ConnectXBoardBase::isConnectedX(QVector<std::pair<int, int> > &winnerPosition) const
{

    auto evaluateLineX = [this](QVector<std::pair<int, int> > &curResult,int X,int Y){
        if(curResult.empty())
        {
            if(m_data.at(X).at(Y).m_item == BoardItem::O || m_data.at(X).at(Y).m_item == BoardItem::X)
            {
                curResult.push_back(std::make_pair(X,Y));
            }
        }
        else
        {
            if(m_data.at(X).at(Y).m_item == m_data.at(curResult[0].first).at(curResult[0].second).m_item)
            {
                curResult.push_back(std::make_pair(X,Y));

                if(curResult.size() >= m_connectX )
                    return true;
            }
            else
            {
                curResult.clear();
                if(m_data.at(X).at(Y).m_item == BoardItem::O || m_data.at(X).at(Y).m_item == BoardItem::X)
                {
                    curResult.push_back(std::make_pair(X,Y));
                }
            }
        }

        return false;
    };



    QVector<QFuture<QVector<std::pair<int,int>>>> results =
        {
            QtConcurrent::run([&evaluateLineX,this]{
                for (int X = 0; X < m_xMax;++X) {
                    QVector<std::pair<int,int>> curResult;

                    for (int Y = 0; Y < m_yMax; ++Y)
                    {

                        if(evaluateLineX(curResult,X,Y))
                        {
                            return curResult;
                        }
                    }
                }
                return QVector<std::pair<int,int>>();
            }),
            QtConcurrent::run([&evaluateLineX,this]{
                for (int Y = 0; Y < m_xMax;++Y) {
                    QVector<std::pair<int,int>> curResult;
                    for (int X = 0; X < m_yMax; ++X)
                    {
                        if(evaluateLineX(curResult,X,Y))
                        {
                            return curResult;
                        }
                    }
                }
                return QVector<std::pair<int,int>>();
            }),

            QtConcurrent::run([&evaluateLineX,this]{
                for (int Y = 0; Y < m_xMax;++Y) {
                    QVector<std::pair<int,int>> curResult;

                    for (int X = 0,Y1 = Y; Y1 < m_yMax; ++X, ++Y1)
                    {
                        if(evaluateLineX(curResult,X,Y1))
                        {
                            return curResult;
                        }
                    }
                }
                return QVector<std::pair<int,int>>();
            }),

            QtConcurrent::run([&evaluateLineX,this]{
                for (int X = 0; X < m_xMax;++X) {
                    QVector<std::pair<int,int>> curResult;

                    for (int X1 = X,Y = 0; X1 < m_xMax; ++X1, ++Y)
                    {
                        if(evaluateLineX(curResult,X1,Y))
                        {
                            return curResult;
                        }
                    }
                }
                return QVector<std::pair<int,int>>();
            }),
            QtConcurrent::run([&evaluateLineX,this]{
                for (int X = m_xMax -1; X > -1;--X) {
                    QVector<std::pair<int,int>> curResult;

                    for (int X1 = X,Y = 0; X1 > -1; --X1, ++Y)
                    {
                        if(evaluateLineX(curResult,X1,Y))
                        {
                            return curResult;
                        }
                    }
                }
                return QVector<std::pair<int,int>>();
            }),
            QtConcurrent::run([&evaluateLineX,this]{
                for (int Y = m_xMax - 1; Y > -1;--Y) {
                    QVector<std::pair<int,int>> curResult;

                    for (int X = m_xMax  - 1,Y1 = Y; Y1 < m_yMax; --X, Y1++)
                    {
                        if(evaluateLineX(curResult,X,Y1))
                        {
                            return curResult;
                        }
                    }
                }
                return QVector<std::pair<int,int>>();
            })

        };


    bool ret = false;
    for (const auto& cur : results) {
        if(cur.result().size() >= m_connectX )
        {
            winnerPosition = cur.result();
            ret = true;
        }
    }

    return ret;
}

bool ConnectXBoardBase::isDraw() const
{

    auto simulatorDraw = [](const ConnectXBoardBase& boardSimulation,BoardItem itemSimulation){
        ConnectXBoardBase m_simulation(boardSimulation);

        for (int X = 0; X < m_simulation.m_xMax; X++) {
            for (int Y = 0; Y < m_simulation.m_yMax;Y++) {
                if(m_simulation.m_data[X][Y].m_item == BoardItem::EMPTY)
                {
                    m_simulation.m_data[X][Y].m_item = itemSimulation;
                }
            }
        }

        QVector<std::pair<int, int> > winnerPosition;
        if(m_simulation.isConnectedX(winnerPosition)) {
            return false;
        }

        return true;
    };


    return (simulatorDraw(*this,BoardItem::O) && simulatorDraw(*this,BoardItem::X));
}

ConnectXScore::Attack ConnectXBoardBase::scoreXDirection(const Position &position,ConnectXScore::Attack::SignedNumber* emptyAttack) const
{
    ConnectXScore::Attack score;

    auto writeXDirection = [&score,&emptyAttack](const ConnectXAnalyseResult& result){
        score.operator+=({result.m_xAttack == 0 ? result.m_oAttack : 0,result.m_oAttack  == 0 ? result.m_xAttack : 0});

        if(emptyAttack != nullptr && result.m_xAttack == 0 && result.m_oAttack == 0)
        {
            emptyAttack->operator+=(result.m_empty);
        }
    };

    for(int x = std::max(0,position.first - (m_connectX - 1)); x <= position.first && ((x + m_connectX - 1) < m_xMax);++x)
    {
        std::vector<BoardItem> connectX;

        for(int z = x; (z - x) < m_connectX; z++)
        {
            if(z < m_xMax && z < position.first + m_connectX)
            {
                connectX.push_back(m_data[z][position.second].m_item);
            }
            else
            {
                break;
            }
        }

        writeXDirection(analyzeConnectX(connectX));
    }

    return score;
}

ConnectXScore::Attack ConnectXBoardBase::scoreYDirection(const Position &position,ConnectXScore::Attack::SignedNumber* emptyAttack) const
{
    ConnectXScore::Attack score;

    auto writeYDirection = [&score,&emptyAttack](const ConnectXAnalyseResult& result){
        score.operator+=({result.m_xAttack == 0 ? result.m_oAttack : 0,result.m_oAttack  == 0 ? result.m_xAttack : 0});

        if(emptyAttack != nullptr && result.m_xAttack == 0 && result.m_oAttack == 0)
        {
            emptyAttack->operator+=(result.m_empty);
        }
    };

    for(int y = std::max(0,position.second - (m_connectX - 1)); y <= position.second && ((y + m_connectX - 1) < m_yMax);++y)
    {
        std::vector<BoardItem> connectX;

        for(int z = y; (z - y) < m_connectX; z++)
        {
            if(z < m_yMax && z < position.second + m_connectX)
            {
                connectX.push_back(m_data[position.first][z].m_item);
            }
            else
            {
                break;
            }
        }

        writeYDirection(analyzeConnectX(connectX));
    }

    return score;
}

ConnectXScore::Attack ConnectXBoardBase::scoreLeftDownDirection(const Position &position,ConnectXScore::Attack::SignedNumber* emptyAttack) const
{
    ConnectXScore::Attack score;

    auto writeLeftDownDirection = [&score,&emptyAttack](const ConnectXAnalyseResult& result){
        score.operator+=({result.m_xAttack == 0 ? result.m_oAttack : 0,result.m_oAttack  == 0 ? result.m_xAttack : 0});

        if(emptyAttack != nullptr && result.m_xAttack == 0 && result.m_oAttack == 0)
        {
            emptyAttack->operator+=(result.m_empty);
        }
    };

    for(int x = position.first - (m_connectX - 1),y = position.second - (m_connectX - 1);
        x <= position.first && y <= position.second && ((x + m_connectX - 1) < m_xMax) && ((y + m_connectX - 1) < m_yMax); ++x,++y)
    {
        std::vector<BoardItem> connectX;

        if(x < 0 || y < 0)
            continue;

        for(int z = x,z1 = y,i = 0; i < m_connectX; z++,++i,z1++)
        {
            if(z < m_xMax && z1 < m_yMax)
            {
                connectX.push_back(m_data[z][z1].m_item);
            }
            else
            {
                break;
            }
        }

        writeLeftDownDirection(analyzeConnectX(connectX));
    }

    return score;
}

ConnectXScore::Attack ConnectXBoardBase::scoreLeftUpDirection(const Position &position,ConnectXScore::Attack::SignedNumber* emptyAttack) const
{
    ConnectXScore::Attack score;

    auto writeLeftUpDirection = [&score,&emptyAttack](const ConnectXAnalyseResult& result){
        score.operator+=({result.m_xAttack == 0 ? result.m_oAttack : 0,result.m_oAttack  == 0 ? result.m_xAttack : 0});

        if(emptyAttack != nullptr && result.m_xAttack == 0 && result.m_oAttack == 0)
        {
            emptyAttack->operator+=(result.m_empty);
        }

    };

    for(int x = position.first - (m_connectX - 1),y = position.second + (m_connectX - 1);
            x <= position.first && y >= position.second && ((x + m_connectX - 1) < m_xMax) && ((y - m_connectX + 1) >= 0); ++x,--y)
    {
        std::vector<BoardItem> connectX;

        if(x < 0 || y >= m_yMax)
            continue;

        for(int z = x,z1 = y,i = 0; i < m_connectX; z++,++i,z1--)
        {
            if(z < m_xMax && z1 < m_yMax)
            {
                connectX.push_back(m_data[z][z1].m_item);
            }
            else
            {
                break;
            }
        }

        writeLeftUpDirection(analyzeConnectX(connectX));
    }

    return score;
}

ConnectXBoardBase::ConnectXAnalyseResult ConnectXBoardBase::analyzeConnectX(const std::vector<BoardItem> &connectX) const
{
    if(connectX.size() == static_cast<size_t>(m_connectX))
    {
        QMap<BoardItem,int> analyze;

        for(const auto& x : connectX)
        {
            analyze[x] += 1;
        }

        return ConnectXAnalyseResult {
            analyze[BoardItem::O] == 0 ? 0     : analyze[BoardItem::O] * ConnectXScorePowFactor.at(analyze[BoardItem::O] - 1),
            analyze[BoardItem::X] == 0 ? 0     : analyze[BoardItem::X] * ConnectXScorePowFactor.at(analyze[BoardItem::X] - 1),
            analyze[BoardItem::EMPTY] == 0 ? 0 : analyze[BoardItem::EMPTY] * ConnectXScorePowFactor.at(analyze[BoardItem::EMPTY] - 1)
        };
    }

    return ConnectXAnalyseResult {0,0,0};
}

}}
