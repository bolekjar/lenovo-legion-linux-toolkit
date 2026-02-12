// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Signal.h"

#include <map>
#include <functional>


#include <signal.h>


namespace bj { namespace framework {



static std::map<int, std::function<void(int)>>     signalToHandler;
static std::map<int,struct sigaction>              signaToSigaction;


static void SignalHandlerHelper(int signal)
{
    signalToHandler[signal](signal);
}

void Signal::SetSignalHandler(int signal,std::function<void(int)> handler)
{
    signalToHandler[signal]              = handler;


    signaToSigaction[signal].sa_handler     = SignalHandlerHelper;
    signaToSigaction[signal].sa_flags       = SA_RESTART;

    sigemptyset(&signaToSigaction[signal].sa_mask);
    sigaction(signal, &signaToSigaction[signal], NULL);
}

}};
