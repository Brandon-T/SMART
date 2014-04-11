#include "SmartRemote.hpp"

#define Debug std::cout << "SMART: "
#if defined _WIN32 || defined _WIN64
#define Close(Handle) CloseHandle(Handle)
#else
#define Close(Handle) close(Handle)
#endif

static Client* LocalClient;
static std::vector<int> Clients;
static std::vector<int> Processes;
static const char* LocalHost = "localhost";
static std::map<int, Client*> PairedClients;

void FreeClient(Client* client)
{
    if (!client) return;
    if (client->MemoryMap && client->MemoryMap->GetDataPointer())
    {
        Debug << "Decrementing Reference Count: [" << client->Data->ID << "]\n";
        if (--client->ReferenceCount == 0)
        {
            Debug << "Freeing Client: [" << client->Data->ID << "]\n";
            if (client->sock)
            {
                delete client->sock;
                client->sock = nullptr;
            }

            PairedClients.erase(client->Data->ID);
            client->Data->Controller = 0;
            client->MemoryMap->ReleaseMemory();
            Close(client->File);
            client->MemoryMap = nullptr;
            client->Data = nullptr;
            delete client;
        }
    }
}

bool Reconnect(Client* client)
{
    Debug << "Attempting to connect to Local-Port: " << client->Data->Port << "\n";
    try
    {
        if (client->sock)
        {
            delete client->sock;
            client->sock = nullptr;
        }
        client->sock = new Socket(client->Data->Port, LocalHost);
        return true;
    }
    catch (std::exception &e)
    {
        Debug << "Connection Failed\n";
        delete client->sock;
        client->sock = nullptr;
    }
    return false;
}

void CallClient(Client* client, char FunctionID)
{
    if (client->sock->Send(&FunctionID, sizeof(char)) != sizeof(char))
    {
        Debug << "Failed To Call Function: " << (int)FunctionID << "\n";
		tstring Error = ErrorMessage(WSAGetLastError());
		if (WSAGetLastError() == 10053) Reconnect(client);
        return;
    }

    fd_set flags;
    FD_ZERO(&flags);
    struct timeval tv = {0, 100000};
    FD_SET(client->sock->GetSocket(), &flags);

    for (int I = 0; I < 600; ++I)
    {
        if (select(client->sock->GetSocket(), &flags, &flags, nullptr, &tv))
        {
            if (client->sock->Recv(&FunctionID, sizeof(char)) != sizeof(char))
            {
                Debug << "Call Appears To Have Failed Or Client Already Killed.\n";
            }
            return;
        }
        tv = {0, 100000};
    }
    Debug << "Client Timed-Out\n";
}

void KillClient(Client* client)
{
    CallClient(client, Die);
}

Client* PairClient(int ID)
{
    #if !defined _WIN32 && !defined _WIN64
        int TID = syscall(SYS_gettid);
    #else
        int TID = GetCurrentThreadId();
    #endif

    auto it = PairedClients.find(ID);
    if (it != PairedClients.end())
    {
        if (it->second->Data->Controller == TID && it->second->Data->Port)
        {
            Debug << "Already Paired To Client.. Incrementing Reference Count.\n";
            ++it->second->ReferenceCount;
            return it->second;
        }

        if (it->second->Data->Controller == 0 && it->second->Data->Port)
        {
            Debug << "Re-Pairing Client.. Incrementing Reference Count.\n";
            ++it->second->ReferenceCount;
            it->second->Data->Controller = TID;
            if (Reconnect(it->second))
            {
                return it->second;
            }
        }
        else
        {
            return nullptr;
        }
    }

    Client* client = new Client;
    std::string FileName = "SMART." + std::to_string(ID);
    #if defined _WIN32 || defined _WIN64
    client->File = CreateFile(FileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_OVERLAPPED, nullptr);
    if (client->File != INVALID_HANDLE_VALUE)
    {
    #else
    client->File = open(FileName.c_str(), O_RDWR);
    if (client->File != -1)
    {
    #endif
        client->MemoryMap = new SharedMemory(FileName, sizeof(SHMData));
        if (client->MemoryMap->CreateFromFile(client->File))
        {
            client->Data = reinterpret_cast<SHMData*>(client->MemoryMap->GetDataPointer());
            int Controller = client->Data->Controller;
            int Width = client->Data->Width;
            int Height = client->Data->Height;

            delete client->MemoryMap;
            if ((!Width || !Height) || (Controller && Controller != TID))
            {
                Close(client->File);
                delete client;
                return nullptr;
            }

            client->MemoryMap = new SharedMemory(FileName, sizeof(SHMData) + (2 * Width * Height * 4));
            if (client->MemoryMap->CreateFromFile(client->File))
            {
                client->Data = reinterpret_cast<SHMData*>(client->MemoryMap->GetDataPointer());
                client->Data->Controller = TID;
				if (client->sock)
				{
					delete client->sock;
				}

                client->sock = nullptr;
                if (!Reconnect(client))
                {
                    FreeClient(client);
                    return nullptr;
                }

                client->ReferenceCount = 1;
                #if defined _WIN32 || defined _WIN64
                FlushFileBuffers(client->File);
                #endif
                PairedClients[ID] = client;
                return client;
            }
        }
        return nullptr;
    }

    Debug<<"Failed To Pair.. No Client By That ID!\n";
    delete client;
    return nullptr;
}

Client* SpawnClient(char* RemotePath, char* Root, char* Parameters, int Width, int Height, char* InitSequence, char* UserAgent, char* JavaArgs, char* Plugins)
{
    char empty = '\0';
    if (!RemotePath) RemotePath = &empty;
    if (!JavaArgs) JavaArgs = &empty;
    if (!Plugins) Plugins = &empty;
    if (!Root) Root = &empty;
    if (!Parameters) Parameters = &empty;
    if (!InitSequence) InitSequence = &empty;
    if (!UserAgent) UserAgent = &empty;

    std::string XbootClassPath = "-Xbootclasspath/p:" + std::string(RemotePath) + "Smart.jar";
    std::string Library = std::string(RemotePath) + "/libsmartjni";
    #if defined _WIN32 || defined _WIN64
        Library += (sizeof(void*) == 8 ? "64.dll" : "32.dll");
    #else
        Library += (sizeof(void*) == 8 ? "64.so" : "32.so");
    #endif

    std::string Arguments = std::string(JavaArgs) + " " + XbootClassPath + " smart.Main " + ParseArguments(Library, std::string(Root), std::string(Parameters), std::to_string(Width), std::to_string(Height), std::string(InitSequence), std::string(UserAgent), std::string(RemotePath), std::string(Plugins));

    #if defined _WIN32 || defined _WIN64
    SHELLEXECUTEINFO info;
    memset(&info, 0, sizeof(SHELLEXECUTEINFO));
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.lpFile = "javaw.exe";
    info.lpParameters = Arguments.c_str();
    info.nShow = SW_SHOWNORMAL;
    ShellExecuteEx(&info);

    if (!info.hProcess) {
        Debug << "Failed To Spawn Process. Please make sure Java is in your Path Environment Variable.\n";
        return nullptr;
    }

    int PID = GetProcessId(info.hProcess);
    Close(info.hProcess);

    int count = 0;
    Client* client = nullptr;
    do {
        Sleep(1000);
        count++;
    } while  (!(client = PairClient(PID)) && count < 10);
    if (count >= 10) return nullptr;
    CallClient(client, Ping);
    return client;
    #else
    int v = fork();
    if (v) {
        int count = 0;
        do {
            sleep(1);
            count++;
        } while  (!(client = pairClient(v)) && count < 10);
        if (count >= 10) return nullptr;
        CallClient(client, Ping);
        return client;
    } else {
        execlp("java","java",bootclasspath,"smart.Main",Library.c_str(), Root, Parameters, Width, Height, InitSequence, UserAgent, RemotePath, Plugins, nullptr);
        debug << "Process terminating. If nothing happened, make sure java is on your path and that SMART is installed correctly.\n";
        exit(1);
    }
    #endif
}

int ClientCount(bool UnpairedOnly)
{
    Clients.clear();
    auto Files = FindFiles("SMART.*");
    for (auto File : Files)
    {
        HANDLE hFile = CreateFile(File.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_OVERLAPPED, nullptr);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            SharedMemory SHM(File, sizeof(int));
            if (SHM.CreateFromFile(hFile))
            {
                SHMData* Data = static_cast<SHMData*>(SHM.GetDataPointer());
				if (UnpairedOnly && !Data->Controller && Data->Port)
				{
					Clients.emplace_back(std::stoi(File.substr(File.find(".") + 1)));
				}
				else if (!UnpairedOnly && Data->Controller && Data->Port)
				{
					Clients.emplace_back(std::stoi(File.substr(File.find(".") + 1)));
				}
            }
            Close(hFile);
        }
    }
    return Clients.size();
}

int ClientID(int Index)
{
    if (Index >= 0 && Index < Clients.size())
    {
        return Clients[Index];
    }
    return -1;
}



/** SIMBA_SMART EXPORTS **/

int exp_clientID(int Index)
{
    return ClientID(Index);
}

int exp_clientCount(bool UnpairedOnly)
{
    return ClientCount(UnpairedOnly);
}

int exp_currentClient()
{
    return LocalClient ? LocalClient->Data->ID : 0;
}

bool exp_killClient(int ID)
{
    Client* client = PairClient(ID);
    if (client != nullptr)
    {
        KillClient(client);
        FreeClient(client);
        return true;
    }
    return false;
}

int exp_spawnClient(char* RemotePath, char* Root, char* Parameters, int Width, int Height, char* InitSequence, char* UserAgent, char* JavaArgs, char* Plugins)
{
    FreeClient(LocalClient);
    LocalClient = SpawnClient(RemotePath, Root, Parameters, Width, Height, InitSequence, UserAgent, JavaArgs, Plugins);
    return LocalClient ? LocalClient->Data->ID : 0;
}

bool exp_pairClient(int ID)
{
    Client* client = PairClient(ID);
    if (client != LocalClient) FreeClient(LocalClient);
    LocalClient = client;
    return LocalClient ? LocalClient->Data->ID : false;
}

void* exp_getImageArray()
{
    return LocalClient ? (char*)LocalClient->Data + LocalClient->Data->ImageOffset : nullptr;
}

void* exp_getDebugArray()
{
    return LocalClient ? (char*)LocalClient->Data + LocalClient->Data->DebugOffset : nullptr;
}

void exp_setTransparentColor(int color)
{
    if (LocalClient)
    {
        *reinterpret_cast<int*>(LocalClient->Data->Args) = color;
        CallClient(LocalClient, setTransparentColor);
    }
}

void exp_setDebug(bool enabled)
{
    if (LocalClient)
    {
        *reinterpret_cast<bool*>(LocalClient->Data->Args) = enabled;
        CallClient(LocalClient, setDebug);
    }
}


void exp_setGraphics(bool enabled)
{
    if (LocalClient)
    {
        *reinterpret_cast<bool*>(LocalClient->Data->Args) = enabled;
        CallClient(LocalClient, setGraphics);
    }
}

bool exp_isActive()
{
    if (LocalClient)
    {
        CallClient(LocalClient, isActive);
        return *reinterpret_cast<bool*>(LocalClient->Data->Args);
    }
    else return false;
}

void exp_setKeyInput(bool enabled)
{
    if (LocalClient)
    {
        *reinterpret_cast<bool*>(LocalClient->Data->Args) = enabled;
        CallClient(LocalClient, setKeyInput);
    }
}

void exp_setMouseInput(bool enabled)
{
    if (LocalClient)
    {
        *reinterpret_cast<bool*>(LocalClient->Data->Args) = enabled;
        CallClient(LocalClient, setMouseInput);
    }
}

void exp_getMousePos(int &x, int &y)
{
    if (LocalClient)
    {
        CallClient(LocalClient, getMousePos);
        x = (reinterpret_cast<int*>(LocalClient->Data->Args))[0];
        y = (reinterpret_cast<int*>(LocalClient->Data->Args))[1];
    }
}

void exp_holdMouse(int x, int y, bool left)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[2] = left;
        CallClient(LocalClient, holdMouse);
    }
}

void exp_releaseMouse(int x, int y, bool left)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[2] = left;
        CallClient(LocalClient, releaseMouse);
    }
}

bool exp_isKeyboardEnabled()
{
    if (LocalClient)
    {
        CallClient(LocalClient, isKeyboardEnabled);
        return *reinterpret_cast<bool*>(LocalClient->Data->Args);
    }
    else return false;
}

bool exp_isMouseEnabled()
{
    if (LocalClient)
    {
        CallClient(LocalClient, isMouseEnabled);
        return *reinterpret_cast<bool*>(LocalClient->Data->Args);
    }
    else return false;
}

void exp_holdMousePlus(int x, int y, int button)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[2] = button;
        CallClient(LocalClient, holdMousePlus);
    }
}

void exp_releaseMousePlus(int x, int y, int button)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[2] = button;
        CallClient(LocalClient, releaseMousePlus);
    }
}

void exp_moveMouse(int x, int y)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        CallClient(LocalClient, moveMouse);
    }
}

void exp_windMouse(int x, int y)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        CallClient(LocalClient, windMouse);
    }
}

void exp_clickMouse(int x, int y, bool left)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[2] = left;
        CallClient(LocalClient, clickMouse);
    }
}

void exp_clickMousePlus(int x, int y, int button)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = x;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = y;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[2] = button;
        CallClient(LocalClient, clickMousePlus);
    }
}

bool exp_isMouseButtonHeld(int button)
{
    if (LocalClient)
    {
        *reinterpret_cast<int*>(LocalClient->Data->Args) = button;
        CallClient(LocalClient, isMouseButtonHeld);
        return *reinterpret_cast<bool*>(LocalClient->Data->Args);
    }
    else return false;
}

void exp_sendKeys(char * str, int keywait, int keymodwait)
{
    if (LocalClient)
    {
        (reinterpret_cast<int*>(LocalClient->Data->Args))[0] = keywait;
        (reinterpret_cast<int*>(LocalClient->Data->Args))[1] = keymodwait;
        strcpy((char*)LocalClient->Data->Args + sizeof(int) * 2, str);
        CallClient(LocalClient, sendKeys);
    }
}

void exp_holdKey(int code)
{
    if (LocalClient)
    {
        *reinterpret_cast<int*>(LocalClient->Data->Args) = code;
        CallClient(LocalClient, holdKey);
    }
}

void exp_releaseKey(int code)
{
    if (LocalClient)
    {
        *reinterpret_cast<int*>(LocalClient->Data->Args) = code;
        CallClient(LocalClient, releaseKey);
    }
}

bool exp_isKeyDown(int code)
{
    if (LocalClient)
    {
        *reinterpret_cast<int*>(LocalClient->Data->Args) = code;
        CallClient(LocalClient, isKeyDown);
        return *reinterpret_cast<bool*>(LocalClient->Data->Args);
    }
    else return false;
}

void exp_writeConsole(char* Text)
{
    if (LocalClient)
    {
        strcpy((char*)LocalClient->Data->Args, Text);
        CallClient(LocalClient, writeConsole);
    }
}

void exp_clearConsole()
{
    if (LocalClient)
    {
        CallClient(LocalClient, clearConsole);
    }
}

void exp_setOperatingMode(int value)
{
    if (LocalClient)
    {
        *reinterpret_cast<int*>(LocalClient->Data->Args) = value;
        CallClient(LocalClient, setMode);
    }
}

/** TODO: REMOVE THE FOLLOWING 3 DEPRECATED FUNCTIONS IF THIS SMART BECOMES OFFICIAL.. **/
bool exp_smartEnabled() {return exp_isActive();}
int exp_getRefresh() {return 0;}
void exp_setRefresh(int value) {}

Client* EIOS_RequestTarget(char* initargs)
{
    Debug << "EIOS Requesting Target\n";
    if (initargs != 0 && strlen(initargs) > 0)
    {
        Client* client = PairClient(atoi(initargs));
        Debug << "Target Identifier: " << client << '\n';
        return client;
    }
    return nullptr; //This result signifies a failure
}

void EIOS_ReleaseTarget(Client* client)
{
    Debug << "EIOS Releasing Target\n";
    FreeClient(client);
}

void EIOS_GetTargetDimensions(Client* client, int * width, int * height)
{
    if (client)
    {
        *width = client->Data->Width;
        *height = client->Data->Height;
    }
}

char* EIOS_GetImageBuffer(Client* client)
{
    Debug << "EIOS requested image buffer\n";
    if (client)
    {
        Debug << "Base: " << client->Data << " Off: " << client->Data->ImageOffset << '\n';
    }
    return client ? (reinterpret_cast<char*>(client->Data) + client->Data->ImageOffset) : 0;
}

void EIOS_UpdateImageBuffer(Client* client) {}

void EIOS_GetMousePosition(Client* client, int * x, int * y)
{
    if (client)
    {
        CallClient(client, getMousePos);
        *x = ((int*)(client->Data->Args))[0];
        *y = ((int*)(client->Data->Args))[1];
    }
}

void EIOS_MoveMouse(Client* client, int x, int y)
{
    if (client)
    {
        ((int*)(client->Data->Args))[0] = x;
        ((int*)(client->Data->Args))[1] = y;
        CallClient(client, moveMouse);
    }
}

void EIOS_HoldMouse(Client* client, int x, int y, int button)
{
    if (client)
    {
        ((int*)(client->Data->Args))[0] = x;
        ((int*)(client->Data->Args))[1] = y;
        ((int*)(client->Data->Args))[2] = button;
        CallClient(client, holdMousePlus);
    }
}

void EIOS_ReleaseMouse(Client* client, int x, int y, int button)
{
    if (client)
    {
        ((int*)(client->Data->Args))[0] = x;
        ((int*)(client->Data->Args))[1] = y;
        ((int*)(client->Data->Args))[2] = button;
        CallClient(client, releaseMousePlus);
    }
}

bool EIOS_IsMouseHeld(Client* client, int button)
{
    if (client)
    {
        *(int*)(client->Data->Args) = button;
        CallClient(client, isMouseButtonHeld);
        return *(bool*)(client->Data->Args);
    }
    else return false;
}

void EIOS_SendString(Client* client, char * str, int keywait, int keymodwait)
{
    if (client)
    {
        ((int*)client->Data->Args)[0] = keywait;
        ((int*)client->Data->Args)[1] = keymodwait;
        strcpy((char*)client->Data->Args + sizeof(int) * 2, str);
        CallClient(client, sendKeys);
    }
}

void EIOS_HoldKey(Client* client, int key)
{
    if (client)
    {
        *(int*)(client->Data->Args) = key;
        CallClient(client, holdKey);
    }
}

void EIOS_ReleaseKey(Client* client, int key)
{
    if (client)
    {
        *(int*)(client->Data->Args) = key;
        CallClient(client, releaseKey);
    }
}

bool EIOS_IsKeyHeld(Client* client, int key)
{
    if (client)
    {
        *(int*)(client->Data->Args) = key;
        CallClient(client, isKeyDown);
        return *(bool*)(client->Data->Args);
    }
    else return false;
}

void internalConstructor()
{
    for (int I = 0; I < 5; ++I)
    {
		std::vector<tstring> SmartClients = FindFiles("SMART.*");

		for (auto S : SmartClients)
		{
			remove(S.c_str());
		}
        Sleep(100);
    }

    exp_clientCount(true);
    LocalClient = nullptr;
    PairedClients = std::map<int, Client*>();
}

void internalDestructor()
{
    FreeClient(LocalClient);

    for (int I = 0; I < 5; ++I)
    {
		std::vector<tstring> SmartClients = FindFiles("SMART.*");

		for (auto S : SmartClients)
		{
			remove(S.c_str());
		}
        Sleep(100);
    }

    PairedClients.clear();
    Clients.clear();
    Processes.clear();
}


int GetFunctionCount()
{
    return NumExports;
}

int GetPluginABIVersion()
{
    return 2;
}

#if !defined _WIN32 && !defined _WIN64
int GetFunctionInfo(int index, void*& address, char*& def)
{
    if (index < NumExports)
    {
        address = dlsym(RTLD_DEFAULT, exports[index * 2]);
        strcpy(def, exports[index * 2 + 1]);
        return index;
    }
    return -1;
}

void load(void)
{
    internalConstructor();
}

void unload(void)
{
    internalDestructor();
}

#else
HMODULE dllinst;

int GetFunctionInfo(int index, void*& address, char*& def)
{
    if (index < NumExports)
    {
        address = (void*)GetProcAddress(dllinst, exports[index * 2]);
        strcpy(def, exports[index * 2 + 1]);
        return index;
    }

    return -1;
}

bool DllMain(HINSTANCE instance, int reason, void * checks)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            dllinst = instance;
            internalConstructor();
            return true;
        }
        case DLL_THREAD_ATTACH:
            return true;for (int I = 0; I < 5; ++I)
    {
		std::vector<tstring> SmartClients = FindFiles("SMART.*");

		for (auto S : SmartClients)
		{
			remove(S.c_str());
		}
        Sleep(100);
    }
        case DLL_PROCESS_DETACH:
            internalDestructor();
            return true;
        case DLL_THREAD_DETACH:
            return true;
    }
    return false;
}
#endif
