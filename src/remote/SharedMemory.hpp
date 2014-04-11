#ifndef MAPSTRUCTURES_HPP_INCLUDED
#define MAPSTRUCTURES_HPP_INCLUDED

#include "Platforms.hpp"
#include <iostream>
#include <map>

class SharedMemory
{
    private:
        void* hFileMap;
        void* pData;
        tstring MapName;
        std::size_t Size;
        bool Debug;
        std::map<tstring, void*> Events;

    public:
        SharedMemory(tstring MapName);
        SharedMemory(tstring MapName, std::size_t Size);
        ~SharedMemory();

        SharedMemory(const SharedMemory& Shm) = delete;
        SharedMemory(SharedMemory&& Shm) = delete;
        SharedMemory& operator = (const SharedMemory& Shm) = delete;
        SharedMemory& operator = (SharedMemory&& Shm) = delete;

        bool CreateFromFile(HANDLE hFile);
        void* operator -> ();
        void* GetDataPointer();
        bool OpenMemoryMap(std::size_t Size);
        bool MapMemory(std::size_t Size);
        bool ReleaseMemory();
        //bool WriteMemory(void* Source);
        //bool ReadMemory(void* Destination);
        bool CreateNewEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, bool bManualReset, bool bInitialState, tstring EventName);
        std::uint32_t OpenSingleEvent(tstring EventName, bool InheritHandle, bool SaveHandle = false, std::uint32_t dwDesiredAccess = EVENT_ALL_ACCESS, std::uint32_t dwMilliseconds = INFINITE);
        bool SetEventSignal(tstring EventName, bool Signaled);
        bool DeleteSingleEvent(tstring EventName);
        bool DeleteAllEvents();

        void SetDebug(bool On);
};

#endif // MAPSTRUCTURES_HPP_INCLUDED
