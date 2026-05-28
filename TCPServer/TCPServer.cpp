#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// RECEIVE FILE

void receiveFile(
    SOCKET clientSocket,
    const char* outputFile
) {

    ofstream outFile(outputFile, ios::binary);

    char buffer[1024];

    int bytesReceived;

    while ((bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer),
        0)) > 0) {

        outFile.write(buffer, bytesReceived);

        cout << "Received block size: "
            << bytesReceived
            << endl;
    }

    outFile.close();

    cout << "\nFile saved as: "
        << outputFile
        << endl;

    // Open file in Notepad
    ShellExecuteA(
        NULL,
        "open",
        "notepad.exe",
        outputFile,
        NULL,
        SW_SHOWNORMAL
    );
}

int main() {

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {

        cout << "WSAStartup failed\n";

        return 1;
    }

    cout << "WinSock initialized\n";

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {

        cout << "Socket creation failed\n";

        WSACleanup();

        return 1;
    }

    cout << "Server socket created\n";

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(
        serverSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    ) == SOCKET_ERROR) {

        cout << "Bind failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    cout << "Bind successful\n";

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {

        cout << "Listen failed\n";

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    cout << "Server listening on port 644...\n";


    // WHOLE FILE


    cout << "\n====================================\n";
    cout << "Waiting for whole file...\n";

    sockaddr_in clientAddr1;

    int clientSize1 = sizeof(clientAddr1);

    SOCKET clientSocket1 = accept(
        serverSocket,
        (sockaddr*)&clientAddr1,
        &clientSize1
    );

    cout << "Client connected for whole file\n";

    receiveFile(
        clientSocket1,
        "received_whole.tex"
    );

    closesocket(clientSocket1);


    // 44 FRAGMENTS


    cout << "\n====================================\n";
    cout << "Waiting for fragmented file...\n";

    sockaddr_in clientAddr2;

    int clientSize2 = sizeof(clientAddr2);

    SOCKET clientSocket2 = accept(
        serverSocket,
        (sockaddr*)&clientAddr2,
        &clientSize2
    );

    cout << "Client connected for fragmented file\n";

    receiveFile(
        clientSocket2,
        "received_fragments.tex"
    );

    closesocket(clientSocket2);



    closesocket(serverSocket);

    WSACleanup();

    cout << "\nServer closed\n";

    return 0;
}