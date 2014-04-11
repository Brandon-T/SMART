#ifndef SOCKETS_HPP_INCLUDED
#define SOCKETS_HPP_INCLUDED

#include "Unicode.hpp"
#include "Functions.hpp"

#include <Winsock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#define WM_SOCKET 0x10000

class Socket
{
    private:
        SOCKET socket;
        std::uint32_t Port;
        std::string Address;
        HWND WindowHandle;
        bool Listen, Initialized, Asynchronous;
        void Swap(Socket &S);
        void UnInitialized();

    public:
        Socket();
        Socket(std::uint32_t Port, std::string Address, bool Listen = false, HWND WindowHandle = nullptr, bool Asynchronous = false);
        Socket(const Socket &S) = delete;
        Socket(Socket&& S);
        ~Socket();

        Socket& operator = (const Socket &S) = delete;
        Socket& operator = (Socket&& S);

        int Recv(void* Buffer, std::uint32_t BufferLength);
        int Recv(SOCKET S, void* Buffer, std::uint32_t BufferLength);
        int Send(void* Buffer, std::size_t BufferSize);
        int Send(SOCKET S, void* Buffer, std::size_t BufferSize);
        void Connect();
        void Connect(std::uint32_t Port, std::string Address, bool Listen, HWND WindowHandle, bool Asynchronous);
        SOCKET Accept(sockaddr* ClientInfo, int* ClientInfoSize);
        void SetTimeOut(long Seconds, long Microseconds, timeval &tv, fd_set &flags);
        void Close();

        SOCKET GetSocket() const;
};

#endif // SOCKETS_HPP_INCLUDED
