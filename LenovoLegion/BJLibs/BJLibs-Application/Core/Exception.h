// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QException>

#include <string>


namespace bj { namespace framework { namespace exception {

struct tag_exception_info {};
struct tag_line_info      {};
struct tag_file_info      {};
struct tag_function_info  {};


template <class Tag,class T>
struct error_info
{
    friend class Exception;

    typedef T value_type;

    error_info( value_type const & v ) : m_value(v) {}
    const value_type& value() const {return m_value;}

private:
    error_info() : m_value(T()) {}

private:

    value_type m_value;
};


class Exception : public QException {

public:

    using  line_error_info_T     = error_info<tag_line_info,int>;
    using  file_error_info_T     = error_info<tag_file_info,std::string>;
    using  function_error_info_T = error_info<tag_function_info,std::string>;

    using  errcode_error_info_T         = error_info<tag_exception_info,int>;
    using  description_error_info_T     = error_info<tag_exception_info,std::string>;

public:

    Exception& operator<<(const line_error_info_T& x);
    Exception& operator<<(const file_error_info_T& x);
    Exception& operator<<(const function_error_info_T& x);
    Exception& operator<<(const errcode_error_info_T& x);
    Exception& operator<<(const description_error_info_T& x);

    const line_error_info_T         lineInfo()          const;
    const file_error_info_T&        fileInfo()          const;
    const function_error_info_T&    functionInfo()      const;
    const errcode_error_info_T&     errcodeInfo()       const;
    const description_error_info_T& descriptionInfo()   const;

    virtual const char* what() const noexcept override;

private:

    line_error_info_T       m_lineInfo;
    file_error_info_T       m_fileInfo;
    function_error_info_T   m_functionInfo;

    errcode_error_info_T        m_errcodeInfo;
    description_error_info_T    m_descriptionInfo;
};

}}}
