#include "Functions.hpp"

tstring GetEnvironmentVariables(const tstring Variable)
{
    return _tgetenv(Variable.c_str());
}

tstring ErrorMessage(std::uint32_t Error, bool Throw)
{
    LPTSTR lpMsgBuf = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);
    if (Throw)
    {
        tstring Buffer(lpMsgBuf);
        throw std::runtime_error(std::string(Buffer.begin(), Buffer.end()));
    }
    return lpMsgBuf;
}

std::vector<tstring> SplitString(tstring StringToSplit, tstring Delimiter)
{
    std::vector<tstring> Result;
    std::size_t Pos = StringToSplit.find_first_of(Delimiter);
    while(Pos != tstring::npos)
    {
        if(Pos > 0)
        {
            Result.emplace_back(StringToSplit.substr(0, Pos));
        }
        StringToSplit = StringToSplit.substr(Pos + 1);
        Pos = StringToSplit.find_first_of(Delimiter);
    }
    if(StringToSplit.length() > 0)
    {
        Result.emplace_back(StringToSplit);
    }
    return Result;
}

tstring Implode(const tstring &Glue, const std::vector<tstring> &Pieces)
{
    tstring Str;
    int Len = Pieces.size();
    for(int I = 0; I < Len; ++I)
    {
        Str += Pieces[I];
        if (I < (Len - 1))
            Str += Glue;
    }
    return Str;
}

std::vector<tstring> FindFiles(tstring FileName)
{
    std::vector<tstring> Result;
    WIN32_FIND_DATA hFound = {0};
    HANDLE hFile = FindFirstFile(FileName.c_str(), &hFound);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            Result.emplace_back(hFound.cFileName);
        } while(FindNextFile(hFile, &hFound));
    }
    FindClose(hFile);
    return Result;
}
