#ifndef UNICODE_HPP_INCLUDED
#define UNICODE_HPP_INCLUDED

//#define UNICODE
//#define _UNICODE

#include <tchar.h>
#include <string>

#ifdef UNICODE
    #define tcout std::wcout
    #define tcin std::wcin
    #define tcerr std::wcerr
    #define to_tstring std::to_wstring
#else
    #define tcout std::cout
    #define tcin std::cin
    #define tcerr std::cerr
    #define to_tstring std::to_string
#endif

typedef std::basic_string<TCHAR> tstring;
typedef std::basic_filebuf<TCHAR> tfilebuf;
typedef std::basic_streambuf<TCHAR> tstreambuf;
typedef std::basic_stringstream<TCHAR> tstringstream;
typedef std::basic_istringstream<TCHAR> tistringstream;
typedef std::basic_ostringstream<TCHAR> tostringstream;
typedef std::basic_istream<TCHAR> tistream;
typedef std::basic_ostream<TCHAR> tostream;
typedef std::basic_fstream<TCHAR> tfstream;
typedef std::basic_ifstream<TCHAR> tifstream;
typedef std::basic_ofstream<TCHAR> tofstream;

#endif // UNICODE_HPP_INCLUDED
