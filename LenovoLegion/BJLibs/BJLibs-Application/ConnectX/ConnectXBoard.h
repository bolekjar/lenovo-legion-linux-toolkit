// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QList>

#include "ConnectXBoardBase.h"
#include "ConnectXBoardTracker.h"

#include <functional>

namespace bj { namespace connectX {


template<class Tracker>
class ConnectXBoard;


template<>
class ConnectXBoard<ConnectXBoardTracker> : public ConnectXBoardBase
{

private:

    class SpinIteratorImpl {

    private:

        enum class IterateDirection  {
            LEFT  = 0,
            RIGHT = 1,
            UP    = 2,
            DOWN  = 3,
            END   = 4
        };

    public:

        using  iterator_category  = std::forward_iterator_tag;

    public:

        SpinIteratorImpl();
        SpinIteratorImpl(BoardData& boardData,ConnectXBoardTracker& tracker,std::function<void()> scoreUpdater,int xMax, int yMax,int posX, int posY);
        SpinIteratorImpl(const SpinIteratorImpl &o);
        SpinIteratorImpl& operator=(const SpinIteratorImpl &o);
        const BoardItemData  &operator*();
        const BoardItemData    &operator->();
        bool operator==(const SpinIteratorImpl &o) const;
        bool operator!=(const SpinIteratorImpl &o) const;
        SpinIteratorImpl &operator++();
        SpinIteratorImpl  operator++(int);

        void update(const BoardItemData  &item);
        void update(const BoardItem  &item);
        void update(bool  analyse);
        void updateProcessing(bool  processing);

        Position position() const;

    private:

        void iterateOneStep();
        void nextPosition(const IterateDirection dir);
        bool isOutOffBoard();
        bool isEnd();

        BoardData*                  m_boardData;
        ConnectXBoardTracker*       m_tracker;
        std::function<void()>       m_scoreUpdater;
        QVector<IterateDirection>   m_stepDirection;
        int                         m_step;
        int                         m_posX;
        int                         m_posY;
        int                         m_xMax;
        int                         m_yMax;
    };

    class IteratorImpl {

    public:

        using  iterator_category  = std::forward_iterator_tag;

    public:

        IteratorImpl(BoardData& boardData,ConnectXBoardTracker& tracker,std::function<void()> scoreUpdater, int xMax, int yMax,int posX, int posY);
        IteratorImpl(const IteratorImpl &o);
        IteratorImpl& operator=(const IteratorImpl &o);
        const BoardItemData  &operator*();
        const BoardItemData  &operator->();
        bool operator==(const IteratorImpl &o) const;
        bool operator!=(const IteratorImpl &o) const;
        IteratorImpl &operator++();
        IteratorImpl  operator++(int);

        void update(const BoardItemData  &item);
        void update(const BoardItem  &item);
        void update(bool  analyse);
        void updateProcessing(bool  processing);

        Position position() const;

    private:

        void iterateOneStep();
        bool isEnd();

        BoardData*                  m_boardData;
        ConnectXBoardTracker*       m_tracker;
        std::function<void()>       m_scoreUpdater;
        int                         m_step;
        int                         m_posX;
        int                         m_posY;
        int                         m_xMax;
        int                         m_yMax;
    };

public:

    using SpinIterator  = SpinIteratorImpl;
    using Iterator      = IteratorImpl;

public:

    ConnectXBoard(int xMax, int yMax, int connectX);
    ConnectXBoard(const ConnectXBoard& o);


    SpinIterator beginSpin();
    SpinIterator beginSpin(const Position& pos);
    SpinIterator endSpin();

    const ConnectXBoardTracker&  tracker()            const;

    Iterator                begin();
    Iterator                end();

    void set(const Position& pos,const BoardItemData value);
    void set(const Position& pos,const BoardItem value);
    void set(const Position& pos,const bool analyse);

    void clear();

private:
    void         score();

    ConnectXBoardTracker m_tracker;

};


using ConnectXBoardTrackLastPosition = ConnectXBoard<ConnectXBoardTracker>;




////////////////////////////////////////////////implementation//////////////////////////////////////////////////

inline ConnectXBoard<ConnectXBoardTracker>::ConnectXBoard(int xMax, int yMax, int connectX) : ConnectXBoardBase(xMax,yMax,connectX){}
inline ConnectXBoard<ConnectXBoardTracker>::ConnectXBoard(const ConnectXBoard &o) : ConnectXBoardBase(o) , m_tracker(o.m_tracker) {}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIterator ConnectXBoard<ConnectXBoardTracker>::beginSpin()
{
    return SpinIterator(m_data,m_tracker,std::bind(&ConnectXBoard<ConnectXBoardTracker>::score,this),m_xMax,m_yMax,m_xMax/2,m_yMax/2);
}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIterator ConnectXBoard<ConnectXBoardTracker>::beginSpin(const Position& pos)
{
    validatePositions(pos.first,pos.second);
    return SpinIterator(m_data,m_tracker,std::bind(&ConnectXBoard<ConnectXBoardTracker>::score,this),m_xMax,m_yMax,pos.first,pos.second);
}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIterator ConnectXBoard<ConnectXBoardTracker>::endSpin()
{
    return SpinIterator(m_data,m_tracker,std::bind(&ConnectXBoard<ConnectXBoardTracker>::score,this),m_xMax,m_yMax,XEnd,YEnd);
}

inline ConnectXBoard<ConnectXBoardTracker>::Iterator ConnectXBoard<ConnectXBoardTracker>::begin()
{
    return Iterator(m_data,m_tracker,std::bind(&ConnectXBoard<ConnectXBoardTracker>::score,this),m_xMax,m_yMax,0,0);
}

inline ConnectXBoard<ConnectXBoardTracker>::Iterator ConnectXBoard<ConnectXBoardTracker>::end()
{
    return Iterator(m_data,m_tracker,std::bind(&ConnectXBoard<ConnectXBoardTracker>::score,this),m_xMax,m_yMax,XEnd,YEnd);
}

inline void ConnectXBoard<ConnectXBoardTracker>::set(const Position &pos, const BoardItemData value)
{
    validatePositions(pos.first,pos.second);
    m_data[pos.first][pos.second] = value;
    m_tracker.store(ConnectXBoardTracker::TrackerData{pos,value.m_item});
    score();
}

inline void ConnectXBoard<ConnectXBoardTracker>::set(const Position &pos, const BoardItem value)
{
    validatePositions(pos.first,pos.second);
    m_data[pos.first][pos.second].m_item = value;
    m_tracker.store(ConnectXBoardTracker::TrackerData{pos,value});
    score();
}

inline void ConnectXBoard<ConnectXBoardTracker>::set(const Position &pos, const bool analyse)
{
    validatePositions(pos.first,pos.second);
    m_data[pos.first][pos.second].m_analyze = analyse;
}

inline void ConnectXBoard<ConnectXBoardTracker>::clear()
{
    for(auto cur = begin();cur != end();++cur){
        cur.update({BoardItem::EMPTY,false,false});
    }

}

inline bj::connectX::ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::SpinIteratorImpl() :
    m_boardData(nullptr),
    m_tracker(nullptr),
    m_scoreUpdater(nullptr),
    m_step(0),
    m_posX(-1),
    m_posY(-1),
    m_xMax(-1),
    m_yMax(-1)
{}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::SpinIteratorImpl(ConnectXBoard<ConnectXBoardTracker>::BoardData &boardData,ConnectXBoardTracker& tracker,std::function<void()> scoreUpdater,int xMax,int yMax,int posX,int posY) :
    m_boardData(&boardData),
    m_tracker(&tracker),
    m_scoreUpdater(scoreUpdater),
    m_step(0),
    m_posX(posX),
    m_posY(posY),
    m_xMax(xMax),
    m_yMax(yMax)
{
    const QList<IterateDirection>          m_iterDirection = {
        IterateDirection::LEFT,
        IterateDirection::UP,
        IterateDirection::RIGHT,
        IterateDirection::DOWN
    };

    QList<IterateDirection>::const_iterator curDirection = m_iterDirection.cbegin();

    auto nextDirection = [&m_iterDirection,&curDirection]()
    {
        ++curDirection;

        if(curDirection == m_iterDirection.constEnd())
        {
            curDirection = m_iterDirection.cbegin();
        }
    };


    m_stepDirection.push_back(*curDirection);
    nextDirection();

    bool changeQuantum = true;
    for(int step = 1,quantum = 0, quantumInit = 1; step < (m_xMax * m_yMax * 4); ++step, quantum--)
    {
        m_stepDirection.push_back(*curDirection);

        if(quantum == 0)
        {
            nextDirection();

            if(changeQuantum)
            {
                ++quantumInit;
                quantum =  quantumInit;
            }

            if(changeQuantum == true)
            {
                changeQuantum = false;
            }
            else
            {
                changeQuantum = true;
                quantum =  quantumInit;
            }
        }
    }

    m_stepDirection.push_back(IterateDirection::END);
}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::SpinIteratorImpl(const ConnectXBoard::SpinIteratorImpl &o) :
    m_boardData(o.m_boardData),
    m_tracker(o.m_tracker),
    m_scoreUpdater(o.m_scoreUpdater),
    m_stepDirection(o.m_stepDirection),
    m_step(o.m_step),
    m_posX(o.m_posX),
    m_posY(o.m_posY),
    m_xMax(o.m_xMax),
    m_yMax(o.m_yMax)
{}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl &ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::operator=(const ConnectXBoard::SpinIteratorImpl &o)
{
    m_boardData = o.m_boardData;
    m_tracker   = o.m_tracker;
    m_scoreUpdater = o.m_scoreUpdater;
    m_stepDirection = o.m_stepDirection;
    m_step = o.m_step;
    m_posX = o.m_posX;
    m_posY = o.m_posY;
    m_xMax = o.m_xMax;
    m_yMax = o.m_yMax;

    return *this;
}

inline const ConnectXBoard<ConnectXBoardTracker>::BoardItemData &ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::operator*()
{
    return (*m_boardData)[m_posX][m_posY];
}

inline const ConnectXBoard<ConnectXBoardTracker>::BoardItemData &ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::operator ->()
{
    return (*m_boardData)[m_posX][m_posY];
}

inline bool ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::operator==(const ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl &o) const
{
   return (m_posX == o.m_posX) && (m_posY == o.m_posY);
}

inline bool ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::operator!=(const ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl &o) const
{
    return (m_posX != o.m_posX) || (m_posY != o.m_posY);
}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl &ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::operator++()
{
    if(isEnd())
        return *this;

    iterateOneStep();
    return *this;
}

inline ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl  ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::operator++(int)
{
    SpinIteratorImpl tmp(*this);

    if(isEnd())
        return tmp;

    iterateOneStep();
    return tmp;
}

inline void ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::iterateOneStep()
{
    do
    {
        nextPosition((m_stepDirection[m_step++]));

        if(isEnd())
            return;

    } while (isOutOffBoard());
}

inline void ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::nextPosition(const ConnectXBoard::SpinIteratorImpl::IterateDirection dir)
{
    switch(dir)
    {

    case IterateDirection::DOWN:
    {
        ++m_posY;
        break;
    }
    case IterateDirection::UP:
    {
        --m_posY;
        break;
    }
    case IterateDirection::LEFT:
    {
        --m_posX;
        break;
    }
    case IterateDirection::RIGHT:
    {
        ++m_posX;
        break;
    }
    case IterateDirection::END:
    {
       m_posX = XEnd;
       m_posY = YEnd;
       break;
    }

    }
}

inline bool ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::isOutOffBoard()
{
    if(m_posX < 0 || m_posX >= m_xMax)
    {
        return true;
    }

    if(m_posY < 0 || m_posY >= m_yMax)
    {
        return true;
    }

    return false;
}

inline bool ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::isEnd()
{
    return (m_posX == XEnd) && (m_posY == YEnd);
}

inline void ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::update(const BoardItemData  &item)
{
    (*m_boardData)[m_posX][m_posY] = item;
    m_tracker->store({std::make_pair(m_posX,m_posY),item.m_item});
    m_scoreUpdater();
}

inline void ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::update(const BoardItem  &item)
{
    (*m_boardData)[m_posX][m_posY].m_item = item;
    m_tracker->store({std::make_pair(m_posX,m_posY),item});
    m_scoreUpdater();
}

inline void ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::update(bool analyse)
{
    (*m_boardData)[m_posX][m_posY].m_analyze = analyse;
}

inline void ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::updateProcessing(bool processing)
{
    (*m_boardData)[m_posX][m_posY].m_processing = processing;
}

inline ConnectXBoard<ConnectXBoardTracker>::Position ConnectXBoard<ConnectXBoardTracker>::SpinIteratorImpl::position() const
{
    return std::make_pair(m_posX,m_posY);
}

inline ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::IteratorImpl(BoardData &boardData,ConnectXBoardTracker& tracker,std::function<void()> scoreUpdater,int xMax, int yMax,int posX, int posY) :
    m_boardData(&boardData),
    m_tracker(&tracker),
    m_scoreUpdater(scoreUpdater),
    m_step(0),
    m_posX(posX),
    m_posY(posY),
    m_xMax(xMax),
    m_yMax(yMax)
{}

inline ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::IteratorImpl(const IteratorImpl &o):
    m_boardData(o.m_boardData),
    m_tracker(o.m_tracker),
    m_scoreUpdater(o.m_scoreUpdater),
    m_step(o.m_step),
    m_posX(o.m_posX),
    m_posY(o.m_posY),
    m_xMax(o.m_xMax),
    m_yMax(o.m_yMax)
{

}

inline ConnectXBoard<ConnectXBoardTracker>::IteratorImpl &ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::operator=(const IteratorImpl &o)
{
    m_boardData = o.m_boardData;
    m_tracker = o.m_tracker;
    m_scoreUpdater = o.m_scoreUpdater;
    m_step = o.m_step;
    m_posX = o.m_posX;
    m_posY = o.m_posY;
    m_xMax = o.m_xMax;
    m_yMax = o.m_yMax;

    return *this;
}

inline const ConnectXBoard<ConnectXBoardTracker>::BoardItemData &ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::operator*()
{
    return (*m_boardData)[m_posX][m_posY];
}

inline const ConnectXBoard<ConnectXBoardTracker>::BoardItemData &ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::operator ->()
{
    return (*m_boardData)[m_posX][m_posY];
}

inline bool ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::operator==(const IteratorImpl &o) const
{
    return (m_posX == o.m_posX) && (m_posY == o.m_posY);
}

inline bool ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::operator!=(const IteratorImpl &o) const
{
    return (m_posX != o.m_posX) || (m_posY != o.m_posY);
}

inline ConnectXBoard<ConnectXBoardTracker>::IteratorImpl &ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::operator++()
{
    if(isEnd())
        return *this;

    iterateOneStep();
    return *this;
}

inline ConnectXBoard<ConnectXBoardTracker>::IteratorImpl ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::operator++(int)
{
    IteratorImpl tmp(*this);
    if(isEnd())
        return tmp;

    iterateOneStep();
    return tmp;
}

inline void ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::iterateOneStep()
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

inline bool ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::isEnd()
{
    return (m_posX == XEnd) && (m_posY == YEnd);
}

inline void ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::update(const BoardItemData  &item)
{
    (*m_boardData)[m_posX][m_posY] = item;
    m_tracker->store({std::make_pair(m_posX,m_posY),item.m_item});
    m_scoreUpdater();
}

inline void ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::update(const BoardItem  &item)
{
    (*m_boardData)[m_posX][m_posY].m_item = item;
    m_tracker->store({std::make_pair(m_posX,m_posY),item});
    m_scoreUpdater();
}

inline void ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::update(bool  analyse)
{
    (*m_boardData)[m_posX][m_posY].m_analyze = analyse;
}

inline void ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::updateProcessing(bool  processing)
{
    (*m_boardData)[m_posX][m_posY].m_processing = processing;
}

inline ConnectXBoard<ConnectXBoardTracker>::Position ConnectXBoard<ConnectXBoardTracker>::IteratorImpl::position() const
{
    return std::make_pair(m_posX,m_posY);
}


inline const ConnectXBoardTracker &bj::connectX::ConnectXBoard<ConnectXBoardTracker>::tracker() const
{
    return m_tracker;
}

inline void bj::connectX::ConnectXBoard<ConnectXBoardTracker>::score()
{
    ConnectXBoardBase::score(m_tracker.last().m_position);
}

}}
