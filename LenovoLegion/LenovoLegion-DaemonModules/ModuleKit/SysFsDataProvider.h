// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Sysfs data provider of a daemon module. Same interface as the daemon SysFsDataProvider.
 *
 * setData: the file is opened unbuffered, so the error of the kernel store (-EINVAL, -EBUSY, ...)
 * is returned by write(). Open failure throws (as before), write failure is only logged
 * (the client re-reads the values after set, other values of the request are still written).
 */

#include "DataProvider.h"
#include "SysFsDriverManager.h"

#include <optional>
#include <string_view>
#include <vector>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProvider : public DataProvider
{
public:

    enum ERROR_CODES : int {
        OPEN_FOR_READING_ERROR              = 1,
        OPEN_FOR_WRITING_ERROR              = 2,
    };

public:

    SysFsDataProvider(SysFsDriverManager* sysFsDriverManager,quint8 dataType);
    ~SysFsDataProvider() override = default;

public:

    static QString getData(const std::filesystem::path &path);

    /*
     * Value of the attribute, empty if the attribute does not exist or can not be read (no exception)
     */
    static std::optional<QString> getOptionalData(const std::filesystem::path &path);

    static void setData(const std::filesystem::path &path, quint8 value);
    static void setData(const std::filesystem::path &path, quint16 value);
    static void setData(const std::filesystem::path &path, quint32 value);
    static void setData(const std::filesystem::path &path, quint64 value);
    static void setData(const std::filesystem::path &path, qint8 value);
    static void setData(const std::filesystem::path &path, qint16 value);
    static void setData(const std::filesystem::path &path, qint32 value);
    static void setData(const std::filesystem::path &path, qint64 value);
    static void setData(const std::filesystem::path &path, bool value);
    static void setData(const std::filesystem::path &path, const std::vector<quint8>& values);
    static void setData(const std::filesystem::path &path, const std::vector<quint32>& values);
    static void setData(const std::filesystem::path &path, const std::string_view &value);

protected:

    static void writeData(const std::filesystem::path &path,const QByteArray& data);

    SysFsDriverManager * m_sysFsDriverManager;
};

}
