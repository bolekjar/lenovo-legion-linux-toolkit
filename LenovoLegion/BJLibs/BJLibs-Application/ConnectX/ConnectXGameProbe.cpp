// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ConnectXGameProbe.h"

#include <QRandomGenerator>

#include <chrono>

namespace {

/*unsigned int randInt(unsigned int low, unsigned int high)
{
    return QRandomGenerator::global()->generate() % ((high + 1) - low) + low;
}*/

}

namespace bj::connectX {

ConnectXGameProbe::ConnectXGameProbe(std::unique_ptr<ConnectXGameSimulation> simulation, int xMax, int yMax, int connectX):
    m_config {true, nullptr, 20, 0, -1},
    m_board(xMax,yMax,connectX),
    m_simulation(std::move(simulation))
{
    randomizeBoard();

}

void ConnectXGameProbe::init(const ConfigType &config)
{
    if(probeInProgress())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::PROBING_IN_PROGRESS,"Probing is in progress !");
    }

    m_config = config;

    randomizeBoard();
}

void ConnectXGameProbe::cleanup()
{
    if(probeInProgress())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::PROBING_IN_PROGRESS,"Probing is in progress !");
    }

    m_config = {true, nullptr, 20, 0, -1};
    m_result.release();
}

void ConnectXGameProbe::startProbe()
{
    if(probeInProgress())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::PROBING_IN_PROGRESS,"Probing is in progress !");
    }


    m_probingCtx= std::make_unique<ProbeCtx>(ProbeCtxType {
        .m_deap = 0,
        .m_limitInSec = m_config.m_probeLimitInSec,
        .m_start = std::chrono::system_clock::now(),
        .m_end   = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + std::chrono::seconds(m_config.m_probeLimitInSec)),
        .m_progressTime  = 0,
        .m_progressDepth = 0
    });


    m_simulation->cleanup();
    m_simulation->init(ConnectXGameSimulation::ConfigType{m_probingCtx->m_deap,BoardType::BoardItem::O,m_config.m_statsEnable,m_config.m_debugNotifier,m_config.m_threadsCount});
    m_simulation->startSimulation(m_board);
}

void ConnectXGameProbe::stopProbe()
{
    if(probeInProgress())
    {
        m_simulation->stopSimulation();
        m_probingCtx.release();
    }
}

bool ConnectXGameProbe::probeInProgress() const
{
    return m_probingCtx != nullptr;
}

bool ConnectXGameProbe::isResultAvailable() const
{
    return m_result != nullptr;
}

const ConnectXGameProbe::ResultType &ConnectXGameProbe::result() const
{
    if(probeInProgress())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::PROBING_IN_PROGRESS,"Probing is in progress !");
    }

    if(m_result == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_RESULT,"Result is not available !");
    }

    return *m_result;
}

const ConnectXGameProbe::BoardType &ConnectXGameProbe::board() const
{
    return m_board;
}

int ConnectXGameProbe::progressTime() const
{
    return m_probingCtx != nullptr ? m_probingCtx->m_progressTime : -1;
}

int ConnectXGameProbe::progressDepth() const
{
    return m_probingCtx != nullptr ? m_probingCtx->m_progressDepth : -1;
}

void ConnectXGameProbe::poll()
{
    m_simulation->poll();

    if(m_probingCtx != nullptr)
    {
        if( ( m_config.m_probeLimitInSec != 0 && (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) > m_probingCtx->m_end)) ||
            ( m_board.m_xMax * m_board.m_yMax) < m_probingCtx->m_deap                                                                           ||
            ( (m_config.m_depth == -1 ? m_config.m_depth : m_config.m_depth + 1) == m_probingCtx->m_deap)
          )
        {
            m_result = std::make_unique<ResultType>(ResultType {m_probingCtx->m_deap - 1,m_probingCtx->m_start});
            m_probingCtx.release();

            m_simulation->stopSimulation();
            randomizeBoard();

            return;
        }

        if(!m_simulation->simulationInProgress())
        {
            ProbeCtx newCtx = *m_probingCtx;

            ++newCtx.m_deap;


            m_probingCtx = std::make_unique<ProbeCtx>(newCtx);

            randomizeBoard();

            m_simulation->cleanup();
            m_simulation->init(ConnectXGameSimulation::ConfigType{m_probingCtx->m_deap,BoardType::BoardItem::O,m_config.m_statsEnable,m_config.m_debugNotifier,m_config.m_threadsCount});
            m_simulation->startSimulation(m_board);
        }

        /*
         * Refresh progress
         */
        m_probingCtx->m_progressDepth = (m_probingCtx->m_deap - 1) * 100 / boardMaxDepth();

        if(m_config.m_probeLimitInSec != 0)
        {
            m_probingCtx->m_progressTime = (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()) -
                                            std::chrono::duration_cast<std::chrono::seconds>(m_probingCtx->m_start.time_since_epoch())).count() * 100 / m_config.m_probeLimitInSec;
        }
    }
}

ConnectXGameProbe::StatisticsType ConnectXGameProbe::stats()
{
    return StatisticsType { m_simulation->stats(), (m_probingCtx == nullptr ?  0 : m_probingCtx->m_deap)};
}

void ConnectXGameProbe::randomizeBoard()
{
    m_board.clear();


    if(m_board.m_xMax <= m_board.m_connectX)
    {
        m_board.set(std::make_pair(m_board.m_xMax/2,m_board.m_yMax/2),{bj::connectX::ConnectXBoardTrackLastPosition::BoardItem::X,false,false});
    }
    else
    {
        std::vector<int> randomizeInterval(m_board.m_xMax/m_board.m_connectX);

        for (size_t i = 0; i < randomizeInterval.size(); ++i) {

            randomizeInterval[i] = m_board.m_connectX;

            if( i + 1  == randomizeInterval.size())
            {
                randomizeInterval[i] += m_board.m_xMax % m_board.m_connectX;
            };
        }

        for (int posY = 0; posY < m_board.m_yMax ; posY += m_board.m_connectX )
        {
            for (int posX = 0; posX < m_board.m_xMax; posX += m_board.m_connectX) {
                m_board.set(std::make_pair(posX,posY),{bj::connectX::ConnectXBoardTrackLastPosition::BoardItem::X,false,false});
            }
        }
    }
}

int ConnectXGameProbe::boardMaxDepth()
{
    if(m_config.m_depth == -1)
    {
        int         emptyPossitions    = 0;

        for (auto c = m_board.cbegin() ; c != m_board.cend(); ++c)
        {
            if(m_board.get(c.position()).m_item == bj::connectX::BoardItem::EMPTY)
            {
                ++emptyPossitions;
            }
        }

        return emptyPossitions;
    }

    return m_config.m_depth;
}


}
