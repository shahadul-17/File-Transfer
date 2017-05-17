#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "socket.h"

#define MAX_FILE_NAME_LENGTH 261
#define BUFFER_LENGTH 8192
#define PORT "51713"

static char serverResponse[2][5];

long getFileSize(FILE *file)
{
    long fileSize = -1;

    if (fseek(file, 0, SEEK_END) == 0)
    {
        fileSize = ftell(file);

        rewind(file);
    }

    return fileSize;
}

void *handleRequest(void *argument)
{
    char fileName[MAX_FILE_NAME_LENGTH], buffer[BUFFER_LENGTH];
    int bytesRead = 0;
    long bytesSent = 0, fileSize = 0;

    SOCKET socket = (SOCKET)argument;

    if ((bytesRead = read(fileName, MAX_FILE_NAME_LENGTH, socket)) > 0)
    {
        fileName[bytesRead] = '\0';

        FILE *file = fopen(fileName, "rb");     // 'r' -> read, 'b' -> binary...

        if (file)
        {
            if (write(serverResponse[0], strlen(serverResponse[0]), socket) == SOCKET_ERROR)
            {
                printf("error: could not send response...\n\n");
            }
            else
            {
                printf("retrieving file size...\n\n");

                fileSize = getFileSize(file);

                if (fileSize == -1)
                {
                    printf("error: we were unable to retrieve file size...\n\n");
                }
                else
                {
                    printf("file size is %li bytes...\n\nsending file...\n\n", fileSize);

                    while ((bytesSent = ftell(file)) < fileSize)
                    {
                        if ((bytesRead = fread(buffer, 1, BUFFER_LENGTH, file)) > 0)
                        {
                            if (write(buffer, bytesRead, socket) == SOCKET_ERROR)
                            {
                                printf("error: file sending interrupted...\n\n");

                                break;
                            }
                        }
                        else
                        {
                            printf("error: we were unable to send the file...\n\n");

                            break;
                        }
                    }

                    if (bytesSent == fileSize)
                    {
                        printf("file sent successfully...\n\n");
                    }
                }
            }

            fclose(file);
        }
        else
        {
            if (write(serverResponse[1], strlen(serverResponse[1]), socket) == SOCKET_ERROR)
            {
                printf("error: could not send response...\n\n");
            }

            printf("error: we were unable to open the file \"%s\"...\n\n", fileName);
        }
    }

    closesocket(socket);

    return NULL;
}

int main()
{
    int selection = 0;

    strcpy(serverResponse[0], "acpt");     // accept...
    strcpy(serverResponse[1], "rjct");     // reject...

    printf("run as:\n\t(1) server\n\t(2) client\n\nselection: ");

    while (selection != 1 && selection != 2)
    {
        scanf("%d", &selection);
    }

    if (selection == 1)
    {
        SOCKET serverSocket = INVALID_SOCKET;

        if (initializeServerSocket(PORT, &serverSocket) == 0)
        {
            printf("\nserver started successfully at port: %s...\n\n", PORT);

            while (1)
            {
                SOCKET socket = INVALID_SOCKET;

                printf("waiting for client...\n\n");

                if (acceptSocket(&socket, &serverSocket) == 0)
                {
                    pthread_t thread;

                    printf("client connected...\n\n");

                    if (pthread_create(&thread, NULL, handleRequest, (void*)socket) != 0)
                    {
                        printf("error: thread creation failed...\n\n");
                    }
                }
            }

            closesocket(serverSocket);
            WSACleanup();
        }
        else
        {
            printf("\nerror: unable to start server at port: %s...\n\n", PORT);
        }
    }
    else
    {
        char ipAddress[40], fileName[MAX_FILE_NAME_LENGTH], buffer[BUFFER_LENGTH];
        int bytesRead = 0;
        long bytesReceived = 0;

        SOCKET socket;

        printf("\nip address: ");
        scanf("%s", ipAddress);

        if (initializeSocket(ipAddress, PORT, &socket) == 0)
        {
            printf("\nsuccessfully connected to the server...\n\n");

            printf("enter the file name: ");
            scanf("%s", fileName);

            if (write(fileName, strlen(fileName), socket) == SOCKET_ERROR)
            {
                printf("\nerror: could not send request for file \"%s\"...\n\n", fileName);
            }
            else
            {
                if ((bytesRead = read(buffer, BUFFER_LENGTH, socket)) > 0)
                {
                    buffer[bytesRead] = '\0';

                    if (strcmp(serverResponse[0], buffer) == 0)
                    {
                        printf("\nrequest accepted by the server...\n\n");

                        FILE *file = fopen(fileName, "wb");     // 'w' -> write, 'b' -> binary...

                        if (file)
                        {
                            printf("receiving file...\n\n");

                            while ((bytesRead = read(buffer, BUFFER_LENGTH, socket)) > 0)
                            {
                                bytesReceived += bytesRead;

                                fwrite(buffer, 1, bytesRead, file);     // i don't know how to handle file write exception...
                                fflush(file);
                            }

                            printf("total number of bytes received %li...\n\n", bytesReceived);

                            if (bytesRead == 0)
                            {
                                printf("file received successfully...\n\n");
                            }
                            else
                            {
                                printf("error: file receive operation interrupted...\n\n");
                            }

                            fclose(file);
                        }
                        else
                        {
                            printf("error: we were unable to open the file...\n\n");
                        }
                    }
                    else
                    {
                        printf("\nerror: server was unable to open the file...\n\n");
                    }
                }
                else
                {
                    printf("\nerror: unable to retrieve server response...\n\n");
                }
            }
        }
        else
        {
            printf("\nerror: we were unable to connect to the server...\n\n");
        }
    }

    return 0;
}
