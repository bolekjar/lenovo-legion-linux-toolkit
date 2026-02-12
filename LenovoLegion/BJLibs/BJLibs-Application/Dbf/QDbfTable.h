// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include <QString>
#include <QVariant>

namespace bj { namespace framework {


namespace Internal {
class QDbfTablePrivate;
}

class QDbfRecord;

class QDbfTable
{

public:

    enum Codepage {
        CodepageNotSet = 0,
        IBM437,
        IBM850,
        IBM866,
        Windows1250,
        Windows1251,
        Windows1252,
        GB18030,
        Kamenicky,
        UnsupportedCodepage
    };

    enum OpenMode {
        ReadOnly = 0,
    };

    enum DbfTableError {
        NoError = 0,
        FileOpenError,
        FileReadError,
        FileWriteError,
        InvalidValue,
        InvalidIndexError,
        InvalidTypeError,
        UnsupportedFile
    };

    QDbfTable();
    explicit QDbfTable(const QString &dbfFileName);
    ~QDbfTable();

    bool open(const QString &fileName);
    bool open();
    void close();

    QString fileName() const;
    DbfTableError error() const;

    QDbfTable::Codepage codepage() const;
    bool overrideCodepage(QDbfTable::Codepage codepage);

    bool isOpen() const;

    int size() const;
    int at() const;
    bool previous() const;
    bool next() const;
    bool first() const;
    bool last() const;
    bool seek(int index) const;

    QDate lastUpdate() const;

    QDbfRecord record() const;

    QVariant value(int fieldIndex) const;

    QVariant value(const QString &name) const;

private:
    Q_DISABLE_COPY(QDbfTable)
    Internal::QDbfTablePrivate *const d;
};

}}
