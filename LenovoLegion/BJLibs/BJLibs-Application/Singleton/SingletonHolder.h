// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

namespace bj { namespace framework {

template <
        typename            T,
        template <class>    class CreationPolicy   ,
        template <class>    class ThreadModelPolicy
        >
class SingletonHolder
{

    using CreationPolicyType    = CreationPolicy<T>;
    using ThreadModelPolicyType = ThreadModelPolicy<T>;

public:

    static T& getInstance();

private:

    SingletonHolder()                                   = delete;
    SingletonHolder(const SingletonHolder&)             = delete;
    SingletonHolder(const SingletonHolder&&)            = delete;
    SingletonHolder& operator=(const SingletonHolder&)  = delete;
    SingletonHolder& operator=(const SingletonHolder&&) = delete;


    static void DeleteSingleton();

private:


   static typename ThreadModelPolicy<T>::VolatileType* m_instance;

};

template <
        typename            T,
        template <class>    class CreationPolicy   ,
        template <class>    class ThreadModelPolicy
        >
typename ThreadModelPolicy<T>::VolatileType* SingletonHolder<T,CreationPolicy,ThreadModelPolicy>::m_instance = nullptr;



template <
        typename            T,
        template <class>    class CreationPolicy   ,
        template <class>    class ThreadModelPolicy
        >
void SingletonHolder<T,CreationPolicy,ThreadModelPolicy>::DeleteSingleton() {

}


template <
        typename            T,
        template <class>    class CreationPolicy   ,
        template <class>    class ThreadModelPolicy
        >
T& SingletonHolder<T,CreationPolicy,ThreadModelPolicy>::getInstance(){

    if(m_instance == nullptr)
    {
        typename ThreadModelPolicyType::LockGuardType m_guard(ThreadModelPolicyType::m_mutex);

        if(m_instance == nullptr)
        {
            m_instance = CreationPolicyType::createInstance();
        }
    }

    return *const_cast<T*>(m_instance);
}


}}
