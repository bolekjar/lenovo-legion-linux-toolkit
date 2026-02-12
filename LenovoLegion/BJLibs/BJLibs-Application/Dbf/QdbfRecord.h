// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QVariant>
#include <QString>

namespace bj { namespace framework {

namespace Internal {
class QDbfRecordPrivate;
}

class QDbfField;

class QDbfRecord
{
public:
    QDbfRecord();
    QDbfRecord(const QDbfRecord &other);
    bool operator==(const QDbfRecord &other) const;
    bool operator!=(const QDbfRecord &other) const;
    QDbfRecord &operator=(const QDbfRecord &other);
    ~QDbfRecord();

    void setRecordIndex(int index);
    int recordIndex() const;

    void setValue(int fieldIndex, const QVariant &value);
    QVariant value(int fieldIndex) const;

    void setValue(const QString &fieldName, const QVariant &value);
    QVariant value(const QString &fieldName) const;

    void setNull(int fieldIndex);
    bool isNull(int fieldIndex) const;

    void setNull(const QString &fieldName);
    bool isNull(const QString &fieldName) const;

    int indexOf(const QString &fieldName) const;
    QString fieldName(int fieldIndex) const;

    QDbfField field(int fieldIndex) const;
    QDbfField field(const QString &fieldName) const;

    void append(const QDbfField &field);
    void replace(int pos, const QDbfField &field);
    void insert(int pos, const QDbfField &field);
    void remove(int pos);

    bool isEmpty() const;

    void setDeleted(bool deleted);
    bool isDeleted() const;

    bool contains(int fieldIndex) const;
    bool contains(const QString &fieldName) const;
    void clear();
    void clearValues();
    int count() const;

private:
    Internal::QDbfRecordPrivate *d;
    void detach();
};

}}
