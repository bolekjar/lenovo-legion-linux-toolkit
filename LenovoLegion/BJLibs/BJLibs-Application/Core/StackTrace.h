// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QStack>
#include <ostream>


namespace bj { namespace framework {

struct StackTrace {

    struct Frame
    {
        uint64_t    m_pos;
        QString     m_name;
        uint64_t    m_addr;
    };

    using Frame_T =  Frame;
    using Stack_T =  QStack<Frame_T>;

    static Stack_T getStackTrace(size_t maxStackTraceSize = 1024);
    static QString getFormatetStackTrace();
};


std::ostream& operator<<(std::ostream& out,const StackTrace::Stack_T& stack);


}}
