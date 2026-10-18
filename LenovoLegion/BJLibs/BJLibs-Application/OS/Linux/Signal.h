// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <functional>


namespace bj { namespace framework {


class Signal
{
public:

    /*
     * Set handler for Unix signal.
     *
     * Handler is NOT called from the signal context, signal number is forwarded
     * by self-pipe to the Qt event loop and handler is called from the thread
     * of QCoreApplication, so it may use any function (logging, Qt calls, ...).
     * Signals received before the event loop is started are delivered once it starts.
     * Registering the same signal again replaces the handler, empty handler restores
     * default signal action (SIG_DFL).
     *
     * QCoreApplication instance must exist, must be called from its thread.
     * Returns false on failure, errno is set.
     */
    static bool SetSignalHandler(int signal,std::function<void(int)> handler);

};

}}
