// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QVariant>

namespace bj { namespace framework {

namespace Internal {

class QDbfFieldPrivate;
class QDbfTablePrivate;

}

class QDbfField
{
public:
    explicit QDbfField(const QString &fieldName = QLatin1String(""));
    QDbfField(const QDbfField &other);
    bool operator==(const QDbfField &other) const;
    bool operator!=(const QDbfField &other) const;
    QDbfField &operator=(const QDbfField &other);
    ~QDbfField();

    enum QDbfType
    {
        Undefined = -1,
        Character,
        Date,
        FloatingPoint,
        Logical,
        Memo,
        Number,
        Integer,
        DateTime
    };

    void setValue(const QVariant &value);
    inline QVariant value() const { return val; }

    void setName(const QString &name);
    QString name() const;

    bool isNull() const;

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    void clear();

    void setType(QDbfType type);
    QDbfType type() const;

    void setLength(int length);
    int length() const;

    void setPrecision(int precision);
    int precision() const;

    void setOffset(int offset);
    int offset() const;

    void setDefaultValue(const QVariant &value);
    QVariant defaultValue() const;

private:
    Internal::QDbfFieldPrivate *d;
    QVariant val;
    void detach();

    friend class Internal::QDbfTablePrivate;
};

}}
