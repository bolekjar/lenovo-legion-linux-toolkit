// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProvider.h"

#include <QFile>
#include <QStringList>

namespace LenovoLegionDaemon::Kit {

namespace {

template<class T>
QByteArray join(const std::vector<T>& values)
{
    QByteArray data;

    for(size_t index = 0; index < values.size(); ++index)
    {
        if(index > 0)
        {
            data.append(',');
        }

        data.append(QByteArray::number(values[index]));
    }

    return data;
}

}

SysFsDataProvider::SysFsDataProvider(SysFsDriverManager* sysFsDriverManager, quint8 dataType) :
    DataProvider(dataType),
    m_sysFsDriverManager(sysFsDriverManager)
{}

QString SysFsDataProvider::getData(const std::filesystem::path &path)
{
    QFile file(QString::fromStdString(path.string()));

    if(!file.open(QIODeviceBase::ReadOnly))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_READING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=ReadOnly !"));
    }

    return QString::fromUtf8(file.readAll()).trimmed();
}

std::optional<QString> SysFsDataProvider::getOptionalData(const std::filesystem::path &path)
{
    QFile file(QString::fromStdString(path.string()));

    if(!file.open(QIODeviceBase::ReadOnly))
    {
        return {};
    }

    return QString::fromUtf8(file.readAll()).trimmed();
}

void SysFsDataProvider::writeData(const std::filesystem::path &path, const QByteArray &data)
{
    QFile file(QString::fromStdString(path.string()));

    if(!file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Unbuffered))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::OPEN_FOR_WRITING_ERROR,std::string("I can not open file (").append(path.string()).append(") with permision=WriteOnly !"));
    }

    if(file.write(data) != data.size())
    {
        LOG_E(QString("Unable to write ").append(QString::fromUtf8(data)).append(" to ").append(QString::fromStdString(path.string())).append(" : ").append(file.errorString()));
    }
}

void SysFsDataProvider::setData(const std::filesystem::path &path, quint8 value)  { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, quint16 value) { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, quint32 value) { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, quint64 value) { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, qint8 value)   { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, qint16 value)  { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, qint32 value)  { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, qint64 value)  { writeData(path,QByteArray::number(value)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, bool value)    { writeData(path,value ? "1" : "0"); }

void SysFsDataProvider::setData(const std::filesystem::path &path, const std::vector<quint8> &values)  { writeData(path,join(values)); }
void SysFsDataProvider::setData(const std::filesystem::path &path, const std::vector<quint32> &values) { writeData(path,join(values)); }

void SysFsDataProvider::setData(const std::filesystem::path &path, const std::string_view &value)
{
    writeData(path,QByteArray(value.data(),static_cast<qsizetype>(value.size())));
}

}
