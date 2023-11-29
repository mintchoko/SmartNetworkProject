#include "Common.h"
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 에디트 컨트롤 출력 함수
void DisplayText(const char* fmt, ...);
// 소켓 함수 오류 출력
void DisplayError(const char* msg);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);

SOCKET sock; // 소켓
char buf[BUFSIZE + 1]; // 데이터 송수신 버퍼
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hSendButton; // 보내기 버튼
HWND hEdit1, hEdit2; // 에디트 컨트롤

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CHAT_DIALOG), NULL, DlgProc);

	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 대화상자 프로시저
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
			EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 대기
			GetDlgItemTextA(hDlg, IDC_MESSAGE_EDIT, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent); // 쓰기 완료 알림
			SetFocus(hEdit1); // 키보드 포커스 전환
			SendMessage(hEdit1, EM_SETSEL, 0, -1); // 텍스트 전체 선택
			return TRUE;
		case IDC_EXIT_BUTTON:
			EndDialog(hDlg, IDC_EXIT_BUTTON); // 대화상자 닫기
			closesocket(sock); // 소켓 닫기
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// 에디트 컨트롤 출력 함수
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

// 소켓 함수 오류 출력
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

// TCP 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// 소켓 생성
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

	// 서버와 데이터 통신
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 대기

		// 문자열 길이가 0이면 보내지 않음
		if (strlen(buf) == 0) {
			EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
			SetEvent(hReadEvent); // 읽기 완료 알림
			continue;
		}

		// 데이터 보내기
		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			DisplayError("send()");
			break;
		}
		DisplayText("[TCP 클라이언트] %d바이트를 보냈습니다.\r\n", retval);

		// 데이터 받기
		retval = recv(sock, buf, retval, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			DisplayError("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		DisplayText("[TCP 클라이언트] %d바이트를 받았습니다.\r\n", retval);
		DisplayText("[받은 데이터] %s\r\n", buf);

		EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
		SetEvent(hReadEvent); // 읽기 완료 알림
	}

	return 0;
}


/*#include <winsock2.h>
#include <windows.h>
#include <cstdio>    // 추가: sprintf_s를 사용하기 위한 헤더 파일
#include <ws2tcpip.h> // 추가: InetPton을 사용하기 위한 헤더 파일
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

        // 윈속 초기화
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            MessageBox(hwndDlg, L"Failed to initialize Winsock.", L"Error", MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, 0);
        }

        // 클라이언트 소켓 생성
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
            // 연결 버튼 동작 추가
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

        case IDC_EXIT_BUTTON:
            // 클라이언트 종료 버튼 동작 추가
            closesocket(clientSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        // 클라이언트 종료 처리 추가
        closesocket(clientSocket);
        WSACleanup();
        EndDialog(hwndDlg, 0);
        break;

        // 추가적인 이벤트 처리

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
    // 대화상자 생성
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CHAT_DIALOG), NULL, DialogProc);

    return 0;
}
*/