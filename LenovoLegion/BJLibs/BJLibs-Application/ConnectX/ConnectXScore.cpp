// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ConnectXScore.h"

namespace bj { namespace connectX {


ConnectXScore::ConnectXScore(){}

ConnectXScore::ConnectXScore(const Attack &xDirection, const Attack &yDirection, const Attack &leftDownDirection, const Attack &leftUpDirection) :
    m_XDirection(xDirection),
    m_YDirection(yDirection),
    m_LeftDownDirection(leftDownDirection),
    m_LeftUpDirection(leftUpDirection)
{}

ConnectXScore::ConnectXScore(const ConnectXScore &o):
    m_XDirection(o.m_XDirection),
    m_YDirection(o.m_YDirection),
    m_LeftDownDirection(o.m_LeftDownDirection),
    m_LeftUpDirection(o.m_LeftUpDirection)
{}

ConnectXScore &ConnectXScore::operator=(const ConnectXScore &o)
{
    m_XDirection = o.m_XDirection;
    m_YDirection = o.m_YDirection;
    m_LeftDownDirection = o.m_LeftDownDirection;
    m_LeftUpDirection = o.m_LeftUpDirection;
    return *this;
}

ConnectXScore ConnectXScore::operator+(const ConnectXScore &o)
{
    return ConnectXScore(m_XDirection + o.m_XDirection,m_YDirection + o.m_YDirection,m_LeftDownDirection + o.m_LeftDownDirection,m_LeftUpDirection + o.m_LeftUpDirection);
}

void ConnectXScore::operator+=(const ConnectXScore &o)
{
    m_XDirection += o.m_XDirection;
    m_YDirection += o.m_YDirection;
    m_LeftDownDirection += o.m_LeftDownDirection;
    m_LeftUpDirection += o.m_LeftUpDirection;
}

bool ConnectXScore::operator==(const ConnectXScore &o) const
{
    return m_XDirection == o.m_XDirection && m_YDirection == o.m_YDirection && m_LeftDownDirection == o.m_LeftDownDirection && m_LeftUpDirection == o.m_LeftUpDirection;
}

void ConnectXScore::set(const Attack &xDirection, const Attack &yDirection, const Attack &leftDownDirection, const Attack &leftUpDirection)
{
    m_XDirection = xDirection;
    m_YDirection = yDirection;
    m_LeftDownDirection     = leftDownDirection;
    m_LeftUpDirection       = leftUpDirection;
}

void ConnectXScore::setXDirectionAttack(const Attack &xDirection)
{
    m_XDirection = xDirection;
}

void ConnectXScore::setYDirectionAttack(const Attack &yDirection)
{
    m_YDirection = yDirection;
}

void ConnectXScore::setLeftDownDirectionAttack(const Attack &leftDownDirection)
{
    m_LeftDownDirection = leftDownDirection;
}

void ConnectXScore::setLeftUpDirectionAttack(const Attack &leftUpDirection)
{
    m_LeftUpDirection = leftUpDirection;
}

void ConnectXScore::clear()
{
    m_XDirection.clear();
    m_YDirection.clear();
    m_LeftDownDirection.clear();
    m_LeftUpDirection.clear();
}

void ConnectXScore::addAttack(const Attack &xDirection, const Attack &yDirection, const Attack &leftDownDirection, const Attack &leftUpDirection)
{
    m_XDirection += xDirection;
    m_YDirection += yDirection;
    m_LeftDownDirection     += leftDownDirection;
    m_LeftUpDirection       += leftUpDirection;
}

void ConnectXScore::addXDirectionAttack(const Attack &xDirection)
{
    m_XDirection += xDirection;
}

void ConnectXScore::addYDirectionAttack(const Attack &yDirection)
{
    m_YDirection += yDirection;
}

void ConnectXScore::addLeftDownDirectionAttack(const Attack &leftDownDirection)
{
    m_LeftDownDirection     += leftDownDirection;
}

void ConnectXScore::addLeftUpDirectionAttack(const Attack &leftUpDirection)
{
    m_LeftUpDirection       += leftUpDirection;
}


ConnectXScore::Attack ConnectXScore::attack() const
{
    Attack tmp;

    tmp = m_XDirection + m_YDirection + m_LeftDownDirection + m_LeftUpDirection;

    return tmp;
}


ConnectXScore::AttackImpl ConnectXScore::AttackImpl::operator +(const AttackImpl &r) const
{
    return AttackImpl(m_oAttack + r.m_oAttack,m_xAttack + r.m_xAttack);
}

void ConnectXScore::AttackImpl::operator+=(const AttackImpl &r)
{
    m_oAttack += r.m_oAttack;
    m_xAttack += r.m_xAttack;
}

bool ConnectXScore::AttackImpl::operator==(const AttackImpl &r) const
{
    return m_oAttack == r.m_oAttack && m_xAttack == r.m_xAttack;
}

void ConnectXScore::AttackImpl::clear()
{
    m_oAttack = 0;
    m_xAttack = 0;
}

const ConnectXScore::AttackImpl::SignedNumber ConnectXScore::AttackImpl::getOAttack() const
{
    return m_oAttack;
}

const ConnectXScore::AttackImpl::SignedNumber ConnectXScore::AttackImpl::getXAttack() const
{
    return m_xAttack;
}

const ConnectXScore::AttackImpl::SignedNumber ConnectXScore::AttackImpl::getOXAttack() const
{
    return m_oAttack - m_xAttack;
}

const ConnectXScore::AttackImpl::SignedNumber ConnectXScore::AttackImpl::getXOAttack() const
{
    return m_xAttack - m_oAttack;
}

ConnectXScore::AttackImpl::AttackImpl() :
    m_oAttack(0),
    m_xAttack(0)
{}

ConnectXScore::AttackImpl::AttackImpl(const SignedNumber &oAttack, const SignedNumber &xAttack) :
    m_oAttack(oAttack),
    m_xAttack(xAttack)
{}

ConnectXScore::AttackImpl::AttackImpl(const AttackImpl &o):
    m_oAttack(o.m_oAttack),
    m_xAttack(o.m_xAttack)
{}

ConnectXScore::AttackImpl &ConnectXScore::AttackImpl::operator=(const AttackImpl &o)
{
    m_oAttack = o.m_oAttack;
    m_xAttack = o.m_xAttack;

    return *this;
}


}}
