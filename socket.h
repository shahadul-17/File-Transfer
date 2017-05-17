#ifndef SOCKET_H
#define SOCKET_H

#define _WIN32_WINNT 0x0501

#include <winsock2.h>
#include <ws2tcpip.h>

int initializeServerSocket(const char *, SOCKET *);
int initializeSocket(const char *, const char *, SOCKET *);
int acceptSocket(SOCKET *, SOCKET *);
int write(char [], int, SOCKET);
int read(char [], int, SOCKET);

#endif
