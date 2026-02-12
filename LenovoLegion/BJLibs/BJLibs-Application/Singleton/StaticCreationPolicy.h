// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

namespace bj { namespace framework {

template <typename T>
class StaticCreationPolicy {

public:

    static T*   createInstance();
    static void destroyInstance(T*);

};

template <typename T>
T* StaticCreationPolicy<T>::createInstance() {
    static T _instance;

    return &_instance;
}

template <typename T>void StaticCreationPolicy<T>::destroyInstance(T*) {}

}}
