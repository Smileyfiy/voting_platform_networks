#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    // Windows-specific initialization
    #ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            printf("Failed. Error Code : %d", WSAGetLastError());
            return 1;
        }
    #endif

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server listening on port %d...\n", PORT);

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        recv(new_socket, buffer, BUFFER_SIZE, 0); // Use recv instead of read for Windows
        printf("Client sent: %s\n", buffer);

        char *reply = "Message received by server";
        send(new_socket, reply, (int)strlen(reply), 0);

        #ifdef _WIN32
            closesocket(new_socket);
        #else
            close(new_socket);
        #endif
        memset(buffer, 0, BUFFER_SIZE);
    }

    return 0;
}