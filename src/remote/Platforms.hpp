#ifndef PLATFORMS_HPP_INCLUDED
#define PLATFORMS_HPP_INCLUDED

#include "Unicode.hpp"

#if defined _WIN32 || defined _WIN64

    #ifdef SOCKETS_HPP_INCLUDED
        #include <Winsock2.h>
        #include <Windows.h>
        #include <Ws2tcpip.h>
    #endif

    #include <Windows.h>
    #include <process.h>
    #ifdef _MSC_VER
        #include <stdint.h>
    #endif

    #define xLoadLibrary(Library) LoadLibrary(Library)
    #define xGetProcAddress(Library, FunctionName) GetProcAddress(Library, FunctionName)
    #define xFreeLibrary(Library) FreeLibrary(Library)

#else

    #include <dirent.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <dlfcn.h>
    #include <cstdarg>
    #include <cstring>
    #include <iostream>
    #include <fcntl.h>
    #include <sys/mman.h>

    #define BI_RGB 0
    #define MAX_PATH 256
    #define __stdcall
    #define STDCALL __stdcall
    #define xLoadLibrary(Library) dlopen(Library, RTLD_LAZY)
    #define xGetProcAddress(Library, FunctionName) dlsym(Library, FunctionName)
    #define xFreeLibrary(Library) dlclose(Library)
    #define RGB(r, g, b) ((std::uint32_t)((std::uint8_t)(r)|((std::uint8_t)(g) << 8)|((std::uint8_t)(b) << 16)))

    typedef struct tagBITMAPFILEHEADER
    {
        std::uint16_t bfType;
        std::uint32_t bfSize;
        std::uint16_t bfReserved1;
        std::uint16_t bfReserved2;
        std::uint32_t bfOffBits;
    } BITMAPFILEHEADER;

    typedef struct tagBITMAPINFOHEADER
    {
        std::uint32_t biSize;
        long  biWidth;
        long  biHeight;
        std::uint16_t  biPlanes;
        std::uint16_t  biBitCount;
        std::uint32_t biCompression;
        std::uint32_t biSizeImage;
        long  biXPelsPerMeter;
        long  biYPelsPerMeter;
        std::uint32_t biClrUsed;
        std::uint32_t biClrImportant;
    } BITMAPINFOHEADER;

    typedef struct tagRGBQUAD
    {
        std::uint8_t rgbBlue;
        std::uint8_t rgbGreen;
        std::uint8_t rgbRed;
        std::uint8_t rgbReserved;
    } RGBQUAD;

    typedef struct tagBITMAPINFO
    {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD          bmiColors[1];
    } BITMAPINFO, *PBITMAPINFO;
#endif

#endif // PLATFORMS_HPP_INCLUDED
