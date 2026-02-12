// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ConnectXBoardTrackerBase.h"


namespace bj { namespace connectX {


class ConnectXBoardTracker : public ConnectXBoardTrackerBase
{

public:

    ConnectXBoardTracker();

    virtual void store(const TrackerData& data) override;

    const TrackerData& last() const;
    bool          isDefined() const;

private:

    TrackerData m_last;
};

}}
