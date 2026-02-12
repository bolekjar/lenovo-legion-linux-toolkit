// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "StringUtils.h"

#include <string>

namespace  {

/*
Transform any std string or string view
into any of the 4 std string types,
(c) 2018-2022 by dbj at dbj dot org
https://dbj.org/license_dbj or CC BY SA 4.0
*/
template<typename T, typename F>
inline T
transform_to ( F str ) noexcept
{
    // note: F has to have
    // the empty()method
    if (str.empty())
        return {};
    // note: F must be able to work
    // with std begin and end
    return { std::begin(str), std::end(str) };
    // also the above line requires, T has a constructor
    // that will take begin and end values of type F.
};

}

namespace LenovoLegionDaemon {

const char *StringUtils::wchar_to_char(const wchar_t *pwchar)
{
    if (pwchar == nullptr) {
        return "";
    }
    // get the number of characters in the string.
    int currentCharIndex = 0;
    char currentChar = (char) pwchar[currentCharIndex];

    while (currentChar != '\0') {
        currentCharIndex++;
        currentChar = (char) pwchar[currentCharIndex];
    }

    const int charCount = currentCharIndex + 1;

    // allocate a new block of memory size char (1 byte) instead of wide char (2 bytes)
    char *filePathC = (char *) malloc(sizeof(char) * charCount);

    for (int i = 0; i < charCount; i++) {
        // convert to char (1 byte)
        char character = (char) pwchar[i];

        *filePathC = character;

        filePathC += sizeof(char);
    }
    filePathC += '\0';

    filePathC -= (sizeof(char) * charCount);

    return filePathC;
}

std::string StringUtils::wstring_to_string(const std::wstring wstring)
{
    return transform_to<std::string>(wstring);
}

std::string StringUtils::u16string_to_string(const std::u16string wstring)
{
    return transform_to<std::string>(wstring);
}

const std::string StringUtils::remove_null_terminating_chars(std::string input)
{
    while (!input.empty() && input.back() == 0) {
        input.pop_back();
    }

    return input;
}

}
