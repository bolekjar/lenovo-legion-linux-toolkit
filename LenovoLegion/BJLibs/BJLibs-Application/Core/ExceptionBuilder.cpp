// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ExceptionBuilder.h"

#include <sstream>

namespace bj { namespace framework { namespace exception
{

std::string ExceptionBuilder::print(const Exception &ex)
{
    std::stringstream description;


    description << "[In file]      < "     << ex.fileInfo().value() << " >" << std::endl
                << "\t\t\t\t\t   [In function]  < " << ex.functionInfo().value() << " >" << std::endl
                << "\t\t\t\t\t   [In line]      < " << ex.lineInfo().value()  << " >" << std::endl
                << "\t\t\t\t\t   [Description]  < errcode = " << ex.errcodeInfo().value() << " txt = \"" << ex.descriptionInfo().value() << "\" >" ;

    return description.str();
}

std::string ExceptionBuilder::print(const char *file, const char *function, int line,int int_info,const char *str_info)
{
    std::stringstream description;


    description << "[In file]      < "     << ((file == nullptr) ? "unknown" : file) << " >" << std::endl
                << "\t\t\t\t\t   [In function]  < " << ((function == nullptr) ? "unknown" : function) << " >" << std::endl
                << "\t\t\t\t\t   [In line]      < " << ((line < 1) ? -1 : line)  << " >" << std::endl
                << "\t\t\t\t\t   [Description]  < errcode = " << int_info << " txt = \"" << (str_info == nullptr ? "uknown" : str_info) << "\" >";

    return description.str();
}


std::string ExceptionBuilder::print(const char *file, const char *function, int line, int int_info,const std::exception &ex)
{
    std::stringstream description;


    description << "[In file]      < "     << ((file == nullptr) ? "unknown" : file) << " >" << std::endl
                << "\t\t\t\t\t   [In function]  < " << ((function == nullptr) ? "unknown" : function) << " >" << std::endl
                << "\t\t\t\t\t   [In line]      < " << ((line < 1) ? -1 : line)  << " >" << std::endl
                << "\t\t\t\t\t   [Description]  < errcode = " << int_info << " txt = \"" << ex.what() << "\" >";

    return description.str();
}

}}}

