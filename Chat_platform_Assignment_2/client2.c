#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PORT 8080

int main() {
    // Windows-specific initialization
    #ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            printf("Failed. Error Code : %d", WSAGetLastError());
            return 1;
        }
    #endif

    int sock = 0;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from Client";
    char buffer[1024] = {0};

    // 1. Create socket
    sock = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // 2. Set IP Address (Using inet_addr for better Windows compatibility)
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 3. Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // 4. Send data
    send(sock, hello, (int)strlen(hello), 0);
    printf("Message sent to server: %s\n", hello);

    // 5. Receive reply
    recv(sock, buffer, 1024, 0);
    printf("Server replied: %s\n", buffer);

    // 6. Cleanup
    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif
    
    return 0;
}