#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Cross-platform socket includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

// Initialize sockets for Windows
static inline void init_sockets() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        exit(1);
    }
#endif
}

// Cleanup sockets for Windows
static inline void cleanup_sockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

#endif   
