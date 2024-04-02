#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

struct msg {
    std::int64_t milliseconds;
    int command;
    double x;
    double y;
    double z;
    double fa1;
    double fa2;
    double fa3;
    double fa4;
    double fa5;
    double fa6;
    double fa7;
    double fx;
    double fy;
    double fz;
};

int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
            *ptr = NULL,
            hints;
    const char *sendbuf = "this is a test";
    char recvbuf[1024];
    int iResult;
    int recvbuflen = 1024;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed with error: " << iResult << std::endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, "65000", &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
        WSACleanup();
        return 1;
    }

    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr=result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                           ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Connect to server.
    iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Unable to connect to server!" << std::endl;
        WSACleanup();
        return 1;
    }
    msg mymsg = {0,65537, 21.0, 3.0, 4.0, 2,0,0,0,0};

    for(int i = 0; i < 20; i++) {
        auto now = std::chrono::system_clock::now();
        // Преобразуем текущее время в количество миллисекунд с эпохи
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        // Получаем количество миллисекунд как int64_t
        std::int64_t milliseconds = duration.count();
        // Send a msg structure
        iResult = send(ConnectSocket, (char *) &mymsg, sizeof(mymsg), 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Bytes Sent: " << iResult << std::endl;

        // Receive until the peer closes the connection
        do {

            iResult = recv(ConnectSocket, (char *)&mymsg, sizeof(msg), 0);
            if (iResult > 0) {
                std::cout << "Bytes received: " << iResult << std::endl;
                std::cout << "fa1 " << mymsg.fa1 << std::endl;
                mymsg.fa1 = mymsg.fa1+1;

            }else if (iResult == 0)
                std::cout << "Connection closed\n";
            else
                std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
            iResult = 0;
        } while (iResult > 0);
    }


    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
