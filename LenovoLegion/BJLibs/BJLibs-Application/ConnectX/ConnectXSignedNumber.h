// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ExceptionBuilder.h>

#include <limits>
#include <ostream>
#include <string>

namespace bj { namespace connectX {


template <typename Num>
class ConnectXSignedNumber
{
    using NumberType = Num;


    static constexpr NumberType plusInf  = std::numeric_limits<NumberType>::max();
    static constexpr NumberType minusInf = std::numeric_limits<NumberType>::min() + 1;
    static constexpr NumberType undef    = std::numeric_limits<NumberType>::min();
    static constexpr NumberType max      = std::numeric_limits<NumberType>::max() - 1;
    static constexpr NumberType min      = std::numeric_limits<NumberType>::min() + 2;

public:

    template<typename T>
    friend std::ostream& operator<< (std::ostream& out, const ConnectXSignedNumber<T>& number);


    DEFINE_EXCEPTION(ConnectXSignedNumber)

    enum ERROR_CODES : int {
        ARTMETIC_OPERATION_ERROR = 1,
        OVERFLOW_ERROR           = 2
    };


    static ConnectXSignedNumber getPlusInf() ;
    static ConnectXSignedNumber getMinusInf();
    static ConnectXSignedNumber getUndef()   ;
    static ConnectXSignedNumber getMax()     ;
    static ConnectXSignedNumber getMin()     ;
    static ConnectXSignedNumber pow(const ConnectXSignedNumber<Num>& x, unsigned int y);
    static std::string toString(const ConnectXSignedNumber<Num>& x);

public:

    ConnectXSignedNumber()                            = default;
    ConnectXSignedNumber(const ConnectXSignedNumber&) = default;
    ConnectXSignedNumber(ConnectXSignedNumber&&)      = default;

    ConnectXSignedNumber(const NumberType &  value);

    ConnectXSignedNumber& operator =(const ConnectXSignedNumber&) = default;
    ConnectXSignedNumber& operator =(ConnectXSignedNumber&&)      = default;

    ConnectXSignedNumber  operator+  (const ConnectXSignedNumber& r) const;
    ConnectXSignedNumber  operator-  (const ConnectXSignedNumber& r) const;
    ConnectXSignedNumber  operator*  (const ConnectXSignedNumber& r) const;
    ConnectXSignedNumber  operator/  (const ConnectXSignedNumber& r) const;
    ConnectXSignedNumber  operator%  (const ConnectXSignedNumber& r) const;

    void        operator++ ();
    void        operator++ (int);
    void        operator-- ();
    void        operator-- (int);


    void        operator+= (const ConnectXSignedNumber& r);
    void        operator-= (const ConnectXSignedNumber& r);
    void        operator*= (const ConnectXSignedNumber& r);
    void        operator/= (const ConnectXSignedNumber& r);
    void        operator%= (const ConnectXSignedNumber& r);

    bool        operator== (const ConnectXSignedNumber& r) const;
    bool        operator!= (const ConnectXSignedNumber& r) const;
    bool        operator<  (const ConnectXSignedNumber& r) const;
    bool        operator>  (const ConnectXSignedNumber& r) const;
    bool        operator>= (const ConnectXSignedNumber& r) const;
    bool        operator<= (const ConnectXSignedNumber& r) const;


    ConnectXSignedNumber        operator&  (const ConnectXSignedNumber& r) const;
    ConnectXSignedNumber        operator|  (const ConnectXSignedNumber& r) const;
    ConnectXSignedNumber        operator^  (const ConnectXSignedNumber& r) const;
    ConnectXSignedNumber        operator~  ()      const;
    ConnectXSignedNumber        operator<< (int r) const;
    ConnectXSignedNumber        operator>> (int r) const;


    NumberType value() const;

private:

    static_assert(plusInf  > 0);
    static_assert(minusInf < 0);

private:

    NumberType m_value = 0;
};

template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::getPlusInf()
{
    return ConnectXSignedNumber(plusInf);
}

template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::getMinusInf()
{
    return ConnectXSignedNumber(minusInf);
}


template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::getUndef()
{
    return ConnectXSignedNumber(undef);
}


template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::getMax()
{
    return ConnectXSignedNumber(max);
}

template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::getMin()
{
    return ConnectXSignedNumber(min);
}

template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::pow(const ConnectXSignedNumber<Num>& x, unsigned int y)
{
    ConnectXSignedNumber<Num> ret = 1;

    for(unsigned int i = 0; i < y;i++)
    {
        ret *= x;
    }

    return ret;
}

 template<typename Num>
std::string ConnectXSignedNumber<Num>::toString(const ConnectXSignedNumber<Num> &x)
{
    return std::to_string(x.m_value);
}

template<typename Num>
ConnectXSignedNumber<Num>::ConnectXSignedNumber(const NumberType &value) :
    m_value(value)
{}

template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::operator+(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    switch (m_value)
    {
    case plusInf:
    {
        if(r.m_value == minusInf)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
        }

        return ConnectXSignedNumber(plusInf);
        break;
    }
    case minusInf:
    {
        if(r.m_value == plusInf)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error!")
        }

        return ConnectXSignedNumber(minusInf);
        break;
    }
    default:
    {
        if(r.m_value == plusInf)
        {
            return ConnectXSignedNumber(plusInf);
        }


        if(r.m_value == minusInf)
        {
            return ConnectXSignedNumber(minusInf);
        }

        NumberType result = m_value + r.m_value;
        if(m_value > 0 && r.m_value > 0 && (result < 0 || result >= max)) {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
         }
        if(m_value < 0 && r.m_value < 0 && (result > 0 || result <= min)) {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
        }
        return ConnectXSignedNumber(result);
        break;
    }
    }
}


template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::operator-(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    switch (m_value)
    {
    case plusInf:
    {
        if(r.m_value == plusInf)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
        }

        return ConnectXSignedNumber(plusInf);
        break;
    }
    case minusInf:
    {
        if(r.m_value == minusInf)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error!")
        }

        return ConnectXSignedNumber(minusInf);
        break;
    }
    default:
    {

        if(r.m_value == plusInf)
        {
            return ConnectXSignedNumber(minusInf);
        }


        if(r.m_value == minusInf)
        {
            return ConnectXSignedNumber(plusInf);
        }


        NumberType result = m_value - r.m_value;
        if(m_value < 0 && r.m_value > 0 && (result > 0 || result <= min)) {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
        }
        if(m_value > 0 && r.m_value < 0 && (result < 0 || result >= max)) {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
        }
        return ConnectXSignedNumber(result);
        break;
    }
    }
}

template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::operator*(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }


    switch (m_value)
    {
    case plusInf:
    {
        if(r.m_value < 0)
        {
            return ConnectXSignedNumber(minusInf);
        }

        if(r.m_value  == 0)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
        }

        return ConnectXSignedNumber(plusInf);
        break;
    }
    case minusInf:
    {
        if(r.m_value < 0)
        {
            return ConnectXSignedNumber(plusInf);
        }

        if(r.m_value  == 0)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
        }

        return ConnectXSignedNumber(minusInf);
        break;
    }
    default:
    {

        if(r.m_value == plusInf)
        {
            if(m_value < 0)
            {
                return ConnectXSignedNumber(minusInf);
            }

            if(m_value  == 0)
            {
                THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
            }

            return ConnectXSignedNumber(plusInf);
        }


        if(r.m_value == minusInf)
        {

            if(m_value < 0)
            {
                return ConnectXSignedNumber(plusInf);
            }


            if(m_value  == 0)
            {
                THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
            }


            return ConnectXSignedNumber(minusInf);
        }


        NumberType result = m_value * r.m_value;

        if(m_value != 0 && r.m_value != 0)
        {
            if (m_value != result / r.m_value)
            {
                THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
            }
        }


        return ConnectXSignedNumber(result);
        break;
    }
    }

}

template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::operator/(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    if(r.m_value == 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    switch (m_value)
    {
    case plusInf:
    {
        if(r.m_value < 0)
        {
            return ConnectXSignedNumber(minusInf);
        }

        if(r.m_value == plusInf || r.m_value == minusInf)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
        }

        return ConnectXSignedNumber(plusInf);
        break;
    }
    case minusInf:
    {
        if(r.m_value < 0)
        {
            return ConnectXSignedNumber(plusInf);
        }

        if(r.m_value == plusInf || r.m_value == minusInf)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
        }

        return ConnectXSignedNumber(minusInf);
        break;
    }
    default:
    {

        if(r.m_value == plusInf)
        {

            return ConnectXSignedNumber(0);
        }


        if(r.m_value == minusInf)
        {

            return ConnectXSignedNumber(0);
        }

        return ConnectXSignedNumber(m_value /r.m_value);
        break;
    }
    }
}


template<typename Num>
ConnectXSignedNumber<Num> ConnectXSignedNumber<Num>::operator%(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    if(m_value == plusInf     ||
       m_value == minusInf    ||
       r.m_value == plusInf   ||
       r.m_value == minusInf
      )
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    return m_value % r.m_value;
}

template<typename Num>
void ConnectXSignedNumber<Num>::operator++()
{
    if(m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    if(m_value == plusInf || m_value == minusInf)
    {
        return;
    }

    if(m_value == max)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
    }

    ++m_value;
}

template<typename Num>
void ConnectXSignedNumber<Num>::operator--()
{
    if(m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    if(m_value == plusInf || m_value == minusInf)
    {
        return;
    }

    if(m_value == min)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
    }

    --m_value;
}

template<typename Num>
void ConnectXSignedNumber<Num>::operator+=(const ConnectXSignedNumber &r)
{
    m_value = (ConnectXSignedNumber(m_value) + ConnectXSignedNumber(r.m_value)).m_value;
}

template<typename Num>
void ConnectXSignedNumber<Num>::operator-=(const ConnectXSignedNumber &r)
{
    m_value = (ConnectXSignedNumber(m_value) - ConnectXSignedNumber(r.m_value)).m_value;
}

template<typename Num>
void ConnectXSignedNumber<Num>::operator*=(const ConnectXSignedNumber &r)
{
    m_value = (ConnectXSignedNumber(m_value) * ConnectXSignedNumber(r.m_value)).m_value;
}
template<typename Num>
void ConnectXSignedNumber<Num>::operator/=(const ConnectXSignedNumber &r)
{
    m_value = (ConnectXSignedNumber(m_value) / ConnectXSignedNumber(r.m_value)).m_value;
}


template<typename Num>
void ConnectXSignedNumber<Num>::operator%=(const ConnectXSignedNumber &r)
{
    m_value = (ConnectXSignedNumber(m_value) % ConnectXSignedNumber(r.m_value)).m_value;
}



template<typename Num>
void ConnectXSignedNumber<Num>::operator--(int)
{
    if(m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    if(m_value == plusInf || m_value == minusInf)
    {
        return;
    }

    if(m_value == min)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
    }

    m_value--;
}

template<typename Num>
void ConnectXSignedNumber<Num>::operator++(int)
{
    if(m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: aritmetic operation error !")
    }

    if(m_value == plusInf || m_value == minusInf)
    {
        return;
    }

    if(m_value == max)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: overflow error !")
    }

    m_value++;
}




template<typename Num>
bool ConnectXSignedNumber<Num>::operator==(const ConnectXSignedNumber &r) const
{
    return m_value == r.m_value;
}

template<typename Num>
bool ConnectXSignedNumber<Num>::operator!=(const ConnectXSignedNumber &r) const
{
    return m_value != r.m_value;
}

template<typename Num>
bool ConnectXSignedNumber<Num>::operator<(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: compare error !")
    }

    return m_value < r.m_value;
}

template<typename Num>
bool ConnectXSignedNumber<Num>::operator>(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: compare error !")
    }

    return m_value > r.m_value;
}

template<typename Num>
bool ConnectXSignedNumber<Num>::operator>=(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: compare error !")
    }

    return m_value >= r.m_value;
}

template<typename Num>
bool ConnectXSignedNumber<Num>::operator<=(const ConnectXSignedNumber &r) const
{
    if(m_value == undef || r.m_value == undef)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::ARTMETIC_OPERATION_ERROR,"Logic error: compare error !")
    }

    return m_value <= r.m_value;
}

template<typename Num>
typename ConnectXSignedNumber<Num>::NumberType ConnectXSignedNumber<Num>::value() const
{
    return m_value;
}

template<typename T>
std::ostream& operator<<(std::ostream& out,const ConnectXSignedNumber<T>&  number)
{
   return out << number.m_value;
}


}}
