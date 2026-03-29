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
    typedef int socklen_t; 
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PORT 8081 // Changed from 8080 to avoid port conflict

int candidate_A = 0;
int candidate_B = 0;

int main() {
    #ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2,2), &wsa);
    #endif

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024];

    server_fd = (int)socket(AF_INET, SOCK_STREAM, 0);

    // FIX: Allow immediate restart of server without "Address already in use" error
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(server_fd, 3);
    printf("Voting Server Online (Port %d). Waiting...\n", PORT);

    while(1) {
        // [ACCEPT REQUEST]
        new_socket = (int)accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        // Handle multiple votes per connection
        while(1) {
            // [PROCESS REQUEST]
            memset(buffer, 0, 1024);
            int bytes_read = recv(new_socket, buffer, 1023, 0);
            
            if (bytes_read <= 0) {
                // Connection closed or error
                break;
            }
            
            // Logic to handle 'a' or 'A' and 'b' or 'B'
            char vote = buffer[0];
            char response[1024];

            if (vote == 'A' || vote == 'a') {
                candidate_A++;
                sprintf(response, "VOTE CAST: Candidate A. (Total A: %d, B: %d)", candidate_A, candidate_B);
            } else if (vote == 'B' || vote == 'b') {
                candidate_B++;
                sprintf(response, "VOTE CAST: Candidate B. (Total A: %d, B: %d)", candidate_A, candidate_B);
            } else if (vote == 'Q' || vote == 'q') {
                sprintf(response, "Goodbye. Final Tally: A=%d B=%d", candidate_A, candidate_B);
            } else {
                sprintf(response, "INVALID VOTE. Use A or B, or Q to quit.");
            }

            // [FORMULATE AND SEND REPLY]
            send(new_socket, response, (int)strlen(response), 0);
            printf("Processed vote: %c | Current Tally: A=%d B=%d\n", vote, candidate_A, candidate_B);
            
            if (vote == 'Q' || vote == 'q') {
                break; // End this client's session
            }
        }

        // [EXIT CONNECTION]
        #ifdef _WIN32
            closesocket(new_socket);
        #else
            close(new_socket);
        #endif
    }
    return 0;
}