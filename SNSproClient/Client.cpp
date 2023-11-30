#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <cstdio>

#define MAX_BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"  // 여기에 서버의 IP 주소를 입력하세요
#define SERVER_PORT 9000

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    // 클라이언트 소켓 생성
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Failed to create client socket.\n");
        WSACleanup();
        return 1;
    }

    // 서버에 연결
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Failed to connect to the server.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server. Type 'exit' to quit.\n");

    while (true) {
        char message[MAX_BUFFER_SIZE];
        printf("Enter message: ");
        gets_s(message, sizeof(message));

        if (strcmp(message, "exit") == 0) {
            break;
        }

        // 서버에 메시지 전송
        if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
            printf("Failed to send message.\n");
            break;
        }
    }

    // 클라이언트 소켓 닫기
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}