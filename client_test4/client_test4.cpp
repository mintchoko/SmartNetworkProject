#include <winsock2.h>
#include <windows.h>
#include <cstdio>    // �߰�: sprintf_s�� ����ϱ� ���� ��� ����
#include <ws2tcpip.h> // �߰�: InetPton�� ����ϱ� ���� ��� ����
#include "resource.h"

#define MAX_BUFFER_SIZE 1024
#define SERVER_PORT 12345

HWND hNickNameEdit, hChatHistoryEdit, hMessageEdit;
SOCKET clientSocket;
WSADATA wsaData;

void AddMessageToChatHistory(const char* message) {
    SendMessage(hChatHistoryEdit, EM_SETSEL, -1, -1);
    SendMessage(hChatHistoryEdit, EM_REPLACESEL, 0, (LPARAM)message);
    SendMessage(hChatHistoryEdit, EM_SCROLLCARET, 0, 0);
}

LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        hNickNameEdit = GetDlgItem(hwndDlg, IDC_NICKNAME_EDIT);
        hChatHistoryEdit = GetDlgItem(hwndDlg, IDC_CHAT_HISTORY_EDIT);
        hMessageEdit = GetDlgItem(hwndDlg, IDC_MESSAGE_EDIT);

        // ���� �ʱ�ȭ
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            MessageBox(hwndDlg, L"Failed to initialize Winsock.", L"Error", MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, 0);
        }

        // Ŭ���̾�Ʈ ���� ����
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            MessageBox(hwndDlg, L"Failed to create client socket.", L"Error", MB_OK | MB_ICONERROR);
            WSACleanup();
            EndDialog(hwndDlg, 0);
        }

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CONNECT_BUTTON:
            // ���� ��ư ���� �߰�
        {
            SOCKADDR_IN serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(SERVER_PORT);
            InetPton(AF_INET, L"127.0.0.1", &(serverAddr.sin_addr));

            if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                MessageBox(hwndDlg, L"Failed to connect to the server.", L"Error", MB_OK | MB_ICONERROR);
                closesocket(clientSocket);
                WSACleanup();
                EndDialog(hwndDlg, 0);
            }

            MessageBox(hwndDlg, L"Connected to the server.", L"Info", MB_OK | MB_ICONINFORMATION);
        }
        break;

        case IDC_SETNICK_BUTTON:
            // �г��� ���� ��ư ���� �߰�
        {
            wchar_t nickName[256];
            GetWindowText(hNickNameEdit, nickName, sizeof(nickName) / sizeof(nickName[0]));

            char buffer[MAX_BUFFER_SIZE];
            sprintf_s(buffer, sizeof(buffer), "[%ls] joined the chat.\r\n", nickName);

            send(clientSocket, buffer, strlen(buffer), 0);
        }
        break;

        case IDC_SEND_BUTTON:
            // �޽��� ���� ��ư ���� �߰�
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

        case IDC_EXIT_BUTTON:
            // Ŭ���̾�Ʈ ���� ��ư ���� �߰�
            closesocket(clientSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        // Ŭ���̾�Ʈ ���� ó�� �߰�
        closesocket(clientSocket);
        WSACleanup();
        EndDialog(hwndDlg, 0);
        break;

        // �߰����� �̺�Ʈ ó��

    default:
        return FALSE;
    }
    return TRUE;
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
