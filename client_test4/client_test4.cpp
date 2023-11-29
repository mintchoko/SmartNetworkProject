#include "Common.h"
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

// ��ȭ���� ���ν���
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(const char* fmt, ...);
// ���� �Լ� ���� ���
void DisplayError(const char* msg);
// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg);

SOCKET sock; // ����
char buf[BUFSIZE + 1]; // ������ �ۼ��� ����
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
HWND hSendButton; // ������ ��ư
HWND hEdit1, hEdit2; // ����Ʈ ��Ʈ��

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// �̺�Ʈ ����
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// ���� ��� ������ ����
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// ��ȭ���� ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CHAT_DIALOG), NULL, DlgProc);

	// �̺�Ʈ ����
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	// ���� ����
	WSACleanup();
	return 0;
}

// ��ȭ���� ���ν���
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_MESSAGE_EDIT);
		hEdit2 = GetDlgItem(hDlg, IDC_CHAT_HISTORY_EDIT);
		hSendButton = GetDlgItem(hDlg, IDOK);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SEND_BUTTON:
			EnableWindow(hSendButton, FALSE); // ������ ��ư ��Ȱ��ȭ
			WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ���
			GetDlgItemTextA(hDlg, IDC_MESSAGE_EDIT, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent); // ���� �Ϸ� �˸�
			SetFocus(hEdit1); // Ű���� ��Ŀ�� ��ȯ
			SendMessage(hEdit1, EM_SETSEL, 0, -1); // �ؽ�Ʈ ��ü ����
			return TRUE;
		case IDC_EXIT_BUTTON:
			EndDialog(hDlg, IDC_EXIT_BUTTON); // ��ȭ���� �ݱ�
			closesocket(sock); // ���� �ݱ�
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cbuf[BUFSIZE * 2];
	vsprintf(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessageA(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}

// ���� �Լ� ���� ���
void DisplayError(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s\r\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// TCP Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// ���� ����
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ������ ���
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // ���� �Ϸ� ���

		// ���ڿ� ���̰� 0�̸� ������ ����
		if (strlen(buf) == 0) {
			EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
			SetEvent(hReadEvent); // �б� �Ϸ� �˸�
			continue;
		}

		// ������ ������
		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			DisplayError("send()");
			break;
		}
		DisplayText("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\r\n", retval);

		// ������ �ޱ�
		retval = recv(sock, buf, retval, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			DisplayError("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		DisplayText("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\r\n", retval);
		DisplayText("[���� ������] %s\r\n", buf);

		EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
		SetEvent(hReadEvent); // �б� �Ϸ� �˸�
	}

	return 0;
}


/*#include <winsock2.h>
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
*/