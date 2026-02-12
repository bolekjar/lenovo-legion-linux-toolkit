// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <mutex>


namespace bj { namespace framework {

template<typename T>
class DefaultThreadModel
{

public:

    using VolatileType  = volatile T;
    using LockGuardType = std::lock_guard<std::mutex>;



public:

    static void lock();
    static void unlock();

public:

    static std::mutex   m_mutex;
};


template<typename T>
std::mutex DefaultThreadModel<T>::m_mutex;



template<typename T>
void DefaultThreadModel<T>::lock() {
    m_mutex.lock();
}

template<typename T>
void DefaultThreadModel<T>::unlock() {
    m_mutex.unlock();
}


}}
