/*
 * TCP Chat Server - Windows (Winsock2)
 * Iterative Connection-Oriented Chat Application
 *
 * Compile (MinGW / MSYS2):
 *   gcc tcp_server_windows.c -o tcp_server.exe -lws2_32
 *
 * Compile (MSVC Developer Prompt):
 *   cl tcp_server_windows.c ws2_32.lib /Fe:tcp_server.exe
 *
 * Run:
 *   tcp_server.exe <port>
 *   tcp_server.exe 9000
 *
 * Commands (sent by client):
 *   REGISTER <username>   — identify yourself
 *   LIST                  — show all connected clients
 *   @<user> <msg>         — private message
 *   QUIT                  — disconnect
 *   <anything else>       — broadcast to all
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
#define BACKLOG 5

/* ── Client record ── */
typedef struct
{
    SOCKET fd;
    struct sockaddr_in addr;
    char username[USERNAME_LEN];
    int active;
} Client;

static Client clients[MAX_CLIENTS];

/* ── Helpers ── */
static void timestamp(char *buf, size_t len)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%H:%M:%S", tm_info);
}

/* Send a full string over TCP (handles partial sends) */
static int send_all(SOCKET fd, const char *msg)
{
    int total = (int)strlen(msg);
    int sent = 0;
    while (sent < total)
    {
        int n = send(fd, msg + sent, total - sent, 0);
        if (n == SOCKET_ERROR)
            return -1;
        sent += n;
    }
    return 0;
}

/* Find a free slot */
static int free_slot(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (!clients[i].active)
            return i;
    return -1;
}

/* Broadcast to every active client except the sender */
static void broadcast(SOCKET sender_fd, const char *msg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].active)
            continue;
        if (clients[i].fd == sender_fd)
            continue;
        send_all(clients[i].fd, msg);
    }
}

/* Remove a client cleanly */
static void remove_client(int idx, const char *ts)
{
    printf("[%s] - %s disconnected.\n", ts, clients[idx].username);
    char out[BUFFER_SIZE];
    snprintf(out, sizeof(out), "SERVER: %s has left the chat.\n",
             clients[idx].username);
    broadcast(clients[idx].fd, out);
    closesocket(clients[idx].fd);
    clients[idx].active = 0;
    clients[idx].fd = INVALID_SOCKET;
}

/* ── Handle one connected client ── */
static void handle_client(SOCKET client_fd, struct sockaddr_in *addr)
{
    char buffer[BUFFER_SIZE];
    char out[BUFFER_SIZE + 64];
    char ts[16];

    int slot = free_slot();
    if (slot < 0)
    {
        send_all(client_fd, "SERVER: Chat room is full. Try later.\n");
        closesocket(client_fd);
        return;
    }

    clients[slot].fd = client_fd;
    clients[slot].addr = *addr;
    clients[slot].active = 1;
    strncpy(clients[slot].username, "anonymous", USERNAME_LEN - 1);

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    timestamp(ts, sizeof(ts));
    printf("[%s] New connection from %s:%d (slot %d)\n",
           ts, ip, ntohs(addr->sin_port), slot);

    send_all(client_fd,
             "SERVER: Connected! Please register: REGISTER <username>\n");

    /* ── Per-client message loop ── */
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        /* ============================================================
         * STEP 2 — PROCESS REQUEST
         * recv() blocks until the client sends data over the TCP stream.
         * The raw bytes land in buffer[] ready to be parsed below.
         * n <= 0 means the client disconnected or the connection broke.
         * ============================================================ */
        int n = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (n <= 0)
        {
            timestamp(ts, sizeof(ts));
            remove_client(slot, ts);
            return;
        }

        buffer[n] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';
        timestamp(ts, sizeof(ts));

        /* ── REGISTER ── */
        if (strncmp(buffer, "REGISTER ", 9) == 0)
        {
            char *uname = buffer + 9;
            uname[strcspn(uname, "\r\n")] = '\0';
            strncpy(clients[slot].username, uname, USERNAME_LEN - 1);
            clients[slot].username[USERNAME_LEN - 1] = '\0';

            printf("[%s] + %s registered from %s:%d\n",
                   ts, uname, ip, ntohs(addr->sin_port));

            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (REGISTER response)
             * Build the welcome string and the broadcast notification.
             * ============================================================ */
            /* STEP 4 — SEND REPLY */
            send_all(client_fd, "SERVER: Welcome to the chat!\n");

            snprintf(out, sizeof(out),
                     "SERVER: %s has joined the chat.\n", uname);
            broadcast(client_fd, out);
            continue;
        }

        /* ── QUIT ── */
        if (strncmp(buffer, "QUIT", 4) == 0)
        {
            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (QUIT response)
             * ============================================================ */
            /* STEP 4 — SEND REPLY */
            send_all(client_fd, "SERVER: Goodbye!\n");

            /* ============================================================
             * STEP 5 — EXIT
             * Close this client's socket, mark slot inactive, notify
             * others, then return — putting the server back at STEP 1
             * (accept()) ready for the next client.
             * ============================================================ */
            remove_client(slot, ts);
            return;
        }

        /* ── LIST ── */
        if (strncmp(buffer, "LIST", 4) == 0)
        {
            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (LIST response)
             * Walk the client table and build the online-users string.
             * ============================================================ */
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

            /* STEP 4 — SEND REPLY */
            send_all(client_fd, list);
            continue;
        }

        /* ── Private message: @username <message> ── */
        if (buffer[0] == '@')
        {
            char target[USERNAME_LEN] = {0};
            char *space = strchr(buffer + 1, ' ');
            if (!space)
            {
                send_all(client_fd,
                         "SERVER: Usage: @username <message>\n");
                continue;
            }
            size_t name_len = space - (buffer + 1);
            if (name_len >= USERNAME_LEN)
                name_len = USERNAME_LEN - 1;
            strncpy(target, buffer + 1, name_len);
            target[name_len] = '\0';
            char *pm_text = space + 1;

            int target_idx = -1;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (!clients[i].active)
                    continue;
                if (_stricmp(clients[i].username, target) == 0)
                {
                    target_idx = i;
                    break;
                }
            }

            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (private message response)
             * Build PM delivery for target and confirmation for sender.
             * ============================================================ */
            if (target_idx < 0)
            {
                snprintf(out, sizeof(out),
                         "SERVER: User '%s' not found or not online.\n",
                         target);
                /* STEP 4 — SEND REPLY */
                send_all(client_fd, out);
            }
            else if (target_idx == slot)
            {
                /* STEP 4 — SEND REPLY */
                send_all(client_fd,
                         "SERVER: You cannot message yourself.\n");
            }
            else
            {
                snprintf(out, sizeof(out), "[%s] [PM from %s]: %s\n",
                         ts, clients[slot].username, pm_text);
                /* STEP 4 — SEND REPLY (to target) */
                send_all(clients[target_idx].fd, out);

                snprintf(out, sizeof(out), "[%s] [PM to %s]: %s\n",
                         ts, target, pm_text);
                /* STEP 4 — SEND REPLY (confirmation to sender) */
                send_all(client_fd, out);

                printf("[%s] PM: %s -> %s: %s\n",
                       ts, clients[slot].username, target, pm_text);
            }
            continue;
        }

        /* ── Regular broadcast ── */
        /* ============================================================
         * STEP 3 — FORMULATE REPLY  (broadcast message)
         * Format the message with timestamp and sender's username.
         * ============================================================ */
        printf("[%s] %s: %s\n", ts, clients[slot].username, buffer);
        snprintf(out, sizeof(out), "[%s] %s: %s\n",
                 ts, clients[slot].username, buffer);

        /* STEP 4 — SEND REPLY (echo to sender + broadcast to all others) */
        send_all(client_fd, out);
        broadcast(client_fd, out);
    }
}

/* ── NOTE ON STEP 5 ────────────────────────────────────────────────────
 * EXIT happens in two ways:
 *   a) Client sends QUIT  → send_all goodbye → remove_client() → return
 *   b) Client drops conn  → recv() returns 0 → remove_client() → return
 * remove_client() closes the socket and marks the slot inactive.
 * Returning from handle_client() puts the server back at STEP 1
 * (the accept() call in main), ready to serve the next client.
 * ─────────────────────────────────────────────────────────────────── */

/* ── Main ── */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    /* Init Winsock */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    /* Init client table */
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].active = 0;
        clients[i].fd = INVALID_SOCKET;
    }

    /* Create TCP socket */
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET)
    {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    /* Reuse address */
    BOOL opt = TRUE;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               (char *)&opt, sizeof(opt));

    /* Bind */
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((u_short)port);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    /* Listen */
    if (listen(server_fd, BACKLOG) == SOCKET_ERROR)
    {
        fprintf(stderr, "listen() failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("=== TCP Chat Server (Windows) on port %d ===\n", port);
    printf("Waiting for connections...\n\n");

    /* ── Iterative accept loop ── */
    while (1)
    {
        struct sockaddr_in client_addr = {0};
        int client_len = sizeof(client_addr);

        /* ============================================================
         * STEP 1 — ACCEPT REQUEST
         * Blocks here until a client performs the TCP 3-way handshake.
         * Returns a new dedicated socket (client_fd) for this client.
         * The listening server_fd stays open for future connections.
         * ============================================================ */
        SOCKET client_fd = accept(server_fd,
                                  (struct sockaddr *)&client_addr,
                                  &client_len);
        if (client_fd == INVALID_SOCKET)
        {
            fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
            continue;
        }

        /* Steps 2-5 all happen inside handle_client().
           The next accept() is not called until handle_client() returns,
           which is the defining trait of an ITERATIVE server. */
        handle_client(client_fd, &client_addr);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}