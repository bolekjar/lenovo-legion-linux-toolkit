// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "Exception.h"


#include <exception>
#include <string>


namespace bj { namespace framework { namespace exception
{


struct ExceptionBuilder
{
    typedef error_info<tag_exception_info,int>                exception_info_T;
    typedef error_info<tag_exception_info,std::string>        exception_info_str_T;


    static std::string print(const Exception& ex);
    static std::string print(const char *file, const char *function, int line,int int_info,const char *str_info);
    static std::string print(const char *file, const char *function, int line,int int_info,const std::exception& ex);
};


#define DEFINE_EXCEPTION(x) struct x ## Exception: public bj::framework::exception::Exception {\
    virtual void raise() const override {    throw *this;}\
    virtual x ## Exception *clone() const override {    return new x ## Exception(*this);}\
};\
typedef x ## Exception                                                             exception_T;         \
typedef bj::framework::exception::ExceptionBuilder                              ExceptionBuilder;    \
typedef ExceptionBuilder::exception_info_T                                         exception_info_T;    \
typedef ExceptionBuilder::exception_info_str_T                                     exception_info_str_T;\
enum ERROR_CODES_GENERIC : int { INTERNAL_ERROR = 0 };


#define DEFINE_EXCEPTION_WITH_INHERITANCE(x,i) struct x ## Exception: public i {\
virtual void raise() const override {    throw *this;}\
    virtual x ## Exception *clone() const override {    return new x ## Exception(*this);}\
};\
    typedef x ## Exception                                                             exception_T;         \
    typedef bj::framework::exception::ExceptionBuilder                              ExceptionBuilder;    \
    typedef ExceptionBuilder::exception_info_T                                         exception_info_T;    \
    typedef ExceptionBuilder::exception_info_str_T                                     exception_info_str_T;\
    enum ERROR_CODES_GENERIC : int { INTERNAL_ERROR = 0 };


#define THROW_EXCEPTION(exname,errcode,description) { exname _ex_;                                                                                \
                                                    _ex_ << bj::framework::exception::ExceptionBuilder::exception_info_T(errcode)           << \
                                                    bj::framework::exception::ExceptionBuilder::exception_info_str_T(description)         << \
                                                    bj::framework::exception::Exception::line_error_info_T(__LINE__)                      << \
                                                    bj::framework::exception::Exception::file_error_info_T(__FILE__)                      << \
                                                    bj::framework::exception::Exception::function_error_info_T(__FUNCTION__);throw _ex_;}
}}}
