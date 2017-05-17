#include "socket.h"

int initializeServerSocket(const char *port, SOCKET *serverSocket)
{
    int errorCode = 0;

    struct addrinfo hints, *addressInformation = NULL;

    WSADATA wsaData;

    if ((errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) == 0)
    {
        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;
        hints.ai_addr = INADDR_ANY;

        if ((errorCode = getaddrinfo(NULL, port, &hints, &addressInformation)) == 0)
        {
            *serverSocket = socket(addressInformation->ai_family, addressInformation->ai_socktype, addressInformation->ai_protocol);

            if (*serverSocket == INVALID_SOCKET)
            {
                errorCode = WSAGetLastError();

                freeaddrinfo(addressInformation);
                WSACleanup();
            }
            else
            {
                if ((errorCode = bind(*serverSocket, addressInformation->ai_addr, (int)addressInformation->ai_addrlen)) == 0)
                {
                    freeaddrinfo(addressInformation);

                    if ((errorCode = listen(*serverSocket, SOMAXCONN)) == SOCKET_ERROR)
                    {
                        closesocket(*serverSocket);
                        WSACleanup();
                    }
                }
                else
                {
                    freeaddrinfo(addressInformation);
                    closesocket(*serverSocket);
                    WSACleanup();
                }
            }
        }
        else
        {
            WSACleanup();
        }
    }

    return errorCode;
}

int initializeSocket(const char *ipAddress, const char *port, SOCKET *_socket)
{
    int errorCode = 0;

    struct addrinfo hints, *pointer, *addressInformation = NULL;

    WSADATA wsaData;

    if ((errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) == 0)
    {
        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_addr = INADDR_ANY;

        if ((errorCode = getaddrinfo(ipAddress, port, &hints, &addressInformation)) == 0)
        {
            for (pointer = addressInformation; pointer != NULL; pointer = pointer->ai_next)
            {
                *_socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);

                if (*_socket == INVALID_SOCKET)
                {
                    errorCode = WSAGetLastError();

                    WSACleanup();
                }
                else
                {
                    if ((errorCode = connect(*_socket, pointer->ai_addr, (int)pointer->ai_addrlen)) == SOCKET_ERROR)
                    {
                        closesocket(*_socket);

                        *_socket = INVALID_SOCKET;

                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            freeaddrinfo(addressInformation);

            if (*_socket == INVALID_SOCKET)
            {
                errorCode = -1;

                WSACleanup();
            }
        }
        else
        {
            WSACleanup();
        }
    }

    return errorCode;
}

int acceptSocket(SOCKET *socket, SOCKET *serverSocket)
{
    int errorCode = 0;

    *socket = accept(*serverSocket, NULL, NULL);

    if (*socket == INVALID_SOCKET)
    {
        errorCode = WSAGetLastError();

        WSACleanup();
    }

    return errorCode;
}

int write(char buffer[], int length, SOCKET socket)
{
    return send(socket, buffer, length, 0);
}

int read(char buffer[], int length, SOCKET socket)
{
    return recv(socket, buffer, length, 0);
}
