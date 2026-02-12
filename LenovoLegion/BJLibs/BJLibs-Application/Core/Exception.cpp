// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Exception.h"

namespace bj { namespace framework { namespace exception {

Exception &Exception::operator<<(const Exception::line_error_info_T &x)
{
    m_lineInfo.m_value = x.m_value;

    return *this;
}

Exception &Exception::operator<<(const Exception::file_error_info_T &x)
{
    m_fileInfo.m_value = x.m_value;

    return *this;
}

Exception &Exception::operator<<(const Exception::function_error_info_T &x)
{
    m_functionInfo.m_value = x.m_value;

    return *this;
}

Exception &Exception::operator<<(const Exception::errcode_error_info_T &x)
{
    m_errcodeInfo.m_value = x.m_value;

    return *this;
}

Exception &Exception::operator<<(const Exception::description_error_info_T &x)
{
    m_descriptionInfo.m_value = x.m_value;

    return *this;
}

const Exception::line_error_info_T Exception::lineInfo() const
{
    return m_lineInfo;
}

const Exception::file_error_info_T& Exception::fileInfo() const
{
    return m_fileInfo;
}

const Exception::function_error_info_T& Exception::functionInfo() const
{
    return m_functionInfo;
}

const Exception::errcode_error_info_T& Exception::errcodeInfo() const
{
    return m_errcodeInfo;
}

const Exception::description_error_info_T& Exception::descriptionInfo() const
{
    return m_descriptionInfo;
}

const char *Exception::what() const noexcept
{
    return m_descriptionInfo.value().c_str();
}

}}}
