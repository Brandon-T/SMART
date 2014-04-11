#include "Sockets.hpp"

Socket::~Socket()
{
    Close();
}

void Socket::Close()
{
    if (socket)
    {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = 0;
    }

    if (Initialized)
    {
        WSACleanup();
        Initialized = false;
    }
}

SOCKET Socket::GetSocket() const {return this->socket;}

Socket::Socket(Socket&& S) : socket(std::move(S.socket)), Port(std::move(S.Port)), Address(std::move(S.Address)), WindowHandle(std::move(S.WindowHandle)), Listen(std::move(S.Listen)), Initialized(std::move(S.Initialized)), Asynchronous(std::move(S.Asynchronous)) {}

Socket::Socket() : socket(0), Port(0), Address(std::string()), WindowHandle(nullptr), Listen(false), Initialized(false), Asynchronous(false) {}

Socket::Socket(std::uint32_t Port, std::string Address, bool Listen, HWND WindowHandle, bool Asynchronous) : socket(0), Port(Port), Address(Address), WindowHandle(WindowHandle), Listen(Listen), Initialized(true), Asynchronous(Asynchronous)
{
    Connect(Port, Address, Listen, WindowHandle, Asynchronous);
}

void Socket::Connect()
{
    UnInitialized();
    Connect(Port, Address, Listen, WindowHandle, Asynchronous);
}

void Socket::Connect(std::uint32_t Port, std::string Address, bool Listen, HWND WindowHandle, bool Asynchronous)
{
    if (!socket)
    {
        this->Port = Port;
        this->Address = Address;
        this->WindowHandle = WindowHandle;
        this->Asynchronous = Asynchronous;
        this->Initialized = true;

        WSADATA wsaData;
        struct sockaddr_in* sockaddr_ipv4;

        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        {
            tstring Error = ErrorMessage(WSAGetLastError());
            throw std::runtime_error("Error: " + std::string(Error.begin(), Error.end()));
        }

        if ((this->socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
        {
            this->Close();
            tstring Error = ErrorMessage(WSAGetLastError());
            throw std::runtime_error("Error: " + std::string(Error.begin(), Error.end()));
        }

        if (Address != "INADDR_ANY")
        {
            struct addrinfo *result = nullptr;
            getaddrinfo(Address.c_str(), nullptr, nullptr, &result);
            struct addrinfo* it;
            for (it = result; it != nullptr; it = it->ai_next)
            {
                sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(it->ai_addr);
                Address = inet_ntoa(sockaddr_ipv4->sin_addr);
                if (Address != "0.0.0.0") break;
            }
            freeaddrinfo(result);
        }

        SOCKADDR_IN SockAddr;
        memset(&SockAddr, 0, sizeof(SockAddr));
        SockAddr.sin_port = htons(Port);
        SockAddr.sin_family = AF_INET;
        SockAddr.sin_addr.s_addr = (Address == "INADDR_ANY" ? htonl(INADDR_ANY) : inet_addr(Address.c_str()));

        if (Listen && (bind(this->socket, reinterpret_cast<SOCKADDR*>(&SockAddr), sizeof(SockAddr)) == SOCKET_ERROR))
        {
            this->Close();
            tstring Error = ErrorMessage(WSAGetLastError());
            throw std::runtime_error("Error: " + std::string(Error.begin(), Error.end()));
        }

        if (Asynchronous && WindowHandle)
        {
            if(WSAAsyncSelect(socket, WindowHandle, WM_SOCKET, FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE | FD_ACCEPT) != 0)
            {
                this->Close();
                tstring Error = ErrorMessage(WSAGetLastError());
            throw std::runtime_error("Error: " + std::string(Error.begin(), Error.end()));
            }
        }

        if (Listen && (listen(this->socket, SOMAXCONN) == SOCKET_ERROR))
        {
            this->Close();
            tstring Error = ErrorMessage(WSAGetLastError());
            throw std::runtime_error("Error: " + std::string(Error.begin(), Error.end()));
        }

        if(!Listen && (connect(this->socket, reinterpret_cast<SOCKADDR*>(&SockAddr), sizeof(SockAddr)) == SOCKET_ERROR))
        {
            this->Close();
            tstring Error = ErrorMessage(WSAGetLastError());
            throw std::runtime_error("Error: " + std::string(Error.begin(), Error.end()));
        }
    }
}

SOCKET Socket::Accept(sockaddr* ClientInfo, int* ClientInfoSize)
{
    static int Size = sizeof(sockaddr);
    return accept(this->socket, ClientInfo, (ClientInfo && ClientInfoSize ? ClientInfoSize : &Size));
}

void Socket::SetTimeOut(long Seconds, long Microseconds, timeval &tv, fd_set &flags)
{
    tv = {0};
    tv.tv_sec = Seconds;
    tv.tv_usec = Microseconds;
    flags = {0};
    FD_ZERO(&flags);
    FD_SET(this->socket, &flags);
}

Socket& Socket::operator = (Socket&& S)
{
    S.Swap(*this);
    return *this;
}

int Socket::Recv(void* Buffer, std::uint32_t BufferLength)
{
    return recv(this->socket, reinterpret_cast<char*>(Buffer), BufferLength, 0);
}

int Socket::Recv(SOCKET S, void* Buffer, std::uint32_t BufferLength)
{
    return recv(S, reinterpret_cast<char*>(Buffer), BufferLength, 0);
}

int Socket::Send(void* Buffer, std::size_t BufferSize)
{
    return send(this->socket, reinterpret_cast<char*>(Buffer), BufferSize, 0);
}

int Socket::Send(SOCKET S, void* Buffer, std::size_t BufferSize)
{
    return send(S, reinterpret_cast<char*>(Buffer), BufferSize, 0);
}

void Socket::Swap(Socket &S)
{
    using std::swap;
    swap(socket, S.socket);
    swap(Port, S.Port);
    swap(Address, S.Address);
    swap(WindowHandle, S.WindowHandle);
    swap(Listen, S.Listen);
    swap(Initialized, S.Initialized);
    swap(Asynchronous, S.Asynchronous);
}

void Socket::UnInitialized()
{
    if (!Initialized)
    {
        throw std::runtime_error("\nError! Socket Not Constructed!");
        MessageBox(nullptr, _T("Socket Not Constructed!"), _T("Initialization Error!"), MB_ICONERROR);
        exit(0);
    }
}
