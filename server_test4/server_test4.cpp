#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include <winsock2.h>
#include <windows.h>
#include <cstdio>    // 추가: sprintf_s를 사용하기 위한 헤더 파일
#include "resource.h"
#include <stdio.h>
#include <process.h>
#include <string.h>

#define MAX_BUFFER_SIZE 512

typedef struct sock_info
{
    SOCKET s;
    HANDLE ev;
    char nick[50];
    char ipaddr[50];
}SOCK_INFO;

int port_number = 9000;
const int client_count = 10;
SOCK_INFO sock_array[client_count + 1];
int total_socket_count = 0;
SOCKET serverSocket;
char buf[MAX_BUFFER_SIZE + 1]; // 데이터 송수신 버퍼
HANDLE hReadEvent, hWriteEvent; // 이벤트

HWND hChatHistoryEdit, hMessageEdit, hSendButton;  // 추가: 서버 IP 주소를 표시할 정적 텍스트 컨트롤 핸들
CRITICAL_SECTION cs;



LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI chat_service(void* param);
int server_init();
int add_client(int index);
int read_client(int index);
void remove_client(int index);
int notify_client(char* message);
int server_close();
unsigned int WINAPI recv_and_forward(void* param);
char* get_client_ip(int index);
void DisplayText(const char* fmt, ...);

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
        hChatHistoryEdit = GetDlgItem(hwndDlg, IDC_CHAT_HISTORY_EDIT);
        hMessageEdit = GetDlgItem(hwndDlg, IDC_MESSAGE_EDIT);
        hSendButton = GetDlgItem(hwndDlg, IDC_SEND_BUTTON);
        return 0;
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_SEND_BUTTON:
            // 메시지 전송 버튼 동작 추가
        {
            char message[MAX_BUFFER_SIZE];
            GetWindowTextA(hMessageEdit, message, sizeof(message));
            DisplayText("%s\r\n", message);
            notify_client(message);
            SetWindowTextA(hMessageEdit, "");
            return TRUE;
        }
        case IDC_EXIT_BUTTON:
            // 서버 종료 버튼 동작 추가
            server_close();
            closesocket(serverSocket);
            WSACleanup();
            EndDialog(hwndDlg, 0);
            break;
        }
        return 0;

    case WM_CLOSE:
        // 서버 종료 처리 추가
        server_close();
        closesocket(serverSocket);
        WSACleanup();
        EndDialog(hwndDlg, 0);
        break;

        // 추가적인 이벤트 처리

    default:
        return FALSE;
    }
    return 0;
}

int server_init() {
    WSADATA wsadata;
    SOCKET s;
    SOCKADDR_IN server_address;

    memset(&sock_array, 0, sizeof(sock_array));
    total_socket_count = 0;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
    {
        MessageBox(NULL, L"WSAStartup 에러.", L"오류", MB_ICONERROR);
        return -1;
    }

    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        MessageBox(NULL, L"socket 에러.", L"오류", MB_ICONERROR);
        return -1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port_number);

    if (bind(s, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        MessageBox(NULL, L"bind 에러", L"오류", MB_ICONERROR);
        return -2;
    }

    if (listen(s, SOMAXCONN) < 0)
    {
        MessageBox(NULL, L"listen 에러", L"오류", MB_ICONERROR);
        return -3;
    }

    return s;
}

int add_client(int index) {
    SOCKADDR_IN addr;
    int len = 0;
    SOCKET accept_sock;

    if (total_socket_count == FD_SETSIZE)
        return 1;
    else {

        len = sizeof(addr);
        memset(&addr, 0, sizeof(addr));
        accept_sock = accept(sock_array[0].s, (SOCKADDR*)&addr, &len);

        HANDLE event = WSACreateEvent();
        sock_array[total_socket_count].ev = event;
        sock_array[total_socket_count].s = accept_sock;
        strcpy_s(sock_array[total_socket_count].ipaddr, inet_ntoa(addr.sin_addr));

        WSAEventSelect(accept_sock, event, FD_READ | FD_CLOSE);

        total_socket_count++;

        char msg[256];
        sprintf_s(msg, " >> 신규 클라이언트 접속(별명 : %s)\n", sock_array[index].nick);
        notify_client(msg);
        DisplayText("%s\r\n", msg);
    }
    return 0;
}

int read_client(int index) {
    unsigned int tid;
    HANDLE mainthread = (HANDLE)_beginthreadex(NULL, 0, recv_and_forward, (void*)index, 0, &tid);
    WaitForSingleObject(mainthread, INFINITE);

    CloseHandle(mainthread);

    return 0;
}

void remove_client(int index) {
    char message[MAXBYTE];
    sprintf_s(message, " >> 클라이언트 접속 종료(별명: %s)\n", sock_array[index].nick);

    closesocket(sock_array[index].s);
    WSACloseEvent(sock_array[index].ev);

    total_socket_count--;
    sock_array[index].s = sock_array[total_socket_count].s;
    sock_array[index].ev = sock_array[total_socket_count].ev;
    strcpy_s(sock_array[index].ipaddr, sock_array[total_socket_count].ipaddr);
    strcpy_s(sock_array[index].nick, sock_array[total_socket_count].nick);

    notify_client(message);
    DisplayText("%s\r\n", message);
}

char* get_client_ip(int index)
{
    static char ipaddress[256];
    int addr_len;
    struct sockaddr_in sock;

    addr_len = sizeof(sock);
    if (getpeername(sock_array[index].s, (struct sockaddr*)&sock, &addr_len) < 0)
        return NULL;

    strcpy_s(ipaddress, inet_ntoa(sock.sin_addr));
    return ipaddress;
}

int notify_client(char* message) {
    for (int i = 1; i < total_socket_count; i++)
        send(sock_array[i].s, message, MAXBYTE, 0);

    return 0;
}

int server_close() {
    for (int i = 1; i < total_socket_count; i++)
    {
        closesocket(sock_array[i].s);
        WSACloseEvent(sock_array[i].ev);
    }
    return 0;
}

unsigned int WINAPI recv_and_forward(void* param) {
    int index = (int)param;
    char message[MAXBYTE], share_message[MAXBYTE];
    SOCKADDR_IN client_address;
    int recv_len = 0, addr_len = 0;
    char* token1 = NULL;
    char* next_token = NULL;

    memset(&client_address, 0, sizeof(client_address));

    while ((recv_len = recv(sock_array[index].s, message, MAXBYTE, 0)) > 0)
    {
        DisplayText("%s\r\n", message);
        addr_len = sizeof(client_address);
        getpeername(sock_array[index].s, (SOCKADDR*)&client_address, &addr_len);
        strcpy_s(share_message, message);

        if (strlen(sock_array[index].nick) <= 0)
        {
            token1 = strtok_s(message, "]", &next_token);
            strcpy_s(sock_array[index].nick, token1 + 1);
        }
        for (int i = 1; i < total_socket_count; i++)
            send(sock_array[i].s, share_message, MAXBYTE, 0);
    }

    _endthreadex(0);
    return 0;
}

DWORD WINAPI chat_service(void* param) {
    WSANETWORKEVENTS ev;
    int index;
    WSAEVENT handle_array[client_count + 1];

    serverSocket = server_init();

    HANDLE event = WSACreateEvent();
    sock_array[total_socket_count].ev = event;
    sock_array[total_socket_count].s = serverSocket;
    strcpy_s(sock_array[total_socket_count].nick, "svr");
    strcpy_s(sock_array[total_socket_count].ipaddr, "0.0.0.0");

    WSAEventSelect(serverSocket, event, FD_ACCEPT);
    total_socket_count++;

    while (true)
    {
        memset(&handle_array, 0, sizeof(handle_array));
        for (int i = 0; i < total_socket_count; i++)
            handle_array[i] = sock_array[i].ev;

        index = WSAWaitForMultipleEvents(total_socket_count,
            handle_array, false, INFINITE, false);
        if ((index != WSA_WAIT_FAILED) && (index != WSA_WAIT_TIMEOUT))
        {
            
            WSAEnumNetworkEvents(sock_array[index].s, sock_array[index].ev, &ev);
            if (ev.lNetworkEvents == FD_ACCEPT)
                add_client(index);
            else if (ev.lNetworkEvents == FD_READ)
                read_client(index);
            else if (ev.lNetworkEvents == FD_CLOSE)
                remove_client(index);
        }
    }
    closesocket(serverSocket);

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
    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    InitializeCriticalSection(&cs);
    // 소켓 통신 스레드 생성
    CreateThread(NULL, 0, chat_service, NULL, 0, NULL);

    // 대화상자 생성
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CHAT_DIALOG), NULL, DialogProc);

    // 메시지 루프
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DeleteCriticalSection(&cs);
    return 0;
}