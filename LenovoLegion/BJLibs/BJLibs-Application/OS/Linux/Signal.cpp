// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Signal.h"

#include <QCoreApplication>
#include <QPointer>
#include <QSocketNotifier>

#include <map>
#include <functional>
#include <climits>


#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>


namespace bj { namespace framework {


/*
 * Self-pipe, [0] is written by SignalHandlerHelper (signal context), [1] is read by signalNotifier (event loop)
 */
static int                                          signalFd[2] = {-1, -1};
static QPointer<QSocketNotifier>                    signalNotifier;
static std::map<int, std::function<void(int)>>      signalToHandler;


/*
 * Async-signal-safe, only forwards signal number to the event loop
 */
static void SignalHandlerHelper(int signal)
{
    const int           savedErrno = errno;
    const unsigned char sig        = static_cast<unsigned char>(signal);

    /*
     * Non-blocking write, if the buffer is full the signal is already pending
     */
    [[maybe_unused]] const ssize_t ret = ::write(signalFd[0], &sig, sizeof(sig));

    errno = savedErrno;
}

/*
 * Called from the event loop
 */
static void SignalDispatch()
{
    unsigned char sig = 0;

    while(::read(signalFd[1], &sig, sizeof(sig)) == sizeof(sig))
    {
        const auto it = signalToHandler.find(sig);

        if(it != signalToHandler.end() && it->second)
        {
            /*
             * Call a copy, handler may re-register (replace) or unregister itself
             */
            const std::function<void(int)> handler = it->second;
            handler(sig);
        }
    }
}

static bool SignalInit()
{
    if(QCoreApplication::instance() == nullptr)
    {
        errno = EINVAL;
        return false;
    }

    if(signalFd[0] == -1 && socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, signalFd) != 0)
    {
        return false;
    }

    if(signalNotifier.isNull())
    {
        signalNotifier = new QSocketNotifier(signalFd[1], QSocketNotifier::Read, QCoreApplication::instance());
        QObject::connect(signalNotifier, &QSocketNotifier::activated, signalNotifier, &SignalDispatch);
    }

    return true;
}

bool Signal::SetSignalHandler(int signal,std::function<void(int)> handler)
{
    if(signal <= 0 || signal > UCHAR_MAX)
    {
        errno = EINVAL;
        return false;
    }

    /*
     * Empty handler, restore default signal action
     */
    if(!handler)
    {
        signalToHandler.erase(signal);

        struct sigaction sa {};
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);

        return sigaction(signal, &sa, nullptr) == 0;
    }

    if(!SignalInit())
    {
        return false;
    }

    signalToHandler[signal] = std::move(handler);

    struct sigaction sa {};
    sa.sa_handler = SignalHandlerHelper;
    sa.sa_flags   = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    return sigaction(signal, &sa, nullptr) == 0;
}

}};
