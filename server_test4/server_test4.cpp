#include <winsock2.h>
#include <windows.h>
#include <cstdio>    // 추가: sprintf_s를 사용하기 위한 헤더 파일
#include "resource.h"

#define MAX_BUFFER_SIZE 1024
#define SERVER_PORT 12345

HWND hChatHistoryEdit, hIpAddressStatic, hMessageEdit, hNickNameEdit;  // 추가: 서버 IP 주소를 표시할 정적 텍스트 컨트롤 핸들
SOCKET serverSocket, clientSocket;
WSADATA wsaData;

void AddMessageToChatHistory(const char* message) {
    SendMessage(hChatHistoryEdit, EM_SETSEL, -1, -1);
    SendMessage(hChatHistoryEdit, EM_REPLACESEL, 0, (LPARAM)message);
    SendMessage(hChatHistoryEdit, EM_SCROLLCARET, 0, 0);
}

DWORD WINAPI ClientHandler(LPVOID lpParam);

LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        hChatHistoryEdit = GetDlgItem(hwndDlg, IDC_CHAT_HISTORY_EDIT);
        hIpAddressStatic = GetDlgItem(hwndDlg, IDC_IPADDRESS_STATIC);

        // 윈속 초기화
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            MessageBox(hwndDlg, L"Failed to initialize Winsock.", L"Error", MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, 0);
        }

        // 서버 소켓 생성
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            MessageBox(hwndDlg, L"Failed to create server socket.", L"Error", MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, 0);
        }

        SOCKADDR_IN serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_PORT);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            MessageBox(hwndDlg, L"Failed to bind server socket.", L"Error", MB_OK | MB_ICONERROR);
            closesocket(serverSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            MessageBox(hwndDlg, L"Error listening on socket.", L"Error", MB_OK | MB_ICONERROR);
            closesocket(serverSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
        }

        // 서버 IP 주소를 표시할 정적 텍스트 컨트롤에 IP 주소를 설정
        char ipAddress[15];  // IPv4 주소의 최대 길이는 15자리
        if (gethostname(ipAddress, sizeof(ipAddress)) == 0) {
            SetWindowTextA(hIpAddressStatic, ipAddress);
        }

        MessageBox(hwndDlg, L"Server is listening for incoming connections.", L"Info", MB_OK | MB_ICONINFORMATION);

        return TRUE;

    case IDC_SETNICK_BUTTON:
        // 닉네임 설정 버튼 동작 추가
    {
        wchar_t nickName[256];
        GetWindowText(hNickNameEdit, nickName, sizeof(nickName) / sizeof(nickName[0]));

        char buffer[MAX_BUFFER_SIZE];
        sprintf_s(buffer, sizeof(buffer), "[%ls] joined the chat.\r\n", nickName);

        send(clientSocket, buffer, strlen(buffer), 0);
    }
    break;

    case IDC_SEND_BUTTON:
        // 메시지 전송 버튼 동작 추가
    {
        wchar_t message[256];
        GetWindowText(hMessageEdit, message, sizeof(message) / sizeof(message[0]));

        wchar_t nickName[256];
        GetWindowText(hNickNameEdit, nickName, sizeof(nickName) / sizeof(nickName[0]));

        char buffer[MAX_BUFFER_SIZE];
        sprintf_s(buffer, sizeof(buffer), "[%ls] %ls\r\n", nickName, message);

        send(clientSocket, buffer, strlen(buffer), 0);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_EXIT_BUTTON:
            // 서버 종료 버튼 동작 추가
            closesocket(clientSocket);
            closesocket(serverSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        // 서버 종료 처리 추가
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        EndDialog(hwndDlg, 0);
        break;

        // 추가적인 이벤트 처리

    default:
        return FALSE;
    }
    return TRUE;
}

DWORD WINAPI ClientHandler(LPVOID lpParam) {
    SOCKET clientSocket = (SOCKET)lpParam;

    // 클라이언트 핸들링 및 메시지 수신 및 처리
    char buffer[MAX_BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        AddMessageToChatHistory(buffer);
    }

    closesocket(clientSocket);

    return 0;
}

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
) {
    // 대화상자 생성
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CHAT_DIALOG), NULL, DialogProc);

    return 0;
}