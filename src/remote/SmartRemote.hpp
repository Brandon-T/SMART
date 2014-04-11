#ifndef SMARTREMOTE_HPP_INCLUDED
#define SMARTREMOTE_HPP_INCLUDED

#include <map>
#include <vector>
#include "Sockets.hpp"
#include "SharedMemory.hpp"

#define FirstFunc            1
#define setTransparentColor  FirstFunc+0
#define setDebug             FirstFunc+1
#define setGraphics          FirstFunc+2
#define setKeyInput          FirstFunc+3
#define setMouseInput        FirstFunc+4
#define isActive             FirstFunc+5
#define isKeyboardEnabled    FirstFunc+6
#define isMouseEnabled   	 FirstFunc+7
#define getMousePos          FirstFunc+8
#define holdMouse            FirstFunc+9
#define releaseMouse         FirstFunc+10
#define holdMousePlus        FirstFunc+11
#define releaseMousePlus     FirstFunc+12
#define moveMouse            FirstFunc+13
#define windMouse            FirstFunc+14
#define clickMouse           FirstFunc+15
#define clickMousePlus       FirstFunc+16
#define isMouseButtonHeld    FirstFunc+17
#define sendKeys             FirstFunc+18
#define holdKey              FirstFunc+19
#define releaseKey           FirstFunc+20
#define isKeyDown            FirstFunc+21
#define writeConsole         FirstFunc+22
#define clearConsole         FirstFunc+23
#define setMode              FirstFunc+24

#define ExtraFuncs           FirstFunc+25
#define Ping                 ExtraFuncs+0
#define Die                  ExtraFuncs+1
#define Kill                 ExtraFuncs+2

typedef struct
{
    int Port = 0;
    int ID = 0;
    int Width = 0, Height = 0;
    int Controller = 0;
    int ImageOffset = 0;
    int DebugOffset = 0;
    std::uint8_t Args[4096] = {0};
} SHMData;

typedef struct
{
    int ReferenceCount = 0;
    #ifndef _WIN32
        int File = 0;
    #else
        HANDLE File = nullptr;
    #endif
    Socket* sock = nullptr;
    SharedMemory* MemoryMap = nullptr;
    SHMData* Data = nullptr;
} Client;


#ifdef SMART_CPP
void FreeClient(Client* client);
bool Reconnect(Client* client);
void CallClient(Client* client, char FunctionID);
void KillClient(Client* client);
Client* PairClient(int ID);
Client* SpawnClient(char* RemotePath, char* Root, char* Parameters, int Width, int Height, char* InitSequence, char* UserAgent, char* JavaArgs, char* Plugins);
int ClientCount(bool UnpairedOnly);
int ClientID(int Index);
#endif

#if !defined _WIN32 && !defined _WIN64
#include <dlfcn.h>
void load() __attribute__((constructor));
void unload() __attribute__((destructor));
#else
extern "C" bool DllMain(HINSTANCE, int, void*) __attribute__((stdcall));
#endif
extern "C" int GetPluginABIVersion();
extern "C" int GetFunctionCount();
extern "C" int GetFunctionInfo(int, void*&, char*&);

extern "C" Client* EIOS_RequestTarget(char *initargs) __attribute__((stdcall));
extern "C" void EIOS_ReleaseTarget(Client* client) __attribute__((stdcall));
extern "C" void EIOS_GetTargetDimensions(Client* client, int* width, int* height) __attribute__((stdcall));
extern "C" char* EIOS_GetImageBuffer(Client* client) __attribute__((stdcall));
extern "C" void EIOS_UpdateImageBuffer(Client* client) __attribute__((stdcall));
extern "C" void EIOS_GetMousePosition(Client* client, int* x, int* y) __attribute__((stdcall));
extern "C" void EIOS_MoveMouse(Client* client, int x, int y) __attribute__((stdcall));
extern "C" void EIOS_HoldMouse(Client* client, int x, int y, int button) __attribute__((stdcall));
extern "C" void EIOS_ReleaseMouse(Client* client, int x, int y, int button) __attribute__((stdcall));
extern "C" bool EIOS_IsMouseHeld(Client* client, int button) __attribute__((stdcall));
extern "C" void EIOS_SendString(Client* client, char* str, int keywait, int keymodwait) __attribute__((stdcall));
extern "C" void EIOS_HoldKey(Client* client, int key) __attribute__((stdcall));
extern "C" void EIOS_ReleaseKey(Client* client, int key) __attribute__((stdcall));
extern "C" bool EIOS_IsKeyHeld(Client* client, int key) __attribute__((stdcall));


extern "C" int exp_clientID(int Index);
extern "C" int exp_clientCount(bool UnpairedOnly);
extern "C" int exp_spawnClient(char* RemotePath, char* Root, char* Parameters, int Width, int Height, char* InitSequence, char* UserAgent, char* JavaArgs, char* Plugins);
extern "C" bool exp_pairClient(int ID);
extern "C" int exp_currentClient();
extern "C" bool exp_killClient(int ID);
extern "C" void* exp_getImageArray();
extern "C" void* exp_getDebugArray();
extern "C" void exp_setTransparentColor(int color);
extern "C" void exp_setDebug(bool enabled);
extern "C" void exp_setGraphics(bool enabled);
extern "C" void exp_setKeyInput(bool enabled);
extern "C" void exp_setMouseInput(bool enabled);
extern "C" bool exp_isActive();
extern "C" bool exp_isKeyboardEnabled();
extern "C" bool exp_isMouseEnabled();
extern "C" void exp_getMousePos(int &x, int &y);
extern "C" void exp_holdMouse(int x, int y, bool left);
extern "C" void exp_releaseMouse(int x, int y, bool left);
extern "C" void exp_holdMousePlus(int x, int y, int button);
extern "C" void exp_releaseMousePlus(int x, int y, int button);
extern "C" void exp_moveMouse(int x, int y);
extern "C" void exp_windMouse(int x, int y);
extern "C" void exp_clickMouse(int x, int y, bool left);
extern "C" void exp_clickMousePlus(int x, int y, int button);
extern "C" bool exp_isMouseButtonHeld(int button);
extern "C" void exp_sendKeys(char *text, int keywait, int keymodwait);
extern "C" void exp_holdKey(int code);
extern "C" void exp_releaseKey(int code);
extern "C" bool exp_isKeyDown(int code);
extern "C" void exp_writeConsole(char* Text);
extern "C" void exp_clearConsole();
extern "C" void exp_setOperatingMode(int value);

//Exports for Local
#define NumExports 36
static char* exports[] = {
    (char*)"exp_clientID", (char*)"Function SmartClientID(Index: Integer): Integer;",
    (char*)"exp_clientCount", (char*)"Function SmartClientCount(UnpairedOnly: Boolean): Integer;",
    (char*)"exp_spawnClient", (char*)"Function SmartSpawnClient(RemotePath, Root, Parameters: String; Width, Height: Integer; InitSequence, UserAgent, JavaArgs, Plugins: String): Integer;",
    (char*)"exp_pairClient", (char*)"Function SmartPairClient(CID: Integer): Boolean;",
    (char*)"exp_currentClient", (char*)"Function SmartCurrentClient: Integer;",
    (char*)"exp_killClient", (char*)"Function SmartKillClient(CID: Integer): Boolean;",
    (char*)"exp_getImageArray", (char*)"Function SmartImageArray: Int"
    #if __SIZEOF_POINTER__ == 4
        "eger;",
    #else
        "64;",
    #endif

    (char*)"exp_getDebugArray", (char*)"Function SmartDebugArray: Int"
    #if __SIZEOF_POINTER__ == 4
        "eger;",
    #else
        "64;",
    #endif

    (char*)"exp_setTransparentColor", (char*)"Procedure SmartSetTransparentColor(Color: Integer);",
    (char*)"exp_setDebug", (char*)"Procedure SmartSetDebug(Enabled: Boolean);",
    (char*)"exp_setGraphics", (char*)"Procedure SmartSetGraphics(Enabled: Boolean);",
    (char*)"exp_setKeyInput", (char*)"Procedure SmartSetKeyInput(Enabled: Boolean);",
    (char*)"exp_setMouseInput", (char*)"Procedure SmartSetMouseInput(Enabled: Boolean);",
    (char*)"exp_isActive", (char*)"Function SmartIsActive: Boolean;",
    (char*)"exp_isKeyboardEnabled", (char*)"Function SmartIsKeyboardEnabled: Boolean;",
    (char*)"exp_isMouseEnabled", (char*)"Function SmartIsMouseEnabled: Boolean",
    (char*)"exp_getMousePos", (char*)"Procedure SmartGetMousePos(var X, Y: Integer);",
    (char*)"exp_holdMouse", (char*)"Procedure SmartHoldMouse(X, Y: Integer; Left: Boolean);",
    (char*)"exp_releaseMouse", (char*)"Procedure SmartReleaseMouse(X, Y: Integer; Left: Boolean);",
    (char*)"exp_holdMousePlus", (char*)"Procedure SmartHoldMousePlus(X, Y, Button: Integer);",
    (char*)"exp_releaseMousePlus", (char*)"Procedure SmartReleaseMouse(X, Y, Button: Integer);",
    (char*)"exp_moveMouse", (char*)"Procedure SmartMoveMouse(X, Y: Integer);",
    (char*)"exp_windMouse", (char*)"Procedure SmartWindMouse(X, Y: Integer);",
    (char*)"exp_clickMouse", (char*)"Procedure SmartClickMouse(X, Y: Integer; Left: Boolean);",
    (char*)"exp_clickMousePlus", (char*)"Procedure SmartClickMousePlus(X, Y, Button: Integer);",
    (char*)"exp_isMouseButtonHeld", (char*)"Function SmartIsMouseButtonHeld(Button: Integer): Boolean;",
    (char*)"exp_sendKeys", (char*)"Procedure SmartSendKeys(Text: String; KeyWait, KeyMouseWait: Integer);",
    (char*)"exp_holdKey", (char*)"Procedure SmartHoldKey(Code: Integer);",
    (char*)"exp_releaseKey", (char*)"Procedure SmartReleaseKey(Code: Integer);",
    (char*)"exp_isKeyDown", (char*)"Function SmartIsKeyDown(Code: Integer): Boolean;",
    (char*)"exp_writeConsole", (char*)"Procedure SmartWriteConsole(Text: String);",
    (char*)"exp_clearConsole", (char*)"Procedure SmartClearConsole();",
    (char*)"exp_setOperatingMode", (char*)"Procedure SmartSetOperatingMode(Value: Integer);",


    /** TODO: REMOVE THE FOLLOWING 3 DEPRECATED FUNCTIONS IF THIS SMART BECOMES OFFICIAL.. **/
    (char*)"exp_smartEnabled", (char*)"Function SmartEnabled: Boolean;",
    (char*)"exp_getRefresh", (char*)"Function SmartGetRefresh: Integer;",
    (char*)"exp_setRefresh", (char*)"Procedure SmartSetRefresh(Value: Integer);"
};

/** TODO: REMOVE THE FOLLOWING 3 DEPRECATED FUNCTIONS IF THIS SMART BECOMES OFFICIAL.. **/
extern "C" bool exp_smartEnabled();
extern "C" int exp_getRefresh();
extern "C" void exp_setRefresh(int value);


#endif // SMARTREMOTE_HPP_INCLUDED
