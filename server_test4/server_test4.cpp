#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include <winsock2.h>
#include <windows.h>
#include <cstdio>    // �߰�: sprintf_s�� ����ϱ� ���� ��� ����
#include "resource.h"
#include <stdio.h>
#include <process.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024
#define SERVER_PORT 9000

typedef struct sock_info
{
    SOCKET s;
    HANDLE ev;
    char nick[50];
    char ipaddr[50];
}SOCK_INFO;

int port_number = SERVER_PORT;
const int client_count = 10;
SOCK_INFO sock_array[client_count + 1];
int total_socket_count = 0;
SOCKET serverSocket;

HWND hChatHistoryEdit, hIpAddressStatic, hMessageEdit, hNickNameEdit;  // �߰�: ���� IP �ּҸ� ǥ���� ���� �ؽ�Ʈ ��Ʈ�� �ڵ�


void AddMessageToChatHistory(const char* message) {
    SendMessage(hChatHistoryEdit, EM_SETSEL, -1, -1);
    SendMessage(hChatHistoryEdit, EM_REPLACESEL, 0, (LPARAM)message);
    SendMessage(hChatHistoryEdit, EM_SCROLLCARET, 0, 0);
}

DWORD WINAPI ClientHandler(LPVOID lpParam);
DWORD WINAPI ClientAcceptThread(LPVOID lpParam);
LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ClientHandler(LPVOID lpParam);

LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        hChatHistoryEdit = GetDlgItem(hwndDlg, IDC_CHAT_HISTORY_EDIT);
        hIpAddressStatic = GetDlgItem(hwndDlg, IDC_IPADDRESS_STATIC);

        memset(&sock_array, 0, sizeof(sock_array));
        total_socket_count = 0;
        // ���� �ʱ�ȭ
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            MessageBox(hwndDlg, L"Failed to initialize Winsock.", L"Error", MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, 0);
        }

        // ���� ���� ����
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            MessageBox(hwndDlg, L"Failed to create server socket.", L"Error", MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, 0);
        }

        SOCKADDR_IN serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
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

        // ���� IP �ּҸ� ǥ���� ���� �ؽ�Ʈ ��Ʈ�ѿ� IP �ּҸ� ����
        char ipAddress[15];  // IPv4 �ּ��� �ִ� ���̴� 15�ڸ�
        if (gethostname(ipAddress, sizeof(ipAddress)) == 0) {
            SetWindowTextA(hIpAddressStatic, ipAddress);
        }

        MessageBox(hwndDlg, L"Server is listening for incoming connections.", L"Info", MB_OK | MB_ICONINFORMATION);

        // Ŭ���̾�Ʈ�� ó���ϱ� ���� ������ ����
        DWORD threadId;
        HANDLE hThread;
        hThread = CreateThread(NULL, 0, ClientAcceptThread, (LPVOID)serverSocket, 0, &threadId);
        if (hThread == NULL) {
            MessageBox(hwndDlg, L"Failed to create client accept thread.", L"Error", MB_OK | MB_ICONERROR);
            closesocket(serverSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
        }
        else {
            CloseHandle(hThread);
        }

        return TRUE;

    case IDC_SEND_BUTTON:
        // �޽��� ���� ��ư ���� �߰�
    {
        wchar_t message[256];
        GetWindowText(hMessageEdit, message, sizeof(message) / sizeof(message[0]));

        char buffer[MAX_BUFFER_SIZE];
        sprintf_s(buffer, sizeof(buffer), "%ls\r\n", message);

        //send((SOCKET)lpParam, buffer, strlen(buffer), 0);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_EXIT_BUTTON:
            // ���� ���� ��ư ���� �߰�
            //closesocket(clientSocket);
            closesocket(serverSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        // ���� ���� ó�� �߰�
        //closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        EndDialog(hwndDlg, 0);
        break;

        // �߰����� �̺�Ʈ ó��

    default:
        return FALSE;
    }
    return TRUE;
}

// Ŭ���̾�Ʈ ������ �޴� ������ �Լ�
DWORD WINAPI ClientAcceptThread(LPVOID lpParam) {
    SOCKET serverSocket = (SOCKET)lpParam;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            MessageBox(NULL, L"Failed to accept client socket.", L"Error", MB_OK | MB_ICONERROR);
            break;
        }

        // Ŭ���̾�Ʈ�� ó���ϱ� ���� ������ ����
        DWORD threadId;
        HANDLE hThread = CreateThread(NULL, 0, ClientHandler, (LPVOID)clientSocket, 0, &threadId);
        if (hThread == NULL) {
            MessageBox(NULL, L"Failed to create client thread.", L"Error", MB_OK | MB_ICONERROR);
            closesocket(clientSocket);
            break;
        }
        else {
            // �����带 �����ϱ� ���� �ڵ��� ������ �Ӵϴ�.
            // �ʿ信 ���� ������ �ڵ��� �����ϴ� ��Ŀ������ ������ �� �ֽ��ϴ�.
            CloseHandle(hThread);
        }
    }

    return 0;
}

DWORD WINAPI ClientHandler(LPVOID lpParam) {
    SOCKET clientSocket = (SOCKET)lpParam;

    // Ŭ���̾�Ʈ �ڵ鸵 �� �޽��� ���� �� ó��
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
    // ��ȭ���� ����
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CHAT_DIALOG), NULL, DialogProc);

    return 0;
}