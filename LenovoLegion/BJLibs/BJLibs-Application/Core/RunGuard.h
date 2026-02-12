// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>

namespace bj { namespace framework {


class RunGuard
{

public:
    RunGuard( const QString& key );
    ~RunGuard();

    bool isAnotherRunning();
    bool tryToRun();
    void release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;

    RunGuard(const RunGuard&) = delete;
    RunGuard(RunGuard&&)      = delete;

    RunGuard& operator=(const RunGuard&) = delete;
    RunGuard& operator=(RunGuard&&)      = delete;

};


}}
