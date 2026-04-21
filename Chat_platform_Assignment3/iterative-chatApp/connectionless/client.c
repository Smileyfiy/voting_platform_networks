/*
 * UDP Chat Client - Windows
 * Iterative Connectionless Chat Application
 *
 * Compile (MinGW / MSYS2):
 *   gcc client.c -o client.exe -lws2_32
 *
 * Compile (MSVC Developer Prompt):
 *   cl client.c ws2_32.lib /Fe:client.exe
 *
 * Run:
 *   client.exe <server_ip> <port>
 *   client.exe 192.168.1.10 9000
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

/* Shared between threads */
static SOCKET g_sock = INVALID_SOCKET;
static struct sockaddr_in g_server;
static int g_socklen = sizeof(g_server);
static volatile int g_running = 1;
static char g_username[USERNAME_LEN];

/* ── Receiver thread: prints incoming messages ── */
void receiver_thread(void *arg)
{
    char buf[BUFFER_SIZE];
    (void)arg;

    while (g_running)
    {
        memset(buf, 0, BUFFER_SIZE);
        int n = recvfrom(g_sock, buf, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&g_server,
                         &g_socklen);
        if (n < 0)
        {
            int err = WSAGetLastError();
            /* WSAETIMEDOUT (10060) and WSAEWOULDBLOCK (10035) are harmless --
               they just mean no packet arrived in this window. Keep looping. */
            if (err == WSAETIMEDOUT || err == WSAEWOULDBLOCK)
                continue;
            /* Any other error while still running is a real problem */
            if (g_running)
                fprintf(stderr, "\n[!] Receive error (WSA %d). Retrying...\n", err);
            Sleep(500);
            continue;
        }
        if (n == 0)
            continue; /* empty datagram, ignore */

        buf[n] = '\0';
        /* Move to new line so incoming text doesn't collide with typed input */
        printf("\r%s\nYou: ", buf);
        fflush(stdout);
    }
    _endthread();
}

/* ── Send a raw string to the server ── */
static void send_msg(const char *msg)
{
    sendto(g_sock, msg, (int)strlen(msg), 0,
           (struct sockaddr *)&g_server, g_socklen);
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

    /* ── Create UDP socket ── */
    g_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_sock == INVALID_SOCKET)
    {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    /* Set receive timeout (10 seconds). On timeout, the receiver loop simply
       continues -- it will NOT exit unless g_running becomes 0 or a real
       socket error occurs (see receiver_thread). */
    DWORD timeout = 10000;
    setsockopt(g_sock, SOL_SOCKET, SO_RCVTIMEO,
               (char *)&timeout, sizeof(timeout));

    /* ── Build server address ── */
    memset(&g_server, 0, sizeof(g_server));
    g_server.sin_family = AF_INET;
    g_server.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &g_server.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid server IP: %s\n", server_ip);
        closesocket(g_sock);
        WSACleanup();
        return 1;
    }

    /* ── Get username ── */
    printf("=========================================\n");
    printf("   UDP Chat Client  |  Server: %s:%d\n", server_ip, port);
    printf("=========================================\n");
    printf("Enter your username: ");
    fflush(stdout);
    fgets(g_username, USERNAME_LEN, stdin);
    g_username[strcspn(g_username, "\r\n")] = '\0';

    /* ── Register with server and wait for confirmation ── */
    char reg_msg[USERNAME_LEN + 10];
    snprintf(reg_msg, sizeof(reg_msg), "REGISTER %s", g_username);

    /* Retry REGISTER up to 5 times until we get a reply */
    char confirm[BUFFER_SIZE];
    int registered = 0;
    for (int attempt = 1; attempt <= 5; attempt++)
    {
        send_msg(reg_msg);
        memset(confirm, 0, BUFFER_SIZE);
        int n = recvfrom(g_sock, confirm, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&g_server, &g_socklen);
        if (n > 0)
        {
            confirm[n] = '\0';
            printf("\r%s\n", confirm); /* print "Welcome!" or "Username updated" */
            registered = 1;
            break;
        }
        printf("[!] No response from server (attempt %d/5), retrying...\n", attempt);
    }
    if (!registered)
    {
        fprintf(stderr, "[!] Could not reach server at %s:%d. Exiting.\n",
                server_ip, port);
        closesocket(g_sock);
        WSACleanup();
        return 1;
    }

    /* ── Print usage help ── */
    printf("\n--- Commands ---\n");
    printf("  LIST          List online users\n");
    printf("  QUIT          Leave the chat\n");
    printf("  <message>     Send a message\n");
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

        if (strlen(input) == 0)
        {
            printf("You: ");
            fflush(stdout);
            continue;
        }

        /* QUIT */
        if (_stricmp(input, "QUIT") == 0)
        {
            send_msg("QUIT");
            g_running = 0;
            printf("Disconnecting...\n");
            break;
        }

        /* LIST */
        if (_stricmp(input, "LIST") == 0)
        {
            send_msg("LIST");
            printf("You: ");
            fflush(stdout);
            continue;
        }

        /* Regular message */
        send_msg(input);
        printf("You: ");
        fflush(stdout);
    }

    /* ── Cleanup ── */
    Sleep(500); /* let receiver thread notice g_running=0 */
    closesocket(g_sock);
    WSACleanup();
    printf("Disconnected. Bye!\n");
    return 0;
}