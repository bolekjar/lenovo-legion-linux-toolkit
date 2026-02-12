// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ExceptionBuilder.h>
#include <ConnectX/ConnectXGameSimulation.h>

#include <chrono>

namespace bj::connectX {

class ConnectXGameProbe
{

public:

    using BoardType     = ConnectXGameSimulation::ConnectXBoardType;
    using NotifierType  = ConnectXGameSimulation::NotifierType;

private:

    struct Config {
        bool            m_statsEnable;
        NotifierType    m_debugNotifier;
        uint32_t        m_probeLimitInSec;
        unsigned int    m_threadsCount;
        int             m_depth;
    };

    struct Result {
        int                                                 m_deapLimit;
        std::chrono::time_point<std::chrono::system_clock>  m_start;
    };

    struct Statistics : ConnectXGameSimulation::StatisticsType {
        int    m_curDeapLimit;
    };

    struct ProbeCtx
    {
        int                                                 m_deap;
        unsigned int                                        m_limitInSec;
        std::chrono::time_point<std::chrono::system_clock>  m_start;
        std::time_t                                         m_end;
        int                                                 m_progressTime;
        int                                                 m_progressDepth;
    };

public:

    using ConfigType = Config;
    using ResultType = Result;
    using StatisticsType = Statistics;
    using ProbeCtxType   = ProbeCtx;

public:

    DEFINE_EXCEPTION(ConnectXGameProbe)

    enum ERROR_CODES : int {
        INVALID_PARAM           = 1,
        PROBING_IN_PROGRESS     = 2,
        INVALID_RESULT          = 3
    };

public:

    ConnectXGameProbe(std::unique_ptr<ConnectXGameSimulation> simulation,int xMax, int yMax, int connectX);

    /*
     * Init, Cleanup
     */
    void init(const ConfigType& config) ;
    void cleanup()                      ;


    /*
     * Probing
     */
    void startProbe();
    void stopProbe();

    /*
     * Getters
     */
    bool              probeInProgress()     const;
    bool              isResultAvailable()   const;
    const ResultType& result()              const;
    const BoardType&  board()               const;
    int               progressTime()        const;
    int               progressDepth()       const;


    /*
     * Pool
     */
    void poll();

    /*
     * Statistics
     */
    StatisticsType stats();

private:

    void randomizeBoard();
    int  boardMaxDepth();

private:

    ConfigType                                  m_config;
    BoardType                                   m_board;
    std::unique_ptr<ConnectXGameSimulation>     m_simulation;

    std::unique_ptr<ResultType>     m_result;
    std::unique_ptr<ProbeCtxType>   m_probingCtx;
};

}

