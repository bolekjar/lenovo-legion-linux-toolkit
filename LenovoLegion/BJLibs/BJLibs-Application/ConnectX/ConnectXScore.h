// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <ConnectX/ConnectXSignedNumber.h>

namespace bj { namespace connectX {

class ConnectXScore
{

public:

    class AttackImpl {

    public:

        using SignedNumber = ConnectXSignedNumber<int64_t>;

    public:

        AttackImpl();
        AttackImpl(const SignedNumber& oAttack,const SignedNumber& xAttack);
        AttackImpl(const AttackImpl& o);

        AttackImpl& operator=(const AttackImpl& o);
        AttackImpl  operator+(const AttackImpl& r) const;
        void        operator+=(const AttackImpl& r);
        bool        operator==(const AttackImpl& r) const;

        void clear();


        const SignedNumber  getOAttack()  const;
        const SignedNumber  getXAttack()  const;
        const SignedNumber  getOXAttack() const;
        const SignedNumber  getXOAttack() const;

    private:

        SignedNumber m_oAttack;
        SignedNumber m_xAttack;
    };

public:

    using Attack = AttackImpl;

public:

    ConnectXScore();
    ConnectXScore(const Attack& xDirection,const Attack& yDirection,const Attack& leftDownDirection,const Attack& leftUpDirection);
    ConnectXScore(const ConnectXScore& o);

    ConnectXScore& operator=(const ConnectXScore& o);
    ConnectXScore  operator+(const ConnectXScore& o);
    void           operator+=(const ConnectXScore& o);
    bool           operator==(const ConnectXScore& o) const;


    void set(const Attack& xDirection,const Attack& yDirection,const Attack& leftDownDirection,const Attack& leftUpDirection);
    void setXDirectionAttack(const Attack& xDirection);
    void setYDirectionAttack(const Attack& yDirection);
    void setLeftDownDirectionAttack(const Attack& leftDownDirection);
    void setLeftUpDirectionAttack(const Attack& leftUpDirection);

    void clear();

    void addAttack(const Attack& xDirection,const Attack& yDirection,const Attack& leftDownDirection,const Attack& leftUpDirection);
    void addXDirectionAttack(const Attack& xDirection);
    void addYDirectionAttack(const Attack& yDirection);
    void addLeftDownDirectionAttack(const Attack& leftDownDirection);
    void addLeftUpDirectionAttack(const Attack& leftUpDirection);

    Attack attack() const;

private:

    Attack m_XDirection;
    Attack m_YDirection;
    Attack m_LeftDownDirection;
    Attack m_LeftUpDirection;

};

}}
