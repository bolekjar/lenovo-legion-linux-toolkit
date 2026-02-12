// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <ConnectX/ConnectXBoardTrackerBase.h>
#include <ConnectX/ConnectXBoard.h>

#include <Core/ExceptionBuilder.h>
#include <BigInt/BigUnsigned.h>

#include <cstdint>

namespace bj { namespace connectX {

struct ConnectXGameInfo {
    const QString m_name;
};

class ConnectXGameSimulation
{
    struct DebugMessage {
        std::string info;
    };

public:

    using DebugMessageType          = DebugMessage;
    using BoardItemType             = BoardItem;
    using NotifierType              = std::function<void (const DebugMessageType& )>;


private:

    struct Statistics {
        uint64_t m_travelledNodes;
    };

    struct Config {
        int             m_deapLimit;
        BoardItemType   m_ownXorO;
        bool            m_statsEnable;
        NotifierType    m_debugNotifier;
        unsigned int    m_threadsCount;
    };

public:

    using ConnectXBoardType         = ConnectXBoardTrackLastPosition;
    using ConnectXScoreType         = ConnectXScore;
    using ResultType                = std::pair<ConnectXScoreType,ConnectXBoardType::Position>;
    using StatisticsType            = Statistics;
    using ConfigType                = Config;


    DEFINE_EXCEPTION(ConnectXGameSimulation)

    enum ERROR_CODES : int {
        INVALID_PARAM           = 1,
        SIMULATION_IN_PROGRESS  = 2
    };

public:
    ConnectXGameSimulation();

    virtual ~ConnectXGameSimulation();


    /*
     * Name of module
     */
    virtual QString name() const  = 0;

    /*
     * Init, Cleanup
     */
    virtual void init(const ConfigType&) = 0;
    virtual void cleanup()               = 0;


    /*
     * Simulation
     */
    virtual void                            startSimulation(ConnectXBoardType&)            = 0;
    virtual void                            stopSimulation()                               = 0;

    /*
     * Getters
     */
    virtual bool                            simulationInProgress()                              const           = 0;
    virtual const ResultType&               result()                                            const           = 0;

    /*
     * Pool
     */
    virtual void poll() = 0;

    /*
     * Statistics
     */
    virtual StatisticsType stats() = 0;
};


}}
