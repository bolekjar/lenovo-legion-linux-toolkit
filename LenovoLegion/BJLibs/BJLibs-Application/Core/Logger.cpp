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
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDevice>

#ifdef Q_OS_UNIX
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


namespace bj { namespace framework {


QMap<Logger::SEVERITY,QString>  Logger::dictonary = {
        {Logger::SEVERITY::INFO,   " <Info> "   },
        {Logger::SEVERITY::DEBUG,  " <Debug> "  },
        {Logger::SEVERITY::WARNING," <Warning> "},
        {Logger::SEVERITY::ERROR,  " <Error> "  },
        {Logger::SEVERITY::TRACE,  " <Trace> "  }
};

/*
 * Log of the previous start renamed to <name>-<yyyy-MM-dd-hh:mm:ss>.<suffix> (time of its last record), every start
 * writes its own log. Only a regular file is renamed (rename() moves a symlink itself, not its target).
 */
static void rotateLogFile(const std::string &pathToLogFile)
{
    const QString path = QString::fromStdString(pathToLogFile);
    const QFileInfo info(path);

    if(info.isSymLink() || !info.isFile())
    {
        return;
    }

    const QString stamp = info.lastModified().toString("yyyy-MM-dd-hh:mm:ss");
    const QString base  = info.dir().filePath(info.completeBaseName() + "-" + stamp);
    const QString suffix = info.suffix().isEmpty() ? QString() : "." + info.suffix();

    /* QFile::rename does not overwrite an existing file: a log with the same time gets a number */
    for(int i = 0; i < 100; ++i)
    {
        const QString target = i == 0 ? base + suffix : base + "." + QString::number(i) + suffix;

        if(!QFileInfo::exists(target) && QFile::rename(path, target))
        {
            return;
        }
    }
}

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

    rotateLogFile(pathToLogFile);

#ifdef Q_OS_UNIX
    /*
     * Log directory may be writable by other users (e.g. group shared), do not follow symlinks
     * and accept only a regular file, otherwise the file could be redirected to any other file.
     */
    const int fd = ::open(pathToLogFile.c_str(), O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0660);

    if(fd < 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("I can not open the file: ").append(pathToLogFile));
    }

    struct stat st {};

    if(fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_nlink != 1)
    {
        ::close(fd);
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("The log file is not a regular file: ").append(pathToLogFile));
    }

    if(!m_logFile.open(fd, QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text, QFileDevice::AutoCloseHandle))
    {
        ::close(fd);
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("I can not open the file: ").append(pathToLogFile));
    }
#else
    m_logFile.setFileName(pathToLogFile.c_str());

    if(!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::FILE_OPEN_ERROR,std::string("I can not open the file: ").append(pathToLogFile));
    }
#endif
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

