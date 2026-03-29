#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif
#include <stdio.h>
#include <stdlib.h>
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

#define PORT 8081

int main() {
    #ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2,2), &wsa);
    #endif

    int sock = 0;
    struct sockaddr_in serv_addr;
    char choice[10];
    char buffer[1024] = {0};

    sock = (int)socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed. Check if server is running on port %d.\n", PORT);
        return -1;
    }

    printf("Connected to voting server. Enter votes (A/B) or Q to quit.\n");

    while (1) {
        printf("Enter Vote (A/B) or Q to quit: ");
        scanf("%s", choice);

        if (choice[0] == 'Q' || choice[0] == 'q') {
            send(sock, "Q", 1, 0);
            recv(sock, buffer, 1024, 0);
            printf("Server says: %s\n", buffer);
            break;
        }

        send(sock, choice, (int)strlen(choice), 0);
        recv(sock, buffer, 1024, 0);
        printf("Server says: %s\n", buffer);
    }

    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif

    printf("\nDone. Press any key to exit.");
    return 0;
}