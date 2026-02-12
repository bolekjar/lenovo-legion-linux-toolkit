// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <functional>


namespace bj { namespace framework {


class Signal
{
public:

    static void SetSignalHandler(int signal,std::function<void(int)> handler);

};

}}
