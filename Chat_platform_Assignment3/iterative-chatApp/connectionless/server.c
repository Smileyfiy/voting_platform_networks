/*
 * UDP Chat Server - Windows (Winsock2)
 * Iterative Connectionless Chat Application
 *
 * Compile (MinGW / MSYS2):
 *   gcc server_windows.c -o server.exe -lws2_32
 *
 * Compile (MSVC Developer Prompt):
 *   cl server_windows.c ws2_32.lib /Fe:server.exe
 *
 * Run:
 *   server.exe <port>
 *   server.exe 9000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define USERNAME_LEN 32

/* ── Registered client record ── */
typedef struct
{
    struct sockaddr_in addr;
    char username[USERNAME_LEN];
    int active;
} Client;

/* ── Helpers ── */
static void timestamp(char *buf, size_t len)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%H:%M:%S", tm_info);
}

static int addr_equal(const struct sockaddr_in *a, const struct sockaddr_in *b)
{
    return (a->sin_addr.s_addr == b->sin_addr.s_addr &&
            a->sin_port == b->sin_port);
}

/* Find existing client slot; returns index or -1 */
static int find_client(Client *clients, int n, const struct sockaddr_in *addr)
{
    for (int i = 0; i < n; i++)
        if (clients[i].active && addr_equal(&clients[i].addr, addr))
            return i;
    return -1;
}

/* Find a free slot; returns index or -1 */
static int free_slot(Client *clients, int n)
{
    for (int i = 0; i < n; i++)
        if (!clients[i].active)
            return i;
    return -1;
}

/* Broadcast a message to every active client except the sender */
static void broadcast(SOCKET sock, Client *clients, int n,
                      const struct sockaddr_in *sender,
                      const char *msg)
{
    for (int i = 0; i < n; i++)
    {
        if (!clients[i].active)
            continue;
        if (sender && addr_equal(&clients[i].addr, sender))
            continue;
        sendto(sock, msg, (int)strlen(msg), 0,
               (struct sockaddr *)&clients[i].addr,
               sizeof(clients[i].addr));
    }
}

/* ── Main ── */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    /* ── Init Winsock ── */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    /* ── Create UDP socket ── */
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    /* Allow address reuse */
    BOOL opt = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    /* ── Bind ── */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((u_short)port);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    Client clients[MAX_CLIENTS];
    memset(clients, 0, sizeof(clients));

    char buffer[BUFFER_SIZE];
    char ts[16], out[BUFFER_SIZE + 64];
    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);

    printf("=== UDP Chat Server (Windows) started on port %d ===\n", port);
    printf("Waiting for clients...\n\n");

    /* ── Iterative server loop ── */
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&client_addr, &client_len);

        if (n == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            /* WSAETIMEDOUT / WSAEWOULDBLOCK are harmless, keep looping */
            if (err == WSAETIMEDOUT || err == WSAEWOULDBLOCK)
                continue;
            fprintf(stderr, "recvfrom() error: %d\n", err);
            continue;
        }

        buffer[n] = '\0';
        timestamp(ts, sizeof(ts));

        int idx = find_client(clients, MAX_CLIENTS, &client_addr);

        /* ── REGISTER command: "REGISTER <username>" ── */
        if (strncmp(buffer, "REGISTER ", 9) == 0)
        {
            char *uname = buffer + 9;
            uname[strcspn(uname, "\r\n")] = '\0';

            if (idx >= 0)
            {
                /* Already registered – update username */
                strncpy(clients[idx].username, uname, USERNAME_LEN - 1);
                sendto(sock, "SERVER: Username updated.\n", 26, 0,
                       (struct sockaddr *)&client_addr, client_len);
            }
            else
            {
                int slot = free_slot(clients, MAX_CLIENTS);
                if (slot < 0)
                {
                    sendto(sock, "SERVER: Chat room is full. Try later.\n", 38, 0,
                           (struct sockaddr *)&client_addr, client_len);
                }
                else
                {
                    clients[slot].addr = client_addr;
                    clients[slot].active = 1;
                    strncpy(clients[slot].username, uname, USERNAME_LEN - 1);

                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
                    printf("[%s] + %s joined from %s:%d\n",
                           ts, uname, ip, ntohs(client_addr.sin_port));

                    sendto(sock, "SERVER: Welcome to the chat!\n", 29, 0,
                           (struct sockaddr *)&client_addr, client_len);

                    /* Notify others */
                    snprintf(out, sizeof(out),
                             "SERVER: %s has joined the chat.\n", uname);
                    broadcast(sock, clients, MAX_CLIENTS, &client_addr, out);
                }
            }
            continue;
        }

        /* ── QUIT command ── */
        if (strncmp(buffer, "QUIT", 4) == 0 && idx >= 0)
        {
            printf("[%s] - %s left.\n", ts, clients[idx].username);
            snprintf(out, sizeof(out),
                     "SERVER: %s has left the chat.\n", clients[idx].username);
            sendto(sock, "SERVER: Goodbye!\n", 17, 0,
                   (struct sockaddr *)&client_addr, client_len);
            clients[idx].active = 0;
            broadcast(sock, clients, MAX_CLIENTS, &client_addr, out);
            continue;
        }

        /* ── LIST command ── */
        if (strncmp(buffer, "LIST", 4) == 0)
        {
            if (idx < 0)
            {
                sendto(sock,
                       "SERVER: Please register first. Send: REGISTER <username>\n",
                       57, 0, (struct sockaddr *)&client_addr, client_len);
                continue;
            }
            char list[BUFFER_SIZE] = "SERVER: Online users: ";
            int found = 0;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (!clients[i].active)
                    continue;
                strncat(list, clients[i].username,
                        sizeof(list) - strlen(list) - 2);
                strncat(list, " ", sizeof(list) - strlen(list) - 1);
                found++;
            }
            if (!found)
                strncat(list, "(none)", sizeof(list) - strlen(list) - 1);
            strncat(list, "\n", sizeof(list) - strlen(list) - 1);
            sendto(sock, list, (int)strlen(list), 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

        /* ── Regular chat message ── */
        if (idx >= 0)
        {
            printf("[%s] %s: %s", ts, clients[idx].username, buffer);
            snprintf(out, sizeof(out), "[%s] %s: %s",
                     ts, clients[idx].username, buffer);
            /* Echo back to sender */
            sendto(sock, out, (int)strlen(out), 0,
                   (struct sockaddr *)&client_addr, client_len);
            /* Broadcast to everyone else */
            broadcast(sock, clients, MAX_CLIENTS, &client_addr, out);
        }
        else
        {
            /* Unregistered client */
            sendto(sock,
                   "SERVER: Please register first. Send: REGISTER <username>\n",
                   57, 0, (struct sockaddr *)&client_addr, client_len);
        }
    }

    /* Cleanup (unreachable in normal flow, but good practice) */
    closesocket(sock);
    WSACleanup();
    return 0;
}