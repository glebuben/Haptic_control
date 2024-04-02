#include "server.h"
#include <iostream>
#include <chrono>
extern bool terminateThread;
extern char recvbuf[1024];
extern int rcvd;
extern int iSendResult;
extern SOCKET ListenSocket;
extern SOCKET ClientSocket;
struct msg
{
    std::int64_t milliseconds;
    int command;
    double x;
    double y;
    double z;

};

extern msg mymsg;
extern std::mutex mtx;

int server()
{

    while (!terminateThread)
    {
        WSADATA wsaData;

        struct addrinfo *result = NULL;
        struct addrinfo hints;

        int iResult;
        int recvbuflen = 1024;

        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0)
        {
            printf("WSAStartup failed with error: %d\n", iResult);
            return 1;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        iResult = getaddrinfo(NULL, "65000", &hints, &result);
        if (iResult != 0)
        {
            printf("getaddrinfo failed with error: %d\n", iResult);
            WSACleanup();
            return 1;
        }

        // Create a SOCKET for connecting to server
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        // Setup the TCP listening socket
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            printf("bind failed with error: %d\n", WSAGetLastError());
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        freeaddrinfo(result);

        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR)
        {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // No longer need server socket
        closesocket(ListenSocket);

        // Receive until the peer shuts down the connection
        do
        {
            // mtx.lock(); // used to prevent access from other threads while changing its value
            rcvd = recv(ClientSocket, (char *)&mymsg, sizeof(msg), 0);
            std::cout << "command " << mymsg.milliseconds << std::endl;
            std::cout << "command " << mymsg.command << std::endl;
            std::cout << "x " << mymsg.x << std::endl;
            std::cout << "y " << mymsg.y << std::endl;
            std::cout << "z " << mymsg.z << std::endl;

            // mtx.unlock();
            if (rcvd > 0)
            {
                printf("Bytes received: %d\n", rcvd);
                iSendResult = send(ClientSocket, (char *)&mymsg, sizeof(msg), 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                }
                printf("Bytes sent: %d\n", iSendResult);
                // mtx.unlock();
            }
            else if (rcvd == 0)
                printf("Connection closing...\n");
            else
            {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        } while (rcvd > 0);

        // shutdown the connection since we're done
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        // cleanup
        closesocket(ClientSocket);
        WSACleanup();
    }

    return 1;
}