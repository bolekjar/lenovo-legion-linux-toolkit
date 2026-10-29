// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Exceptions of daemon modules, derived from Module::Error (std::exception) which the framework catches.
 * Same interface as the BJLibs exceptions used by the daemon (THROW_EXCEPTION, errcodeInfo, descriptionInfo),
 * so the code of drivers and providers stays unchanged, without dependency on BJLibs.
 */

#include <ModuleTypes.h>

#include <string>

namespace LenovoLegionDaemon::Kit {

class Exception : public Module::Error
{
public:

    template<class T>
    struct Info
    {
        T m_value;

        const T& value() const { return m_value; }
    };

public:

    Exception(int code,const std::string& description) :
        Module::Error(description),
        m_code(code),
        m_description(description)
    {}

    Info<int>         errcodeInfo()     const { return { m_code }; }
    Info<std::string> descriptionInfo() const { return { m_description }; }

private:

    int         m_code;
    std::string m_description;
};

/*
 * Exceptions of drivers and of data providers are different types (as in the daemon)
 */
class DriverException : public Exception
{
public:
    using Exception::Exception;
};

class ProviderException : public Exception
{
public:
    using Exception::Exception;
};

}

#define THROW_EXCEPTION(exname,errcode,description) { throw exname(static_cast<int>(errcode),std::string(description)); }

/*
 * Own exception type of a class (same as BJLibs DEFINE_EXCEPTION): x##Exception and exception_T
 */
#define DEFINE_EXCEPTION(x)                                                         \
    class x ## Exception : public ::LenovoLegionDaemon::Kit::Exception              \
    {                                                                               \
    public:                                                                         \
        using ::LenovoLegionDaemon::Kit::Exception::Exception;                      \
    };                                                                              \
    using exception_T = x ## Exception;
