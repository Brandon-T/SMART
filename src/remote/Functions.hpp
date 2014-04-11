#ifndef FUNCTIONS_HPP_INCLUDED
#define FUNCTIONS_HPP_INCLUDED

#include "Unicode.hpp"
#include "Platforms.hpp"
#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

template<typename... Args>
static tstring ParseArguments(tstring FirstArgument, const Args... RemainingArguments)
{
    tstring Result = tstring();
    std::initializer_list<tstring> List = {FirstArgument, RemainingArguments...};
    for(tstring Argument : List)
    {
        Result += "\"" + Argument + "\" ";
    }

    return Trim(Result);
}

template <typename T>
T ReadPointer(char*& Pointer)
{
    T Result = *(reinterpret_cast<T*>(Pointer));
    Pointer += sizeof(T);
    return Result;
}

template <typename T>
void WritePointer(char*& Pointer, const T& Value)
{
    *(reinterpret_cast<T*>(Pointer)) = Value;
    Pointer += sizeof(T);
}

tstring GetEnvironmentVariables(const tstring Variable);

tstring ErrorMessage(std::uint32_t Error, bool Throw = false);

std::vector<tstring> SplitString(tstring StringToSplit, tstring Delimiter);

tstring Implode(const tstring &Glue, const std::vector<tstring> &Pieces);

std::vector<tstring> FindFiles(tstring FileName);

#endif // FUNCTIONS_HPP_INCLUDED
