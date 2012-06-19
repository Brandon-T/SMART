/**
 *  Copyright 2012 by Benjamin J. Land (a.k.a. BenLand100)
 *
 *  This file is part of the SMART-Remote
 *
 *  SMART-Remote is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SMART-Remote is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SMART-Remote. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Remote.h"
#include "Bridge.h"
#include <iostream>
#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifndef _WIN32
#include <sched.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#endif

using namespace std;

static char *path;
static char *root,*params,*initseq,*useragent,*jvmpath,*maxmem;
static int width,height;
static void *img,*dbg;
static void *memmap;
static int server_socket,client_socket;
static void **functions;

static char shmfile[256];
#ifndef _WIN32
static int fd;
#else

#endif

shm_data *data;

/**
 * Loads the SMART library, links the required methods, and starts SMART according
 * to the current arguments
 */
void initSMART() {
    char lib[256];
    #ifndef _WIN32
        #if __SIZEOF_POINTER__ == 4
            sprintf(lib,"%s/libsmart32.so",path);
            void* libsmart = dlopen(lib,RTLD_LAZY);
        #else
            sprintf(lib,"%s/libsmart64.so",path);
            void* libsmart = dlopen(lib,RTLD_LAZY);
        #endif
    #else
        #if __SIZEOF_POINTER__ == 4
            sprintf(lib,"%s/libsmart32.dll",path);
            HMODULE libsmart = LoadLibrary(lib);
        #else
            sprintf(lib,"%s/libsmart64.dll",path);
            HMODULE libsmart = LoadLibrary(lib);
        #endif
    #endif
    cout << "Library: " << libsmart << '\n';
    if (!libsmart) exit(1); //ABORT EVERYTHING THE LIBRARY WASN'T FOUND
    functions = (void**)malloc(sizeof(void*)*NumImports);
    SetupRemote setup;
    SetUserAgent setAgent;
    SetJVMPath setPath;
    SetMaxJVMMem setMem;
    bool failed = false;
    #ifndef _WIN32
        setup = (SetupRemote)dlsym(libsmart, "setupRemote");
        setAgent = (SetUserAgent)dlsym(libsmart, "setUserAgent");
        setPath = (SetJVMPath)dlsym(libsmart, "setJVMPath");
        setMem = (SetMaxJVMMem)dlsym(libsmart, "setMaxJVMMem");
        for (int i = 0; i < NumImports; i++) {
            functions[i] = (void*)dlsym(libsmart, imports[i]);
            if (!functions[i]) {
                cout << "Failed to import " << imports[i] << '\n';
                failed = true;
            }
        }
    #else
        setup = (SetupRemote)GetProcAddress(libsmart, "setupRemote");
        setAgent = (SetUserAgent)GetProcAddress(libsmart, "setUserAgent");
        setPath = (SetJVMPath)GetProcAddress(libsmart, "setJVMPath");
        setMem = (SetMaxJVMMem)GetProcAddress(libsmart, "setMaxJVMMem");
        for (int i = 0; i < NumImports; i++) {
            functions[i] = (void*)GetProcAddress(libsmart, imports[i]);
            if (!functions[i]) {
                cout << "Failed to import " << imports[i] << '\n';
                failed = true;
            }
        }
    #endif
    if (failed || !setup || !setAgent || !setPath || !setMem) {
        cout << "SMART methods failed to import, aborting.\n";
        exit(1);
    }
    if (useragent) setAgent(useragent);
    if (jvmpath) setPath(jvmpath);
    if (maxmem) setMem(atoi(maxmem));
    setup(root,params,width,height,img,dbg,initseq,data->id);
}

/**
 * Executes functions passed from the paired process
 */
void execfun(int funid) {
    switch (funid) {
        case getRefresh:
            *(int*)(data->args) = ((type_getRefresh)(functions[getRefresh-FirstFunc]))();
            break;
        case setRefresh:
            ((type_setRefresh)(functions[setRefresh-FirstFunc]))(*(int*)(data->args));
            break;
        case setTransparentColor:
            ((type_setTransparentColor)(functions[setTransparentColor-FirstFunc]))(*(int*)(data->args));
            break;
        case setDebug:
            ((type_setDebug)(functions[setDebug-FirstFunc]))(*(bool*)(data->args));
            break;
        case setGraphics:
            ((type_setGraphics)(functions[setGraphics-FirstFunc]))(*(bool*)(data->args));
            break;
        case setEnabled:
            ((type_setEnabled)(functions[setEnabled-FirstFunc]))(*(bool*)(data->args));
            break;
        case isActive:
            *(bool*)(data->args) = ((type_isActive)(functions[isActive-FirstFunc]))();
            break;
        case isBlocking:
            *(bool*)(data->args) = ((type_isBlocking)(functions[isBlocking-FirstFunc]))();
            break;
        case getMousePos:
            ((type_getMousePos)(functions[getMousePos-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1]);
            break;
        case holdMouse:
            ((type_holdMouse)(functions[holdMouse-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],(bool)(((int*)(data->args))[2-FirstFunc]));
            break;
        case releaseMouse:
            ((type_releaseMouse)(functions[releaseMouse-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],(bool)(((int*)(data->args))[2-FirstFunc]));
            break;
        case holdMousePlus:
            ((type_holdMousePlus)(functions[holdMousePlus-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],((int*)(data->args))[2]);
            break;
        case releaseMousePlus:
            ((type_releaseMousePlus)(functions[releaseMousePlus-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],((int*)(data->args))[2]);
            break;
        case moveMouse:
            ((type_moveMouse)(functions[moveMouse-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1]);
            break;
        case windMouse:
            ((type_windMouse)(functions[windMouse-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1]);
            break;
        case clickMouse:
            ((type_clickMouse)(functions[clickMouse-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],(bool)(((int*)(data->args))[2-FirstFunc]));
            break;
        case clickMousePlus:
            ((type_clickMousePlus)(functions[clickMousePlus-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],((int*)(data->args))[2]);
            break;
        case isMouseButtonHeld:
            *(bool*)(data->args) = ((type_isMouseButtonHeld)(functions[isMouseButtonHeld-FirstFunc]))(*(int*)(data->args));
            break;
        case sendKeys:
            ((type_sendKeys)(functions[sendKeys-FirstFunc]))((char*)data->args+2*sizeof(int),((int*)data->args)[0],((int*)data->args)[0]);
            break;
        case holdKey:
            ((type_holdKey)(functions[holdKey-FirstFunc]))(*(int*)data->args);
            break;
        case releaseKey:
            ((type_releaseKey)(functions[releaseKey-FirstFunc]))(*(int*)data->args);
            break;
        case isKeyDown:
            *(bool*)(data->args) = ((type_isKeyDown)(functions[isKeyDown-FirstFunc]))(*(int*)data->args);
            break;
        case getColor:
            *(int*)(data->args) = ((type_getColor)(functions[getColor-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1]);
            break;
        case findColor:
            ((int*)(data->args))[2] = ((type_findColor)(functions[findColor-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],((int*)(data->args))[2],((int*)(data->args))[3],((int*)(data->args))[4],((int*)(data->args))[5],((int*)(data->args))[6]);
            //x&y autoupdated
            break;
        case findColorTol:
            ((int*)(data->args))[2] = ((type_findColorTol)(functions[findColorTol-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],((int*)(data->args))[2],((int*)(data->args))[3],((int*)(data->args))[4],((int*)(data->args))[5],((int*)(data->args))[6],((int*)(data->args))[7]);
            //x&y autoupdated
            break;
        case findColorSpiral:
            ((int*)(data->args))[2] = ((type_findColorSpiral)(functions[findColorSpiral-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],((int*)(data->args))[2],((int*)(data->args))[3],((int*)(data->args))[4],((int*)(data->args))[5],((int*)(data->args))[6]);
            //x&y autoupdated
            break;
        case findColorSpiralTol:
            ((int*)(data->args))[2] = ((type_findColorSpiralTol)(functions[findColorSpiralTol-FirstFunc]))(((int*)(data->args))[0],((int*)(data->args))[1],((int*)(data->args))[2],((int*)(data->args))[3],((int*)(data->args))[4],((int*)(data->args))[5],((int*)(data->args))[6],((int*)(data->args))[7]);
            //x&y autoupdated
            break;
        case Ping:
            //do nothing
            break;
        default:
            cout << "Invalid function id: " << funid << '\n';
    }
}

int init_socks(int &port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        return 0; //failed
    }
    port = 4200;
    while (port < 4300) { //No one will open 100 SMARTs on one machine... right?
        cout << "Trying port " << port << '\n';
        struct sockaddr_in local;
        memset((char *) &local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = INADDR_ANY;
        local.sin_port = htons(port);
        if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
            port++;
            continue;
        } else {
            break;
        }
    }
    if (port == 4300) {
        port = 0;
        #ifndef _WIN32
            close(server_socket);
        #else
            closesocket(server_socket);
        #endif
        cout << "Could not bind server socket\n";
        return 0; //failed
    }
    if (listen(server_socket,5)!=0) {
        cout << "Could not listen on server socket\n";
        return 0;
    }
    cout << "Remote socket listening\n";
    return server_socket;
}

int poll_conn(int server_socket) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(server_socket, &rfds);
    if (select(server_socket+1, &rfds, &rfds, NULL, &tv)) {
        cout << "Client socket connected: ";
        int client_socket = accept(server_socket,NULL,NULL);
        cout << client_socket << '\n';
        if (client_socket > 0) return client_socket;
    }
    return 0;
}

void clean_shm() {
    #ifndef _WIN32
    //Free memory and delete SMART.[pid] on terminate
    munmap(memmap,width*height*2*4+sizeof(shm_data));
    close(fd);
    unlink(shmfile);
    #else
    UnmapViewOfFile(data);
    CloseHandle(memmap);
    DeleteFile(shmfile);
    #endif
}

int main(int argc, char** argv) {
    //Read init args from arguments
    if (argc != 10) exit(0);
    
    #ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
    #endif
    
    path = argv[1];
    if (strlen(path)==0) path = (char*)".";    
    root = argv[2];
    params = argv[3];
    width = atoi(argv[4]);
    height = atoi(argv[5]);
    initseq = argv[6];
    
    useragent = argv[7]; 
    if (strlen(useragent)<=0) useragent = 0;
    jvmpath = argv[8];
    if (strlen(jvmpath)<=0) jvmpath = 0;
    maxmem = argv[9];
    if (strlen(maxmem)<=0) maxmem = 0;
   
    //Create the shared memory file
    #ifndef _WIN32
    sprintf(shmfile,"SMART.%i",getpid());
    fd = open(shmfile,O_CREAT|O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO); 
    lseek(fd,width*height*2*4+sizeof(shm_data),0);
    write(fd,"\0",1); //ensure proper size
    fsync(fd); //flush to disk
    memmap = mmap(NULL, width*height*4*2+sizeof(shm_data), PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
    data = (shm_data*)memmap;
    #else
    sprintf(shmfile,"SMART.%i",(int)GetCurrentProcessId());
    HANDLE file = CreateFile(
        shmfile,
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_RANDOM_ACCESS|FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_OVERLAPPED,
        NULL);
    SetFilePointer(file,sizeof(shm_data)+2*width*height*4,0,FILE_BEGIN);
    SetEndOfFile(file);
    memmap = CreateFileMapping(file,NULL,PAGE_READWRITE,0,sizeof(shm_data)+4*2*width*height,shmfile);
    data = (shm_data*) MapViewOfFile(memmap,FILE_MAP_ALL_ACCESS,0,0,sizeof(shm_data)+4*2*width*height);
    #endif
    cout << "Shared Memory mapped to " << memmap << "\n";
    
    memset(data,0x00,sizeof(shm_data)+4*2*width*height);
    
    //Init the shm_data structure
    #ifndef _WIN32
    data->id = getpid();
    #else
    data->id = GetCurrentProcessId();
    #endif
    data->port = 0;
    data->paired = 0;
    data->width = width;
    data->height = height;
    data->time = 0;
    data->die = 0;
    data->imgoff = sizeof(shm_data);
    data->dbgoff = sizeof(shm_data)+4*width*height;
    int port;
    server_socket = init_socks(port);
    data->port = port;
    client_socket = 0;
    if (!server_socket) {
        cout << "Error starting sockets\n";
        clean_shm();
        exit(1);
    }
    #ifndef _WIN32
    fsync(fd);
    #else
    FlushFileBuffers(file);
    #endif

    //Let SMART use the shared memory for the images
    img = (((char*)data) + data->imgoff);
    dbg = (((char*)data) + data->dbgoff);
    

    unsigned int i = 0;
    while (!(client_socket = poll_conn(server_socket))) {
        if (i++ > 25) {
            cout << "Failed initial socket pairing\n";
            clean_shm();
            exit(1);
        }
    }
    
    //Load the smart plugin and link functions
    initSMART();

    #ifdef _WIN32
    HANDLE paired = NULL;
    #endif
    
    //Event loop: updates time, checks for function calls, and unpairs if the paired thread dies
    //Terminates when the SMART client closes OR if we recieve a die flag from a paired thread
    //Might try making this sleep for a second, then update time/check pairing, and allow to be woken 
    //sleep by a signal
    for (unsigned int i = 0; !data->die && ((type_isActive)functions[isActive-FirstFunc])(); i++) {
        data->time = time(0);
        if (client_socket) {
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(client_socket, &rfds);
            for (int i = 0; i < 25 && select(client_socket+1, &rfds, NULL, NULL, &tv); i++) {
                int funid;
                if (recv(client_socket,(char*)&funid,sizeof(int),0) != sizeof(int)) {
                    #ifndef _WIN32
                        close(client_socket);
                    #else
                        closesocket(client_socket);
                    #endif
                    client_socket = 0;
                } else {
                    execfun(funid);
                    if (send(client_socket,(const char*)&funid,sizeof(int),0) != sizeof(int)) {
                        #ifndef _WIN32
                            close(client_socket);
                        #else
                            closesocket(client_socket);
                        #endif
                        client_socket = 0;
                    }
                }
            }
        } else {
            client_socket = poll_conn(server_socket);
        }
        #ifndef _WIN32
        if (data->paired && syscall(SYS_tkill,data->paired,0)) {
        #else
        if (!paired && data->paired) {
            paired = OpenThread(SYNCHRONIZE,FALSE,data->paired);
            if (!paired) {
                cout << "Paired thread no longer exists: reset\n";
                data->paired = 0;
            }
        }
        if (paired && !data->paired) {
            cout << "Controller unpaired cleanly\n";
            CloseHandle(paired);
            paired = NULL;
        }
        if (paired && (WaitForSingleObject(paired, 0)!= WAIT_TIMEOUT)) {
            CloseHandle(paired);
            paired = NULL;
        #endif
            cout << "Paired thread terminated: reset\n";
            data->paired = 0;
            #ifndef _WIN32
                close(client_socket);
            #else
                closesocket(client_socket);
            #endif
            client_socket = 0;
            #ifndef _WIN32
            fsync(fd);
            #else
            FlushFileBuffers(file);
            #endif
        }
    }
    
    #ifdef _WIN32
    CloseHandle(file);
    if (paired) CloseHandle(paired);
    #endif
    
    clean_shm();
    #ifndef _WIN32
        close(server_socket);
    #else
        closesocket(server_socket);
    #endif
    if (client_socket)
        #ifndef _WIN32
            close(client_socket);
        #else
            closesocket(client_socket);
        #endif
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    exit(1); //Make sure the JVM exits
}
