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

    // Create client socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET) {

        cout << "Socket creation failed\n";

        WSACleanup();
        return 1;
    }

    cout << "Client socket created\n";

    // Server address
    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)) == SOCKET_ERROR) {

        cout << "Connection failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    cout << "Connected to server!\n";

    // Data to send
    const char* message =
        "\\documentclass{article}\n"
        "\\begin{document}\n"
        "Hello TCP Lab 2\n"
        "\\end{document}";

    // Send data
    send(
        clientSocket,
        message,
        strlen(message),
        0
    );

    cout << "\nData sent successfully\n";

    // Close socket
    closesocket(clientSocket);

    WSACleanup();

    cout << "Client closed\n";

    return 0;
}