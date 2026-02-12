// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ExceptionBuilder.h"

#include <QFile>
#include <QString>

#include <mutex>
#include <atomic>
#include <sstream>
#include <bitset>

namespace bj { namespace framework {

class Logger
{

public:

    enum SEVERITY : quint8 {
        INFO    = 0,
        WARNING = 1,
        DEBUG   = 2,
        ERROR   = 3,
        TRACE  =  4
    };

    using SEVERITY_BITSET =  std::bitset<5>;

private:


    static QMap<Logger::SEVERITY,QString>  dictonary;

public:

    enum ERROR_CODES : int {
        FILE_OPEN_ERROR = 1
    };

    DEFINE_EXCEPTION(Logger)

public:

    Logger();
    ~Logger();

    void setSeverity(const SEVERITY_BITSET& severity);

    void init(const std::string& pathToLogFile);
    void write(const QString& data,SEVERITY severity = INFO);
    void write(const std::stringstream& data,SEVERITY severity = INFO);

private:
     std::atomic<SEVERITY_BITSET>  m_severity;

     std::mutex         m_mutex;
     QFile              m_logFile;
};


#define LOG_INFO(x,__LoggerSingletonHolder__)     __LoggerSingletonHolder__::getInstance().write(x,bj::framework::Logger::SEVERITY::INFO)
#define LOG_WARNING(x,__LoggerSingletonHolder__)  __LoggerSingletonHolder__::getInstance().write(x,bj::framework::Logger::SEVERITY::WARNING)
#define LOG_DEBUG(x,__LoggerSingletonHolder__)    __LoggerSingletonHolder__::getInstance().write(x,bj::framework::Logger::SEVERITY::DEBUG)
#define LOG_ERROR(x,__LoggerSingletonHolder__)    __LoggerSingletonHolder__::getInstance().write(x,bj::framework::Logger::SEVERITY::ERROR)
#define LOG_TRACE(x,__LoggerSingletonHolder__)    __LoggerSingletonHolder__::getInstance().write(x,bj::framework::Logger::SEVERITY::TRACE)



}}
