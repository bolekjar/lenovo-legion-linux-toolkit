// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "QDbfTable.h"
#include "QDbfField.h"
#include "QdbfRecord.h"

#include <QDataStream>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>

static const quint16 DBC_LENGTH = 263;
static const quint8 TERMINATOR_LENGTH = 1;

static const quint8 TABLE_DESCRIPTOR_LENGTH = 32;
static const quint8 TABLE_RECORDS_COUNT_OFFSET = 4;
static const quint8 TABLE_FIRST_RECORD_POSITION_OFFSET = 8;
static const quint8 RECORD_LENGTH_OFFSET = 10;
static const quint8 CODEPAGE_OFFSET = 29;

static const quint8 FIELD_DESCRIPTOR_LENGTH = 32;
static const quint8 FIELD_NAME_LENGTH = 10;
static const quint8 FIELD_LENGTH_OFFSET = 16;

static const quint8 MEMO_BLOCK_LENGTH_OFFSET = 6;
static const quint16 MEMO_DBT_BLOCK_LENGTH = 512;
static const quint8 MEMO_SIGNATURE_TEXT = 1;

static const quint8 FIELD_TYPE_CHARACTER = 0x43;      // C
static const quint8 FIELD_TYPE_DATE = 0x44;           // D
static const quint8 FIELD_TYPE_FLOATING_POINT = 0x46; // F
static const quint8 FIELD_TYPE_LOGICAL = 0x4C;        // L
static const quint8 FIELD_TYPE_MEMO = 0x4D;           // M
static const quint8 FIELD_TYPE_NUMBER = 0x4E;         // N
static const quint8 FIELD_TYPE_INTEGER = 0x49;        // I
static const quint8 FIELD_TYPE_DATE_TIME = 0x54;      // T

static const quint8 CODEPAGE_NOT_SET = 0x00;
static const quint8 CODEPAGE_US_MSDOS = 0x01;
static const quint8 CODEPAGE_INTERNATIONAL_MSDOD = 0x02;
static const quint8 CODEPAGE_RUSSIAN_OEM = 0x26;
static const quint8 CODEPAGE_RUSSIAN_MSDOS = 0x65;
static const quint8 CODEPAGE_EASTERN_EUROPEAN_WINDOWS = 0xC8;
static const quint8 CODEPAGE_RUSSIAN_WINDOWS = 0xC9;
static const quint8 CODEPAGE_WINDOWS_ANSI_LATIN_1 = 0x03;
static const quint8 CODEPAGE_GB18030 = 0x7A;
static const quint8 CODEPAGE_KAMENICKY = 0x68;

static const quint8 LOGICAL_UNDEFINED = 0x3F; // ?
static const quint8 LOGICAL_YES = 0x59;       // Y
static const quint8 LOGICAL_NO = 0x4E;        // N
static const quint8 LOGICAL_TRUE = 0x54;      // T
static const quint8 LOGICAL_FALSE = 0x46;     // F
static const quint8 FIELD_DELETED = 0x2A;     // *
static const quint8 FIELD_NAME_SPACER = 0x00;
static const char END_OF_DBASE_MEMO_BLOCK[] = { 0x1A, 0x1A };

static const quint8 DATE_LENGTH = 8;
static const quint8 YEAR_OFFSET = 0;
static const quint8 YEAR_LENGTH = 4;
static const quint8 MONTH_OFFSET = 4;
static const quint8 MONTH_LENGTH = 2;
static const quint8 DAY_OFFSET = 6;
static const quint8 DAY_LENGTH = 2;

static const quint8 TIME_LENGTH = 6;
static const quint8 HOUR_OFFSET = 0;
static const quint8 HOUR_LENGTH = 2;
static const quint8 MINUTE_OFFSET = 2;
static const quint8 MINUTE_LENGTH = 2;
static const quint8 SECOND_OFFSET = 4;
static const quint8 SECOND_LENGTH = 2;

static const quint8 DATETIME_LENGTH = 14;
static const quint8 DATETIME_DATE_OFFSET = 0;
static const quint8 DATETIME_TIME_OFFSET = 8;

static const quint8 TIMESTAMP_LENGTH = 8;


namespace bj { namespace framework {

namespace Internal {


static unsigned char kamenToCE[] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
    0x89,0x9f,0x8e,0x93,0x8a,0x91,0xe8,0x8b,0x9e,0x9d,0x4c,0xea,0x6c,0x6c,0x80,0xe7,
    0x83,0xec,0xeb,0x99,0x9a,0xee,0xf3,0xf2,0xf9,0x85,0x86,0xe1,0x4c,0xf8,0xdb,0xe9,
    0x87,0x92,0x97,0x9c,0xcb,0xc5,0xf1,0xef,0xe4,0xde,0x72,0x52,0xd7,0xa4,0xc7,0xc8,
    0xca,0x8c,0xf7,0xd7,0xd7,0xa7,0xa3,0xd7,0xd7,0x81,0xd7,0xd7,0x61,0xd7,0xd7,0xc2,
    0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd8,0xb8,0xdf,0xd7,0xbf,0xd7,0xc0,0x8c,0xd7,
    0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0x88,0xd7,0xd7,0x95,0xd7,0xd7,0xd7,0xd7,
    0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,
    0xd7,0xd7,0xb3,0xb2,0xd7,0xd7,0xd6,0x7e,0x27,0x84,0xd7,0xc3,0xd7,0xd7,0xd7,0xd7
};

static unsigned char CEtoCP1250[] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
    0xc4,0x14,0x14,0xc9,0xa5,0xd6,0xdc,0xe1,0xb9,0xc8,0xe4,0xe8,0xc6,0xe6,0xe9,0x8f,
    0x9f,0xcf,0xed,0xef,0x14,0x14,0x14,0xf3,0x14,0xf4,0xf6,0x14,0xfa,0xcc,0xec,0xfc,
    0x86,0xb0,0xca,0x14,0xa7,0xb7,0xb6,0xdf,0xae,0xa9,0x99,0xea,0xa8,0x14,0x14,0x14,
    0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0xb3,0x14,0x14,0xbc,0xbe,0xc5,0xe5,0x14,
    0x14,0xd1,0xac,0x14,0xf1,0xd2,0x14,0xab,0xbb,0x85,0xca,0xf2,0xd5,0x14,0xf5,0x14,
    0x96,0x97,0x20,0x93,0x20,0x20,0xf7,0x14,0x14,0xc0,0xe0,0xd8,0x8b,0x9b,0xf8,0x14,
    0x14,0x8a,0x82,0x84,0x9a,0x8c,0x9c,0xc1,0x8d,0x9d,0xcd,0x8e,0x9e,0x14,0xd3,0xd4,
    0x14,0xd9,0xda,0xf9,0xdb,0xfb,0x14,0x14,0xdd,0xfd,0x14,0xaf,0xa3,0xbf,0x14,0xa1
};

class QTextCodecKamenicky : public QTextCodec
{
    virtual QByteArray name() const {
        return QByteArray{"Kamenicky"};
    }

    virtual int mibEnum() const {
        return 666;
    }

protected:

    virtual QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    virtual QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;
};


QString QTextCodecKamenicky::convertToUnicode(const char *in, int length, ConverterState *) const {
    QTextCodec *codec = QTextCodec::codecForName("Windows-1250");
    QByteArray  data(in,length);


    for(char& cur : data)
    {
        cur =  static_cast<char>(kamenToCE[static_cast<unsigned char>(cur)]);
        cur =  static_cast<char>(CEtoCP1250[static_cast<unsigned char>(cur)]);
    }


    return codec->toUnicode(data);
}


QByteArray QTextCodecKamenicky::convertFromUnicode(const QChar *, int, ConverterState *) const {
    return QByteArray();
}


static QTextCodecKamenicky kamenicky;



class QDbfTablePrivate
{
public:
    QDbfTablePrivate();
    explicit QDbfTablePrivate(const QString &dbfFileName);

    enum QDbfMemoType
    {
        NoMemo,
        DBaseMemo,
        DBaseIVMemo,
        FoxProMemo
    };

    enum Location
    {
        BeforeFirstRow = -1,
        FirstRow = 0
    };

    void clear();
    bool openMemoFile();
    QVariant memoFieldValue(int index) const;
    QDataStream::ByteOrder memoByteOrder() const;
    bool overrideCodepage(QDbfTable::Codepage m_codepage);
    bool isValueValid(int i, const QVariant &value) const;
    void setTextCodec();
    QDate dateFromByteArray(const QByteArray &byteArray) const;
    QTime timeFromByteArray(const QByteArray &byteArray) const;
    void setLastUpdate();

    QString m_tableFileName;
    mutable QFile m_tableFile;
    mutable QFile m_memoFile;
    mutable QDbfTable::DbfTableError m_error;
    QTextCodec *m_textCodec;
    QDbfMemoType m_memoType;
    QDate m_lastUpdate;
    QDbfTable::Codepage m_codepage;
    quint16 m_headerLength;
    quint16 m_recordLength;
    quint16 m_fieldsCount;
    qint16 m_memoBlockLength;
    qint32 m_memoNextFreeBlockIndex;
    qint32 m_recordsCount;
    mutable qint32 m_currentIndex;
    mutable QDbfRecord m_currentRecord;
    QDbfRecord m_record;
    mutable bool m_bufered;
    bool m_dbc;
};

QDbfTablePrivate::QDbfTablePrivate() :
    m_error(QDbfTable::NoError),
    m_textCodec(QTextCodec::codecForLocale()),
    m_memoType(QDbfTablePrivate::NoMemo),
    m_codepage(QDbfTable::CodepageNotSet),
    m_headerLength(0),
    m_recordLength(0),
    m_fieldsCount(0),
    m_memoBlockLength(0),
    m_memoNextFreeBlockIndex(0),
    m_recordsCount(0),
    m_currentIndex(0),
    m_bufered(false),
    m_dbc(false)
{
}

QDbfTablePrivate::QDbfTablePrivate(const QString &dbfFileName) :
    m_tableFileName(dbfFileName),
    m_error(QDbfTable::NoError),
    m_textCodec(QTextCodec::codecForLocale()),
    m_memoType(QDbfTablePrivate::NoMemo),
    m_codepage(QDbfTable::CodepageNotSet),
    m_headerLength(0),
    m_recordLength(0),
    m_fieldsCount(0),
    m_memoBlockLength(0),
    m_memoNextFreeBlockIndex(0),
    m_recordsCount(0),
    m_currentIndex(0),
    m_bufered(false),
    m_dbc(false)
{
}

void QDbfTablePrivate::clear()
{
    m_error = QDbfTable::NoError;
    m_dbc = false;
    m_memoType = QDbfTablePrivate::NoMemo;
    m_codepage = QDbfTable::CodepageNotSet;
    m_headerLength = 0;
    m_recordLength = 0;
    m_fieldsCount = 0;
    m_recordsCount = 0;
    m_memoNextFreeBlockIndex = 0;
    m_memoBlockLength = 0;
    m_currentIndex = 0;
    m_bufered = false;
    m_currentRecord = QDbfRecord();
    m_record = QDbfRecord();
}

bool QDbfTablePrivate::openMemoFile()
{
    QString memoFileExtension;
    switch (m_memoType) {
    case QDbfTablePrivate::DBaseMemo:
    case QDbfTablePrivate::DBaseIVMemo:
        memoFileExtension = QLatin1String("dbt");
        break;
    case QDbfTablePrivate::FoxProMemo:
        memoFileExtension = QLatin1String("fpt");
        break;
    default:
        Q_ASSERT(false);
        m_error = QDbfTable::FileOpenError;
        return false;
    }

    QFileInfo tableFileInfo(m_tableFileName);
    const QDir &tableDir = tableFileInfo.dir();
    const QString &baseName = tableFileInfo.baseName();
    const QString &filter = QString(QLatin1String("%1.%2")).arg(baseName, memoFileExtension);
    const QStringList &entries = tableDir.entryList(QStringList(filter), QDir::Files);
    if (entries.isEmpty()) {
        return false;
    }
    const QString &memoFileName = QString(QLatin1String("%1/%2")).arg(tableDir.canonicalPath(), entries.first());
    m_memoFile.setFileName(memoFileName);

    if (!m_memoFile.exists(memoFileName) || !m_memoFile.open(QIODevice::ReadOnly)) {
        m_error = QDbfTable::FileOpenError;
        return false;
    }

    QDataStream stream(&m_memoFile);
    stream.setByteOrder(memoByteOrder());

    stream >> m_memoNextFreeBlockIndex;

    if (m_memoType == QDbfTablePrivate::FoxProMemo) {
        stream.device()->seek(MEMO_BLOCK_LENGTH_OFFSET);
        stream >> m_memoBlockLength;
        if (m_memoBlockLength < 1) {
            m_memoBlockLength = 1;
        }
    } else {
        m_memoBlockLength = MEMO_DBT_BLOCK_LENGTH;
    }

    m_error = QDbfTable::NoError;
    return true;
}

QVariant QDbfTablePrivate::memoFieldValue(int index) const
{
    Q_ASSERT(m_memoFile.isOpen() && m_memoFile.isReadable());

    qint64 position = static_cast<qint64>(m_memoBlockLength) * index;

    if (m_memoType == QDbfTablePrivate::DBaseMemo) {
        QByteArray data;
        forever {
            if (!m_memoFile.seek(position)) {
                m_error = QDbfTable::FileReadError;
                return QVariant::Invalid;
            }
            const QByteArray &block = m_memoFile.read(m_memoBlockLength);
            if (block.isEmpty()) {
                m_error = QDbfTable::FileReadError;
                return QVariant::Invalid;
            }
            const int endOfBlockPosition = block.indexOf(END_OF_DBASE_MEMO_BLOCK);
            if (endOfBlockPosition == -1) {
                data.append(block);
                position += m_memoBlockLength;
            } else {
                data.append(block.left(endOfBlockPosition));
                break;
            }
        }
        return m_textCodec->toUnicode(data);
    }

    QDataStream stream(&m_memoFile);
    stream.setByteOrder(memoByteOrder());
    if (!stream.device()->seek(position)) {
        m_error = QDbfTable::FileReadError;
        return QVariant::Invalid;
    }

    qint32 signature;
    stream >> signature;

    qint32 dataLength;
    stream >> dataLength;

    const QByteArray &data = m_memoFile.read(dataLength);
    if (dataLength > 0 && data.isEmpty()) {
        return QVariant::Invalid;
    }

    if (signature == MEMO_SIGNATURE_TEXT) {
        return m_textCodec->toUnicode(data);
    }

    m_error = QDbfTable::NoError;
    return data;
}

QDataStream::ByteOrder QDbfTablePrivate::memoByteOrder() const
{
    return (m_memoType == QDbfTablePrivate::DBaseIVMemo) ? QDataStream::LittleEndian : QDataStream::BigEndian;
}
bool QDbfTablePrivate::overrideCodepage(QDbfTable::Codepage codepage)
{
    m_codepage = codepage;
    setTextCodec();

    m_error = QDbfTable::NoError;
    return true;
}

bool QDbfTablePrivate::isValueValid(int i, const QVariant&) const
{
    switch (m_currentRecord.field(i).type()) {
    case QDbfField::Character:
        return QMetaType::canConvert(QMetaType(QDbfField::Character),QMetaType(QVariant::String));
    case QDbfField::Date:
        return QMetaType::canConvert(QMetaType(QDbfField::Date),QMetaType(QVariant::Date));
    case QDbfField::FloatingPoint:
        return QMetaType::canConvert(QMetaType(QDbfField::FloatingPoint),QMetaType(QVariant::Double));
    case QDbfField::Number:
        return QMetaType::canConvert(QMetaType(QDbfField::Number),QMetaType(QVariant::Double));
    case QDbfField::Logical:
        return QMetaType::canConvert(QMetaType(QDbfField::Logical),QMetaType(QVariant::Bool));
    case QDbfField::Memo:
        return m_memoType != QDbfTablePrivate::NoMemo && QMetaType::canConvert(QMetaType(QDbfField::Memo),QMetaType(QVariant::String));
    case QDbfField::Integer:
        return QMetaType::canConvert(QMetaType(QDbfField::Integer),QMetaType(QVariant::Int));
    case QDbfField::DateTime:
        return QMetaType::canConvert(QMetaType(QDbfField::DateTime),QMetaType(QVariant::DateTime));
    default:
        return false;
    }
}

void QDbfTablePrivate::setTextCodec()
{
    switch (m_codepage) {
    case QDbfTable::Windows1250:
        m_textCodec = QTextCodec::codecForName("Windows-1250");
        break;
    case QDbfTable::Windows1251:
        m_textCodec = QTextCodec::codecForName("Windows-1251");
        break;
    case QDbfTable::Windows1252:
        m_textCodec = QTextCodec::codecForName("Windows-1252");
        break;
    case QDbfTable::IBM850:
        m_textCodec = QTextCodec::codecForName("IBM 850");
        break;
    case QDbfTable::IBM866:
        m_textCodec = QTextCodec::codecForName("IBM 866");
        break;
    case QDbfTable::GB18030:
        m_textCodec = QTextCodec::codecForName("GB18030");
        break;
    case QDbfTable::Kamenicky:
        m_textCodec = QTextCodec::codecForName("Kamenicky");
        break;
    default:
        m_textCodec = QTextCodec::codecForLocale();
        break;
    }
}

QDate QDbfTablePrivate::dateFromByteArray(const QByteArray& byteArray) const
{
    Q_ASSERT(byteArray.length() == DATE_LENGTH);

    bool ok = false;

    const int y = byteArray.mid(YEAR_OFFSET, YEAR_LENGTH).toInt(&ok);
    if (!ok) {
        return QDate();
    }

    const int m = byteArray.mid(MONTH_OFFSET, MONTH_LENGTH).toInt(&ok);
    if (!ok) {
        return QDate();
    }

    const int d = byteArray.mid(DAY_OFFSET, DAY_LENGTH).toInt(&ok);
    if (!ok) {
        return QDate();
    }

    return QDate(y, m, d);
}

QTime QDbfTablePrivate::timeFromByteArray(const QByteArray& byteArray) const
{
    Q_ASSERT(byteArray.length() == TIME_LENGTH);

    bool ok = false;

    const int h = byteArray.mid(HOUR_OFFSET, HOUR_LENGTH).toInt(&ok);
    if (!ok) {
        return QTime();
    }

    const int m = byteArray.mid(MINUTE_OFFSET, MINUTE_LENGTH).toInt(&ok);
    if (!ok) {
        return QTime();
    }

    const int s = byteArray.mid(SECOND_OFFSET, SECOND_LENGTH).toInt(&ok);
    if (!ok) {
        return QTime();
    }

    return QTime(h, m, s);
}

} // namespace Internal

QDbfTable::QDbfTable() :
    d(new Internal::QDbfTablePrivate())
{
}

QDbfTable::QDbfTable(const QString &dbfFileName) :
    d(new Internal::QDbfTablePrivate(dbfFileName))
{
}

QDbfTable::~QDbfTable()
{
    delete d;
}

QString QDbfTable::fileName() const
{
    return d->m_tableFile.fileName();
}

QDbfTable::DbfTableError QDbfTable::error() const
{
    return d->m_error;
}

bool QDbfTable::open(const QString &fileName)
{
    d->m_tableFileName = fileName;
    return open();
}

void QDbfTable::close()
{
    d->clear();
    d->m_tableFile.close();
    d->m_memoFile.close();
}

bool QDbfTable::open()
{
    close();


    QFileInfo fileInfo(d->m_tableFileName);
    d->m_tableFile.setFileName(fileInfo.canonicalFilePath());

    if (!d->m_tableFile.exists() || !d->m_tableFile.open(QIODevice::ReadOnly)) {
        d->m_error = QDbfTable::FileOpenError;
        return false;
    }

    QDataStream stream(&d->m_tableFile);
    stream.setByteOrder(QDataStream::LittleEndian);

    Internal::QDbfTablePrivate::QDbfMemoType memoType = Internal::QDbfTablePrivate::NoMemo;

    // Table version
    quint8 version;
    stream >> version;
    switch(version) {
    case 0x02:
    case 0x03:
    case 0x04:
        break;
    case 0x30:
    case 0x31:
        memoType = Internal::QDbfTablePrivate::FoxProMemo;
        d->m_dbc = true;
        break;
    case 0x83:
        memoType = Internal::QDbfTablePrivate::DBaseMemo;
        break;
    case 0x8B:
    case 0x8C:
        memoType = Internal::QDbfTablePrivate::DBaseIVMemo;
        break;
    case 0xF5:
        memoType = Internal::QDbfTablePrivate::FoxProMemo;
        break;
    default:
        d->m_error = QDbfTable::UnsupportedFile;
        return false;
    }

    // Last update
    quint8 y;
    stream >> y;
    quint16 year = (y < 80 ? 2000 : 1900) + y;
    quint8 month;
    stream >> month;
    quint8 day;
    stream >> day;
    d->m_lastUpdate = QDate::fromString(QString(QLatin1String("%1%2%3")).arg(year).arg(month).arg(day), QLatin1String("yyyyMd"));

    // Number of records
    if (!d->m_tableFile.seek(TABLE_RECORDS_COUNT_OFFSET)) {
        d->m_error = QDbfTable::FileReadError;
        return false;
    }
    quint32 recordsCount = 0;
    stream >> recordsCount;
    if (std::numeric_limits<qint32>::max() < recordsCount) {
        d->m_error = QDbfTable::FileReadError;
        return false;
    }
    d->m_recordsCount = static_cast<qint32>(recordsCount);

    // Length of header structure
    if (!d->m_tableFile.seek(TABLE_FIRST_RECORD_POSITION_OFFSET)) {
        d->m_error = QDbfTable::FileReadError;
        return false;
    }
    stream >> d->m_headerLength;

    // Length of each record
    if (!d->m_tableFile.seek(RECORD_LENGTH_OFFSET)) {
        d->m_error = QDbfTable::FileReadError;
        return false;
    }
    stream >> d->m_recordLength;

    // Codepage
    if (!d->m_tableFile.seek(CODEPAGE_OFFSET)) {
        d->m_error = QDbfTable::FileReadError;
        return false;
    }

    quint8 codepage;
    stream >> codepage;
    switch(codepage) {
    case CODEPAGE_NOT_SET:
        break;
    case CODEPAGE_US_MSDOS:
        d->m_codepage = QDbfTable::IBM437;
        break;
    case CODEPAGE_INTERNATIONAL_MSDOD:
        d->m_codepage = QDbfTable::IBM850;
        break;
    case CODEPAGE_WINDOWS_ANSI_LATIN_1:
        d->m_codepage = QDbfTable::Windows1252;
        break;
    case CODEPAGE_RUSSIAN_OEM:
    case CODEPAGE_RUSSIAN_MSDOS:
        d->m_codepage = QDbfTable::IBM866;
        break;
    case CODEPAGE_EASTERN_EUROPEAN_WINDOWS:
        d->m_codepage = QDbfTable::Windows1250;
        break;
    case CODEPAGE_RUSSIAN_WINDOWS:
        d->m_codepage = QDbfTable::Windows1251;
        break;
    case CODEPAGE_GB18030:
        d->m_codepage = QDbfTable::GB18030;
        break;
    case CODEPAGE_KAMENICKY:
        d->m_codepage = QDbfTable::Kamenicky;
        break;
    default:
        d->m_codepage = QDbfTable::UnsupportedCodepage;
        break;
    }
    d->setTextCodec();


    quint16 fieldDescriptorsLength = d->m_headerLength - TABLE_DESCRIPTOR_LENGTH - TERMINATOR_LENGTH;
    if (d->m_dbc) {
        fieldDescriptorsLength -= DBC_LENGTH;
    }

    d->m_fieldsCount = fieldDescriptorsLength / FIELD_DESCRIPTOR_LENGTH;

    qint32 fieldOffset = 1;
    for (quint16 i = 0; i < d->m_fieldsCount; ++i) {
        const qint32 fieldDescriptorOffset = static_cast<qint32>(FIELD_DESCRIPTOR_LENGTH) * i + TABLE_DESCRIPTOR_LENGTH;

        // Field name
        if (!d->m_tableFile.seek(fieldDescriptorOffset)) {
            d->m_error = QDbfTable::FileReadError;
            return false;
        }
        QByteArray fieldName;
        bool       endField(false);
        for (quint8 j = 0; j <= FIELD_NAME_LENGTH; ++j) {
            qint8 fieldNameChar;
            stream >> fieldNameChar;

            if(!endField)
            {
                fieldName.append(fieldNameChar);
            }

            if(fieldNameChar == FIELD_NAME_SPACER)
            {
                endField = true;
            }
        }

        // Field type
        quint8 fieldTypeChar;
        stream >> fieldTypeChar;
        QDbfField::QDbfType fieldType;
        QVariant defaultValue;
        switch (fieldTypeChar) {
        case FIELD_TYPE_CHARACTER:
            fieldType = QDbfField::Character;
            defaultValue = QLatin1String("");
            break;
        case FIELD_TYPE_DATE:
            fieldType = QDbfField::Date;
            defaultValue = QDate();
            break;
        case FIELD_TYPE_FLOATING_POINT:
            fieldType = QDbfField::FloatingPoint;
            defaultValue = 0;
            break;
        case FIELD_TYPE_LOGICAL:
            fieldType = QDbfField::Logical;
            defaultValue = false;
            break;
        case FIELD_TYPE_MEMO:
            fieldType = QDbfField::Memo;
            defaultValue = QLatin1String("");
            d->m_memoType = memoType;
            break;
        case FIELD_TYPE_NUMBER:
            fieldType = QDbfField::Number;
            defaultValue = 0;
            break;
        case FIELD_TYPE_INTEGER:
            fieldType = QDbfField::Integer;
            defaultValue = 0;
            break;
        case FIELD_TYPE_DATE_TIME:
            fieldType = QDbfField::DateTime;
            defaultValue = QDateTime();
            break;
        default:
            fieldType = QDbfField::Undefined;
            defaultValue = QVariant::Invalid;
            break;
        }

        // Field length
        if (!d->m_tableFile.seek(fieldDescriptorOffset + FIELD_LENGTH_OFFSET)) {
            d->m_error = QDbfTable::FileReadError;
            return false;
        }
        quint8 fieldLength;
        stream >> fieldLength;

        // Decimal count
        quint8 fieldPrecision;
        stream >> fieldPrecision;

        // Build field
        QDbfField field(d->m_textCodec->toUnicode(fieldName));
        field.setType(fieldType);
        field.setLength(fieldLength);
        field.setPrecision(fieldPrecision);
        field.setOffset(fieldOffset);
        field.setDefaultValue(defaultValue);
        field.setValue(defaultValue);
        d->m_record.append(field);

        fieldOffset += fieldLength;
    }

    if (d->m_memoType != Internal::QDbfTablePrivate::NoMemo) {
        return d->openMemoFile();
    }

    return true;
}

QDbfTable::Codepage QDbfTable::codepage() const
{
    return d->m_codepage;
}

bool QDbfTable::overrideCodepage(QDbfTable::Codepage codepage)
{
    return d->overrideCodepage(codepage);
}

bool QDbfTable::isOpen() const
{
    return d->m_tableFile.isOpen();
}

int QDbfTable::size() const
{
    return d->m_recordsCount;
}

int QDbfTable::at() const
{
    return d->m_currentIndex;
}

bool QDbfTable::previous() const
{
    if (at() <= Internal::QDbfTablePrivate::FirstRow) {
        return false;
    }

    if (at() > (d->m_recordsCount - 1)) {
        return last();
    }

    return seek(at() - 1);
}

bool QDbfTable::next() const
{
    if (at() < Internal::QDbfTablePrivate::FirstRow) {
        return first();
    }

    if (at() >= (d->m_recordsCount - 1)) {
        return false;
    }

    return seek(at() + 1);
}

bool QDbfTable::first() const
{
    return seek(Internal::QDbfTablePrivate::FirstRow);
}

bool QDbfTable::last() const
{
    return seek(d->m_recordsCount - 1);
}

bool QDbfTable::seek(int index) const
{
    const qint32 previousIndex = d->m_currentIndex;

    if (index < Internal::QDbfTablePrivate::FirstRow) {
        d->m_currentIndex = Internal::QDbfTablePrivate::BeforeFirstRow;
    } else if (index > (d->m_recordsCount - 1)) {
        d->m_currentIndex = d->m_recordsCount - 1;
    } else {
        d->m_currentIndex = index;
    }

    if (previousIndex != d->m_currentIndex) {
        d->m_bufered = false;
    }

    return true;
}

QDate QDbfTable::lastUpdate() const
{
  return d->m_lastUpdate;
}

QDbfRecord QDbfTable::record() const
{
    if (d->m_bufered) {
        return d->m_currentRecord;
    }

    d->m_currentRecord = d->m_record;

    if (d->m_currentIndex < Internal::QDbfTablePrivate::FirstRow) {
        return d->m_currentRecord;
    }

    if (!d->m_tableFile.isOpen()) {
        d->m_error = QDbfTable::FileReadError;
        return d->m_currentRecord;
    }

    const qint64 position = static_cast<qint64>(d->m_recordLength) * d->m_currentIndex + d->m_headerLength;

    if (!d->m_tableFile.seek(position)) {
        d->m_error = QDbfTable::FileReadError;
        return d->m_currentRecord;
    }

    d->m_currentRecord.setRecordIndex(d->m_currentIndex);

    const QByteArray &recordData = d->m_tableFile.read(d->m_recordLength);
    if (recordData.length() != d->m_recordLength) {
        d->m_error = QDbfTable::FileReadError;
        return d->m_record;
    }

    d->m_currentRecord.setDeleted(recordData.at(0) == FIELD_DELETED ? true : false);

    for (int i = 0; i < d->m_currentRecord.count(); ++i) {
        const QByteArray &byteArray = recordData.mid(d->m_currentRecord.field(i).offset(), d->m_currentRecord.field(i).length());
        QVariant value;
        switch (d->m_currentRecord.field(i).type()) {
        case QDbfField::Character:
            value = d->m_textCodec->toUnicode(byteArray);
            break;
        case QDbfField::Date:
            value = QVariant(d->dateFromByteArray(byteArray));
            break;
        case QDbfField::FloatingPoint:
        case QDbfField::Number:
            if (d->m_record.field(i).precision() == 0) {
                value = byteArray.trimmed().toInt();
            } else {
                value = byteArray.trimmed().toDouble();
            }
            break;
        case QDbfField::Logical: {
            QString val = QString::fromLatin1(byteArray.toUpper());
            if (val == QString(QLatin1Char(LOGICAL_UNDEFINED))) {
                value = QVariant::Bool;
            } else if (val == QString(QLatin1Char(LOGICAL_TRUE)) || val == QString(QLatin1Char(LOGICAL_YES))) {
                value = true;
            } else if (val == QString(QLatin1Char(LOGICAL_FALSE)) || val == QString(QLatin1Char(LOGICAL_NO))) {
                value = false;
            } else {
                value = QVariant::Invalid;
            }
            break;
        }
        case QDbfField::Memo:
            if (d->m_memoType == Internal::QDbfTablePrivate::NoMemo) {
                value = QVariant::Invalid;
            } else if (byteArray.length() == 10) {
                if (!byteArray.trimmed().isEmpty()) {
                    bool ok = false;
                    int index = QString::fromLatin1(byteArray).toInt(&ok);
                    value = ok ? d->memoFieldValue(index) : QVariant::Invalid;
                } else {
                    value = QVariant::String;
                }
            } else if (byteArray.length() == 4) {
                QDataStream stream(byteArray);
                stream.setByteOrder(QDataStream::LittleEndian);
                qint32 index;
                stream >> index;
                value = d->memoFieldValue(index);
            } else {
                value = QVariant::Invalid;
            }
            break;
        case QDbfField::Integer: {
            QDataStream stream(byteArray);
            stream.setByteOrder(QDataStream::LittleEndian);
            qint32 val;
            stream >> val;
            value = val;
            break;
        }
        case QDbfField::DateTime: {
            if (byteArray.length() == DATETIME_LENGTH) {
                const QDate &date = d->dateFromByteArray(byteArray.mid(DATETIME_DATE_OFFSET, DATE_LENGTH));
                const QTime &time = d->timeFromByteArray(byteArray.mid(DATETIME_TIME_OFFSET, TIME_LENGTH));
                value = QVariant(QDateTime(date, time));
            } else if (byteArray.length() == TIMESTAMP_LENGTH) {
                QDataStream stream(byteArray);
                stream.setByteOrder(QDataStream::LittleEndian);
                qint32 day;
                stream >> day;
                qint32 msecs;
                stream >> msecs;
                const QDate &date = QDate::fromJulianDay(day);
#if QT_VERSION < 0x050200
                const QTime &time = QTime(0, 0, 0, 0).addMSecs(msecs);
#else
                const QTime &time = QTime::fromMSecsSinceStartOfDay(msecs);
#endif
                value = QVariant(QDateTime(date, time));
            } else {
                value = QVariant::Invalid;
            }
            break;
        }
        default:
            value = QVariant::Invalid;
            break;
        }

        d->m_currentRecord.setValue(i, value);
    }

    d->m_bufered = true;
    d->m_error = QDbfTable::NoError;
    return d->m_currentRecord;
}


QVariant QDbfTable::value(int fieldIndex) const
{
    return record().value(fieldIndex);
}

QVariant QDbfTable::value(const QString& name) const
{
    return record().value(name);
}



}}
