#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// READ FILE

vector<char> readFile(const char* filename) {

    ifstream file(filename, ios::binary);

    if (!file.is_open()) {
        cout << "Cannot open file\n";
        exit(1);
    }

    vector<char> data(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );

    file.close();

    return data;
}

// CREATE CONNECTION

SOCKET createConnection() {

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET) {

        cout << "Socket creation failed\n";

        WSACleanup();

        exit(1);
    }

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(
        clientSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    ) == SOCKET_ERROR) {

        cout << "Connection failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(clientSocket);

        WSACleanup();

        exit(1);
    }

    return clientSocket;
}

int main() {

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {

        cout << "WSAStartup failed\n";

        return 1;
    }

    cout << "WinSock initialized\n";

    vector<char> fileData = readFile("test.tex");

    cout << "File loaded successfully\n";

    // SEND WHOLE FILE

    cout << "\n====================================\n";
    cout << "Sending whole file...\n";

    SOCKET wholeSocket = createConnection();

    send(
        wholeSocket,
        fileData.data(),
        fileData.size(),
        0
    );

    cout << "Whole file sent successfully\n";

    closesocket(wholeSocket);

    // SEND 44 FRAGMENTS

    cout << "\n====================================\n";
    cout << "Sending file as 44 fragments...\n";

    SOCKET fragmentSocket = createConnection();

    int totalSize = fileData.size();

    int fragmentSize = totalSize / 44;

    int offset = 0;

    for (int i = 0; i < 44; i++) {

        int currentSize;

        if (i == 43)
            currentSize = totalSize - offset;
        else
            currentSize = fragmentSize;

        send(
            fragmentSocket,
            &fileData[offset],
            currentSize,
            0
        );

        cout << "Fragment "
            << i + 1
            << " sent. Size: "
            << currentSize
            << endl;

        offset += currentSize;
    }

    cout << "\n44 fragments sent successfully\n";

    closesocket(fragmentSocket);

    WSACleanup();

    cout << "\nClient closed\n";

    return 0;
}