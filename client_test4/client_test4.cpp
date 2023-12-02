#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <process.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <cstdio>    // 추가: sprintf_s를 사용하기 위한 헤더 파일
#include <ws2tcpip.h> // 추가: InetPton을 사용하기 위한 헤더 파일
#include "resource.h"

#define MAX_BUFFER_SIZE 512
#define SERVER_PORT 9000
#define SERVER_IP "127.0.0.1"

int client_init(char* ip, int port);
DWORD WINAPI chat_service(void* param);
LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DisplayText(const char* fmt, ...);

HWND hNickNameEdit, hChatHistoryEdit, hMessageEdit, hSendButton;
SOCKET clientSocket;
WSADATA wsaData;
HANDLE hReadEvent, hWriteEvent; // 이벤트
char buf[MAX_BUFFER_SIZE + 1]; // 데이터 송수신 버퍼

char ip_addr[256] = "";
int port_number = 9000;
char nickname[50] = "";

void DisplayText(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cbuf[MAX_BUFFER_SIZE * 2];
	vsprintf(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(hChatHistoryEdit);
	SendMessage(hChatHistoryEdit, EM_SETSEL, nLength, nLength);
	SendMessageA(hChatHistoryEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}

LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        hNickNameEdit = GetDlgItem(hwndDlg, IDC_NICKNAME_EDIT);
        hChatHistoryEdit = GetDlgItem(hwndDlg, IDC_CHAT_HISTORY_EDIT);
        hMessageEdit = GetDlgItem(hwndDlg, IDC_MESSAGE_EDIT);
		hSendButton = GetDlgItem(hwndDlg, IDC_SEND_BUTTON);
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
			nickname = 
            char buffer[MAX_BUFFER_SIZE];
            sprintf_s(buffer, sizeof(buffer), "[%ls] joined the chat.\r\n", nickName);

            send(clientSocket, buffer, strlen(buffer), 0);
        }
        break;

        case IDC_SEND_BUTTON:
            // 메시지 전송 버튼 동작 추가
        {

			EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 대기
			GetDlgItemTextA(hwndDlg, IDC_MESSAGE_EDIT, buf, MAX_BUFFER_SIZE + 1);
			SetEvent(hWriteEvent); // 쓰기 완료 알림
			SetFocus(hMessageEdit); // 키보드 포커스 전환
			SendMessage(hMessageEdit, EM_SETSEL, 0, -1); // 텍스트 전체 선택
			SetWindowText(hMessageEdit, L"");
			return TRUE;
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

int client_init(char* ip, int port)
{
	
	SOCKET server_socket;
	WSADATA wsadata;
	SOCKADDR_IN server_address = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		MessageBox(NULL, L"WSAStartup 에러.", L"오류", MB_ICONERROR);
		return -1;
	}

	if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		MessageBox(NULL, L"socket 에러.", L"오류", MB_ICONERROR);
		return -1;
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(ip);
	server_address.sin_port = htons(port);

	if ((connect(server_socket, (struct sockaddr*)&server_address, sizeof(server_address))) < 0)
	{
		MessageBox(NULL, L"connect 에러", L"오류", MB_ICONERROR);
		return -1;
	}

	return server_socket;
}

DWORD WINAPI chat_service(void* params)
{
	char recv_message[MAXBYTE];
	int len = 0;
	int index = 0;
	WSANETWORKEVENTS ev;
	int retval;
	
	SOCKET s = client_init(ip_addr, port_number);

	while (1)
	{
		
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 대기

		// 문자열 길이가 0이면 보내지 않음
		if (strlen(buf) == 0) {
			EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
			SetEvent(hReadEvent); // 읽기 완료 알림
			continue;
		}

		// 데이터 보내기
		retval = send(s, buf, (int)strlen(buf), 0);
		DisplayText("[TCP 클라이언트] %d바이트를 보냈습니다.\r\n", retval);

		// 데이터 받기
		retval = recv(s, buf, retval, MSG_WAITALL);
		if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		DisplayText("[TCP 클라이언트] %d바이트를 받았습니다.\r\n", retval);
		DisplayText("[받은 데이터] %s\r\n", buf);

		EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
		SetEvent(hReadEvent); // 읽기 완료 알림
	}
	WSACleanup();
	_endthreadex(0);

	return 0;
}

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
) {
	HANDLE mainthread;
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	
	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    strcpy_s(ip_addr, SERVER_IP);//서버 주소
    port_number = SERVER_PORT;//포트 번호
    strcpy_s(nickname, "1");//별명

	
	
	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, chat_service, NULL, 0, NULL);

	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CHAT_DIALOG), NULL, DialogProc);

	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	// 윈속 종료
	WSACleanup();
	return 0;
}