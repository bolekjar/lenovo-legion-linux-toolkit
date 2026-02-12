// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "QdbfRecord.h"
#include "QDbfField.h"

#include <QDebug>
#include <QVariant>
#include <QVector>

namespace bj { namespace framework {

namespace Internal {

class QDbfRecordPrivate
{
public:
    QDbfRecordPrivate();
    QDbfRecordPrivate(const QDbfRecordPrivate &other);

    QAtomicInt ref;
    int m_index;
    bool m_isDeleted;
    QVector<QDbfField> m_fields;
};

QDbfRecordPrivate::QDbfRecordPrivate() :
    ref(1),
    m_index(-1),
    m_isDeleted(false)
{
}

QDbfRecordPrivate::QDbfRecordPrivate(const QDbfRecordPrivate &other) :
    ref(1),
    m_index(other.m_index),
    m_isDeleted(other.m_isDeleted),
    m_fields(other.m_fields)
{
}

} // namespace Internal

QDbfRecord::QDbfRecord() :
  d(new Internal::QDbfRecordPrivate())
{
}

QDbfRecord::QDbfRecord(const QDbfRecord &other) :
    d(other.d)
{
    d->ref.ref();
}

QDbfRecord &QDbfRecord::operator=(const QDbfRecord &other)
{
    if (this == &other) {
        return *this;
    }

    qAtomicAssign(d, other.d);
    return *this;
}

bool QDbfRecord::operator==(const QDbfRecord &other) const
{
    return (recordIndex() == other.recordIndex() &&
            isDeleted() == other.isDeleted() &&
            d->m_fields == other.d->m_fields);
}

bool QDbfRecord::operator!=(const QDbfRecord& other) const
{
    return !operator==(other);
}

QDbfRecord::~QDbfRecord()
{
    if (!d->ref.deref()) {
        delete d;
    }
}

void QDbfRecord::setRecordIndex(int index)
{
    d->m_index = index;
}

int QDbfRecord::recordIndex() const
{
    return d->m_index;
}

void QDbfRecord::setValue(int fieldIndex, const QVariant &value)
{
    if (!contains(fieldIndex)) {
        return;
    }

    detach();
    d->m_fields[fieldIndex].setValue(value);
}

QVariant QDbfRecord::value(int fieldIndex) const
{
    return d->m_fields.value(fieldIndex).value();
}

void QDbfRecord::setValue(const QString &fieldName, const QVariant &value)
{
    setValue(indexOf(fieldName), value);
}

QVariant QDbfRecord::value(const QString &fieldName) const
{
    return value(indexOf(fieldName));
}

void QDbfRecord::setNull(int fieldIndex)
{
    if (!contains(fieldIndex)) {
        return;
    }

    detach();
    d->m_fields[fieldIndex].clear();
}

bool QDbfRecord::isNull(int fieldIndex) const
{
    return d->m_fields.value(fieldIndex).isNull();
}

void QDbfRecord::setNull(const QString &fieldName)
{
    setNull(indexOf(fieldName));
}

bool QDbfRecord::isNull(const QString &fieldName) const
{
    return isNull(indexOf(fieldName));
}

int QDbfRecord::indexOf(const QString &fieldName) const
{
    const QString &name = fieldName.toUpper();

    for (int i = 0; i < count(); ++i) {
        if (d->m_fields.at(i).name().toUpper() == name) {
            return i;
        }
    }

    return -1;
}

QString QDbfRecord::fieldName(int fieldIndex) const
{
    return d->m_fields.value(fieldIndex).name();
}

QDbfField QDbfRecord::field(int fieldIndex) const
{
    return d->m_fields.value(fieldIndex);
}

QDbfField QDbfRecord::field(const QString &fieldName) const
{
    return field(indexOf(fieldName));
}

void QDbfRecord::append(const QDbfField &field)
{
    detach();
    d->m_fields.append(field);
}

void QDbfRecord::replace(int pos, const QDbfField &field)
{
    if (!contains(pos)) {
        return;
    }

    detach();
    d->m_fields[pos] = field;
}

void QDbfRecord::insert(int pos, const QDbfField &field)
{
    detach();
    d->m_fields.insert(pos, field);
}

void QDbfRecord::remove(int pos)
{
    if (!contains(pos)) {
        return;
    }

    detach();
    d->m_fields.remove(pos);
}

bool QDbfRecord::isEmpty() const
{
    return d->m_fields.isEmpty();
}

void QDbfRecord::setDeleted(bool deleted)
{
    detach();
    d->m_isDeleted = deleted;
}

bool QDbfRecord::isDeleted() const
{
    return d->m_isDeleted;
}

bool QDbfRecord::contains(int fieldIndex) const
{
    return fieldIndex >= 0 && fieldIndex < d->m_fields.count();
}

bool QDbfRecord::contains(const QString &fieldName) const
{
    return (indexOf(fieldName) >= 0);
}

void QDbfRecord::clear()
{
    detach();
    d->m_fields.clear();
}

void QDbfRecord::clearValues()
{
    detach();
    int count = d->m_fields.count();
    for (int i = 0; i < count; ++i) {
        d->m_fields[i].clear();
    }
}

int QDbfRecord::count() const
{
    return d->m_fields.count();
}

void QDbfRecord::detach()
{
    qAtomicDetach(d);
}

}}
