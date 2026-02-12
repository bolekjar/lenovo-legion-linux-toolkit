// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "DataProvider.h"
#include "SysFsDriverManager.h"

#include <QObject>
#include <QByteArray>

#include <string_view>

namespace LenovoLegionDaemon {

    class SysFsDataProvider : public DataProvider
    {
    public:

        enum ERROR_CODES : int {
            OPEN_FOR_READING_ERROR              = 1,
            OPEN_FOR_WRITING_ERROR              = 2,
        };


        DEFINE_EXCEPTION(SysFsData);

    public:
        SysFsDataProvider(SysFsDriverManager* sysFsDriverManager,QObject* parent,quint8 dataType);
        virtual ~SysFsDataProvider() = default;

    public:

        static QString getData(const std::filesystem::path &path);

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

        SysFsDriverManager * m_sysFsDriverManager;
    };

}
