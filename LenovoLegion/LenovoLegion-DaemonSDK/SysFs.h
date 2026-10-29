// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * DRAFT of the daemon module API (see LenovoLegion-Daemon/ModulesDesign.md).
 *
 * Helpers for reading and writing sysfs attributes.
 */

#include "ModuleTypes.h"

#include <QByteArray>
#include <QFile>
#include <QString>

#include <filesystem>

namespace LenovoLegionDaemon::Module::SysFs {

inline bool exists(const std::filesystem::path& path)
{
    std::error_code ec;
    return std::filesystem::exists(path,ec);
}

/*
 * Read attribute, trimmed. Throws Error if the attribute can not be read.
 */
inline QString read(const std::filesystem::path& path)
{
    QFile file(QString::fromStdString(path.string()));

    if(!file.open(QIODeviceBase::ReadOnly))
    {
        throw Error("Unable to read " + path.string() + " : " + file.errorString().toStdString());
    }

    return QString::fromUtf8(file.readAll()).trimmed();
}

/*
 * Write attribute. Unbuffered, so the error of the kernel store (-EINVAL, -EBUSY, -EOPNOTSUPP, ...)
 * is returned by write(). Returns false on error, error description in errorString (optional).
 */
inline bool write(const std::filesystem::path& path,const QByteArray& value,QString* errorString = nullptr)
{
    QFile file(QString::fromStdString(path.string()));

    if(!file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Unbuffered) || file.write(value) != value.size())
    {
        if(errorString != nullptr)
        {
            *errorString = file.errorString();
        }

        return false;
    }

    return true;
}

inline bool write(const std::filesystem::path& path,qint64 value,QString* errorString = nullptr)
{
    return write(path,QByteArray::number(value),errorString);
}

}
