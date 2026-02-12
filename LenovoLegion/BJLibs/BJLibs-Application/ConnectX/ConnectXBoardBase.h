// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ConnectXBoardTypes.h"
#include "ConnectXScore.h"

#include <Core/ExceptionBuilder.h>
#include <QVector>

namespace bj { namespace connectX {


class ConnectXBoardBase
{

    struct ConnectXAnalyseResult
    {
        ConnectXScore::Attack::SignedNumber m_oAttack;
        ConnectXScore::Attack::SignedNumber m_xAttack;
        ConnectXScore::Attack::SignedNumber m_empty;
    };

public:

    using BoardItem             = bj::connectX::BoardItem;
    using BoardItemData         = bj::connectX::BoardItemData;
    using BoardData             = QVector<QVector<BoardItemData>>;
    using BoardDataScore        = QVector<QVector<ConnectXScore>>;
    using Position              = std::pair<int,int>;

    static const int XEnd;
    static const int YEnd;
    static const std::array<int64_t,6> ConnectXScorePowFactor;

private:

    class ConstIteratorImpl {

    public:

        using  iterator_category  = std::forward_iterator_tag;

    public:

        ConstIteratorImpl(const BoardData& boardData, int xMax, int yMax,int posX, int posY);
        ConstIteratorImpl(const ConstIteratorImpl &o);
        const ConstIteratorImpl& operator=(const ConstIteratorImpl &o);
        const BoardItemData  &operator*() const;
        const BoardItemData  &operator->() const;
        bool operator==(const ConstIteratorImpl &o) const;
        bool operator!=(const ConstIteratorImpl &o) const;
        const ConstIteratorImpl &operator++();
        const ConstIteratorImpl  operator++(int);

        Position position() const;

    private:

        void iterateOneStep();
        bool isEnd();

        const BoardData*            m_boardData;
        int                         m_step;
        int                         m_posX;
        int                         m_posY;
        int                         m_xMax;
        int                         m_yMax;
    };


public:

    using ConstIterator = ConstIteratorImpl;

public:

    ConnectXBoardBase(int xMax, int yMax, int connectX);
    ConnectXBoardBase(const ConnectXBoardBase& o);

    virtual ~ConnectXBoardBase();

    const ConstIterator     cbegin() const;
    const ConstIterator     cend()   const;

    BoardItemData           get(const Position& pos)  const;
    bool                    isEmpty()                 const;
    const BoardDataScore&   dataScore()               const;
    ConnectXScore           overallScore()            const;

    bool                    isScorable(const Position& pos,const BoardItem& move)       const;
    bool                    isRated(const Position& pos)                                const;
    bool                    isConnectedX(QVector<std::pair<int, int> > &winnerPosition) const;
    bool                    isDraw()                                                    const;

private:

    ConnectXScore::Attack scoreXDirection(const Position& position,ConnectXScore::Attack::SignedNumber* emptyAttack = nullptr)                 const;
    ConnectXScore::Attack scoreYDirection(const Position& position,ConnectXScore::Attack::SignedNumber* emptyAttack = nullptr)                 const;
    ConnectXScore::Attack scoreLeftDownDirection(const Position& position,ConnectXScore::Attack::SignedNumber* emptyAttack = nullptr)          const;
    ConnectXScore::Attack scoreLeftUpDirection(const Position& position,ConnectXScore::Attack::SignedNumber* emptyAttack = nullptr)            const;
    ConnectXAnalyseResult analyzeConnectX(const std::vector<BoardItem> &connectX)   const;

protected:

    void                  score(const Position& position);
    void                  validatePositions(const int posX,const int posY) const;

protected:

    BoardData                                m_data;
    BoardDataScore                           m_dataScore;
    ConnectXScore                            m_overallScore;

public:

    enum ERROR_CODES : int {
        INVALID_PARAMETER = 0
    };

    DEFINE_EXCEPTION(ConnectXBoard)

    const int m_xMax;
    const int m_yMax;
    const int m_connectX;

    static const ConnectXScore::Attack::SignedNumber minusInf;
    static const ConnectXScore::Attack::SignedNumber plusInf;
    static const ConnectXScore::Attack::SignedNumber undef;
    const ConnectXScore::Attack::SignedNumber m_minusWinerScore;
    const ConnectXScore::Attack::SignedNumber m_plusWinerScore;
};


}}

