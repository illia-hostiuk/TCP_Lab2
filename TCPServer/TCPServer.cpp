#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {

    WSADATA wsaData;

    // WinSock initialization
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed\n";
        return 1;
    }

    cout << "WinSock initialized\n";

    // Create TCP socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    cout << "Server socket created\n";

    // Server address
    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(serverSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)) == SOCKET_ERROR) {

        cout << "Bind failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Bind successful\n";

    // Listen
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {

        cout << "Listen failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server listening on port 644...\n";

    // Accept client
    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(
        serverSocket,
        (sockaddr*)&clientAddr,
        &clientSize
    );

    if (clientSocket == INVALID_SOCKET) {

        cout << "Accept failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Client connected!\n";

    // Receive data
    char buffer[1024];

    int bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer),
        0
    );

    if (bytesReceived > 0) {

        buffer[bytesReceived] = '\0';

        cout << "\nReceived data:\n";
        cout << buffer << endl;
    }

    // Close sockets
    closesocket(clientSocket);
    closesocket(serverSocket);

    WSACleanup();

    cout << "\nServer closed\n";

    return 0;
}