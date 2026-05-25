#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCKAPI_

#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define IDC_START_BUTTON 1
#define IDC_SEND_BUTTON  2
#define IDC_LOG_BOX      3

#define WM_SOCKET (WM_USER + 1)

HWND hLogBox;

SOCKET serverSocket;
SOCKET clientSocket;

bool serverStarted = false;

// ADD LOG TO GUI

void AddLog(const char* text) {

    int length = GetWindowTextLengthA(hLogBox);

    SendMessageA(
        hLogBox,
        EM_SETSEL,
        length,
        length
    );

    SendMessageA(
        hLogBox,
        EM_REPLACESEL,
        FALSE,
        (LPARAM)text
    );

    SendMessageA(
        hLogBox,
        EM_REPLACESEL,
        FALSE,
        (LPARAM)"\r\n"
    );
}

// START SERVER

void StartServer(HWND hwnd) {

    if (serverStarted) {

        AddLog("[INFO] Server already started");

        return;
    }

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {

        AddLog("[ERROR] WSAStartup failed");

        return;
    }

    AddLog("[INFO] WinSock initialized");

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {

        AddLog("[ERROR] Socket creation failed");

        return;
    }

    AddLog("[INFO] Server socket created");

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(
        serverSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    ) == SOCKET_ERROR) {

        AddLog("[ERROR] Bind failed");

        return;
    }

    AddLog("[INFO] Bind successful");

    listen(serverSocket, SOMAXCONN);

    AddLog("[INFO] Server listening on port 644");

    // WSAAsyncSelect
    WSAAsyncSelect(
        serverSocket,
        hwnd,
        WM_SOCKET,
        FD_ACCEPT | FD_READ | FD_CLOSE
    );

    serverStarted = true;
}

// SEND TEST FILE

void SendTestFile() {

    SOCKET sendSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sendSocket == INVALID_SOCKET) {

        AddLog("[ERROR] Client socket failed");

        return;
    }

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(
        sendSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    ) == SOCKET_ERROR) {

        AddLog("[ERROR] Connect failed");

        closesocket(sendSocket);

        return;
    }

    AddLog("[INFO] Connected to server");

    std::ifstream file("test.tex", std::ios::binary);

    if (!file.is_open()) {

        AddLog("[ERROR] Cannot open test.tex");

        closesocket(sendSocket);

        return;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    file.close();

    send(
        sendSocket,
        content.c_str(),
        content.size(),
        0
    );

    AddLog("[INFO] File sent successfully");

    closesocket(sendSocket);
}

// WINDOW PROCEDURE

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
) {

    switch (uMsg) {

    case WM_CREATE:

        CreateWindowA(
            "BUTTON",
            "Start Server",
            WS_VISIBLE | WS_CHILD,
            20,
            20,
            150,
            40,
            hwnd,
            (HMENU)IDC_START_BUTTON,
            NULL,
            NULL
        );

        CreateWindowA(
            "BUTTON",
            "Send Test File",
            WS_VISIBLE | WS_CHILD,
            200,
            20,
            150,
            40,
            hwnd,
            (HMENU)IDC_SEND_BUTTON,
            NULL,
            NULL
        );

        hLogBox = CreateWindowA(
            "EDIT",
            "",
            WS_VISIBLE |
            WS_CHILD |
            WS_BORDER |
            ES_MULTILINE |
            ES_AUTOVSCROLL |
            WS_VSCROLL,
            20,
            80,
            540,
            300,
            hwnd,
            (HMENU)IDC_LOG_BOX,
            NULL,
            NULL
        );

        break;

        // BUTTONS

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDC_START_BUTTON:

            StartServer(hwnd);

            break;

        case IDC_SEND_BUTTON:

            SendTestFile();

            break;
        }

        break;

        // SOCKET EVENTS

    case WM_SOCKET:

        switch (WSAGETSELECTEVENT(lParam)) {

        case FD_ACCEPT:

            clientSocket = accept(
                serverSocket,
                NULL,
                NULL
            );

            AddLog("[INFO] Client connected");

            WSAAsyncSelect(
                clientSocket,
                hwnd,
                WM_SOCKET,
                FD_READ | FD_CLOSE
            );

            break;

        case FD_READ:

        {
            char buffer[1024];

            int bytesReceived = recv(
                clientSocket,
                buffer,
                sizeof(buffer),
                0
            );

            if (bytesReceived > 0) {

                std::ofstream outFile(
                    "received_async.tex",
                    std::ios::binary
                );

                outFile.write(buffer, bytesReceived);

                outFile.close();

                AddLog("[INFO] Data received");
                AddLog("[INFO] File saved as received_async.tex");

                ShellExecuteA(
                    NULL,
                    "open",
                    "notepad.exe",
                    "received_async.tex",
                    NULL,
                    SW_SHOWNORMAL
                );
            }

            break;
        }

        case FD_CLOSE:

            if (clientSocket != INVALID_SOCKET) {

                AddLog("[INFO] Client disconnected");

                closesocket(clientSocket);

                clientSocket = INVALID_SOCKET;
            }

            break;
        }

        break;

    case WM_DESTROY:

        closesocket(serverSocket);

        WSACleanup();

        PostQuitMessage(0);

        return 0;
    }

    return DefWindowProc(
        hwnd,
        uMsg,
        wParam,
        lParam
    );
}

// WINMAIN

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int nCmdShow
) {

    const wchar_t CLASS_NAME[] = L"AsyncServerWindow";

    WNDCLASSW wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Async TCP Server",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        600,
        450,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};

    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);

        DispatchMessage(&msg);
    }

    return 0;
}