/*
 * TCP Chat Client - Windows (Winsock2)
 * Iterative Connection-Oriented Chat Application
 *
 * Compile (MinGW / MSYS2):
 *   gcc tcp_client.c -o tcp_client.exe -lws2_32
 *
 * Compile (MSVC Developer Prompt):
 *   cl tcp_client.c ws2_32.lib /Fe:tcp_client.exe
 *
 * Run:
 *   tcp_client.exe <server_ip> <port>
 *   tcp_client.exe 192.168.1.10 9000
 *
 * Commands:
 *   REGISTER <username>   — identify yourself (done automatically)
 *   LIST                  — show online users
 *   @<user> <msg>         — private message
 *   QUIT                  — disconnect
 *   <anything else>       — broadcast to all
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h> /* _beginthread */

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define USERNAME_LEN 32

static SOCKET g_sock = INVALID_SOCKET;
static volatile int g_running = 1;

/* ── Receiver thread: prints messages from the server ── */
static void receiver_thread(void *arg)
{
    char buf[BUFFER_SIZE];
    (void)arg;

    while (g_running)
    {
        memset(buf, 0, BUFFER_SIZE);
        int n = recv(g_sock, buf, BUFFER_SIZE - 1, 0);

        if (n <= 0)
        {
            if (g_running)
            {
                printf("\n[!] Disconnected from server.\n");
                g_running = 0;
            }
            break;
        }

        buf[n] = '\0';
        /* Overwrite the "You: " prompt line, print message, reprint prompt */
        printf("\r%s\nYou: ", buf);
        fflush(stdout);
    }
    _endthread();
}

/* ── Main ── */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    /* ── Init Winsock ── */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    /* ── Create TCP socket ── */
    g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_sock == INVALID_SOCKET)
    {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    /* ── Build server address ── */
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid server IP: %s\n", server_ip);
        closesocket(g_sock);
        WSACleanup();
        return 1;
    }

    /* ── Connect (TCP handshake) ── */
    printf("Connecting to %s:%d ...\n", server_ip, port);
    if (connect(g_sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "connect() failed: %d\n", WSAGetLastError());
        closesocket(g_sock);
        WSACleanup();
        return 1;
    }
    printf("Connected!\n");

    /* ── Wait for server's initial greeting ── */
    char greeting[BUFFER_SIZE] = {0};
    int n = recv(g_sock, greeting, BUFFER_SIZE - 1, 0);
    if (n > 0)
    {
        greeting[n] = '\0';
        printf("%s", greeting);
    }

    /* ── Get username from user ── */
    printf("=========================================\n");
    printf("   TCP Chat Client  |  Server: %s:%d\n", server_ip, port);
    printf("=========================================\n");
    char username[USERNAME_LEN];
    printf("Enter your username: ");
    fflush(stdout);
    fgets(username, USERNAME_LEN, stdin);
    username[strcspn(username, "\r\n")] = '\0';

    /* ── Send REGISTER and wait for Welcome confirmation ── */
    char reg_msg[USERNAME_LEN + 10];
    snprintf(reg_msg, sizeof(reg_msg), "REGISTER %s\n", username);
    send(g_sock, reg_msg, (int)strlen(reg_msg), 0);

    char confirm[BUFFER_SIZE] = {0};
    n = recv(g_sock, confirm, BUFFER_SIZE - 1, 0);
    if (n > 0)
    {
        confirm[n] = '\0';
        printf("%s", confirm);
    }
    else
    {
        fprintf(stderr, "[!] Server did not respond to REGISTER.\n");
        closesocket(g_sock);
        WSACleanup();
        return 1;
    }

    /* ── Print help ── */
    printf("\n--- Commands ---\n");
    printf("  LIST              List online users\n");
    printf("  @username <msg>   Send a private message\n");
    printf("  QUIT              Leave the chat\n");
    printf("  <message>         Broadcast to everyone\n");
    printf("----------------\n\n");

    /* ── Start receiver thread ── */
    _beginthread(receiver_thread, 0, NULL);

    /* ── Sender loop (main thread) ── */
    char input[BUFFER_SIZE];
    printf("You: ");
    fflush(stdout);

    while (g_running)
    {
        if (!fgets(input, BUFFER_SIZE, stdin))
            break;
        input[strcspn(input, "\r\n")] = '\0';

        if (!g_running)
            break;

        if (strlen(input) == 0)
        {
            printf("You: ");
            fflush(stdout);
            continue;
        }

        /* Append newline as message delimiter for the server */
        char msg[BUFFER_SIZE + 2];
        snprintf(msg, sizeof(msg), "%s\n", input);

        if (_stricmp(input, "QUIT") == 0)
        {
            send(g_sock, msg, (int)strlen(msg), 0);
            g_running = 0;
            printf("Disconnecting...\n");
            break;
        }

        if (send(g_sock, msg, (int)strlen(msg), 0) == SOCKET_ERROR)
        {
            printf("\n[!] Send failed. Connection lost.\n");
            g_running = 0;
            break;
        }

        printf("You: ");
        fflush(stdout);
    }

    /* ── Cleanup ── */
    Sleep(300);
    closesocket(g_sock);
    WSACleanup();
    printf("Disconnected. Bye!\n");
    return 0;
}