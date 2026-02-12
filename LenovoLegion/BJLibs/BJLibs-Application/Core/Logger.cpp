// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Logger.h"


#include <QCoreApplication>
#include <QDateTime>
#include <QTextStream>
#include <QFileDevice>


namespace bj { namespace framework {


QMap<Logger::SEVERITY,QString>  Logger::dictonary = {
        {Logger::SEVERITY::INFO,   " <Info> "   },
        {Logger::SEVERITY::DEBUG,  " <Debug> "  },
        {Logger::SEVERITY::WARNING," <Warning> "},
        {Logger::SEVERITY::ERROR,  " <Error> "  },
        {Logger::SEVERITY::TRACE,  " <Trace> "  }
};

void Logger::setSeverity(const SEVERITY_BITSET& severity)
{
    m_severity.store(severity,std::memory_order_relaxed);
}

void Logger::init(const std::string &pathToLogFile)
{
    std::unique_lock<std::mutex> m_guard(m_mutex);

    if(m_logFile.isOpen())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("The log file engine was initialized: ").append(pathToLogFile));
    }

    m_logFile.setFileName(pathToLogFile.c_str());

    if(!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("I can not open the file: ").append(pathToLogFile));
    }
}

void Logger::write(const QString &data,SEVERITY severity)
{
    if(m_severity.load(std::memory_order_relaxed)[severity] == false) {
        return;
    }

    if(!m_logFile.isOpen())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("The log file engine was not initialized : "));
    }

    QString text = data;

    text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz ") + dictonary[severity] + text ;

    {
        std::unique_lock<std::mutex> m_guard(m_mutex);

        QTextStream out(&m_logFile);
        out.setEncoding(QStringConverter::Utf8);

        out << text;
        Qt::endl(out);

        if(m_logFile.error() != QFileDevice::NoError)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("I can not write to log file: ").append(m_logFile.fileName().toStdString()));
        }
    }
}

void Logger::write(const std::stringstream &data, Logger::SEVERITY severity)
{
    if(m_severity.load(std::memory_order_relaxed)[severity]  == false) {
        return;
    }

    if(!m_logFile.isOpen())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("The log file engine was not initialized : "));
    }

    QString text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz ") + dictonary[severity] + data.str().c_str();

    {
        std::unique_lock<std::mutex> m_guard(m_mutex);

        QTextStream out(&m_logFile);
        out.setEncoding(QStringConverter::Utf8);

        out << text;
        Qt::endl(out);

        if(m_logFile.error() != QFileDevice::NoError)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("I can not write to log file: ").append(m_logFile.fileName().toStdString()));
        }
    }
}


Logger::Logger()
    : m_severity(0xF)
{}

Logger::~Logger()
{
    m_logFile.close();
}

}}

