/*
 * UDP Chat Server - Kali Linux
 * Iterative Connectionless Chat Application
 *
 * Compile: gcc server.c -o server
 * Run:     ./server <port>
 * Example: ./server 9000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

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

/* Broadcast a message to every client except the sender */
static void broadcast(int sock, Client *clients, int n,
                      const struct sockaddr_in *sender,
                      const char *msg)
{
    for (int i = 0; i < n; i++)
    {
        if (!clients[i].active)
            continue;
        if (sender && addr_equal(&clients[i].addr, sender))
            continue;
        sendto(sock, msg, strlen(msg), 0,
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
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char ts[16], out[BUFFER_SIZE + 64];

    Client clients[MAX_CLIENTS];
    memset(clients, 0, sizeof(clients));

    /* Create UDP socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Allow address reuse */
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Bind */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("=== UDP Chat Server started on port %d ===\n", port);
    printf("Waiting for clients...\n\n");

    /* ── Iterative server loop ── */
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        /* ============================================================
         * STEP 1 — ACCEPT REQUEST
         * UDP has no connections, so there is no accept() call.
         * recvfrom() plays this role instead — it blocks until any
         * datagram arrives on the socket, then returns the data and
         * the sender's address (IP + port) in client_addr.
         * Every iteration of this loop is one full request cycle.
         * ============================================================ */
        int n = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&client_addr, &client_len);
        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';
        timestamp(ts, sizeof(ts));

        /* ============================================================
         * STEP 2 — PROCESS REQUEST
         * Identify who sent this datagram by looking up their address
         * in the client table. Then parse the command in buffer[] and
         * decide what action to take and what reply to prepare.
         * ============================================================ */
        int idx = find_client(clients, MAX_CLIENTS, &client_addr);

        /* ── REGISTER command: "REGISTER <username>" ── */
        if (strncmp(buffer, "REGISTER ", 9) == 0)
        {
            char *uname = buffer + 9;
            /* Trim newline */
            uname[strcspn(uname, "\r\n")] = '\0';

            if (idx >= 0)
            {
                /* Already registered – just update username */
                strncpy(clients[idx].username, uname, USERNAME_LEN - 1);

                /* ====================================================
                 * STEP 3 — FORMULATE REPLY  (REGISTER / update)
                 * ==================================================== */
                /* STEP 4 — SEND REPLY */
                sendto(sock, "SERVER: Username updated.\n",
                       26, 0, (struct sockaddr *)&client_addr, client_len);
            }
            else
            {
                int slot = free_slot(clients, MAX_CLIENTS);
                if (slot < 0)
                {
                    /* STEP 3 — FORMULATE REPLY  (room full) */
                    /* STEP 4 — SEND REPLY */
                    sendto(sock, "SERVER: Chat room is full. Try later.\n",
                           38, 0, (struct sockaddr *)&client_addr, client_len);
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

                    /* ====================================================
                     * STEP 3 — FORMULATE REPLY  (REGISTER welcome)
                     * Build welcome for sender and join notice for others.
                     * ==================================================== */
                    /* STEP 4 — SEND REPLY (welcome to new client) */
                    sendto(sock, "SERVER: Welcome to the chat!\n",
                           29, 0, (struct sockaddr *)&client_addr, client_len);

                    /* STEP 4 — SEND REPLY (broadcast join notice) */
                    snprintf(out, sizeof(out),
                             "SERVER: %s has joined the chat.\n", uname);
                    broadcast(sock, clients, MAX_CLIENTS, &client_addr, out);
                }
            }

            /* ============================================================
             * STEP 5 — EXIT (this request cycle)
             * Unlike TCP, there is no connection to close. "Exit" in UDP
             * simply means finishing this datagram's handling and looping
             * back to recvfrom() (STEP 1) to wait for the next datagram.
             * ============================================================ */
            continue;
        }

        /* ── QUIT command ── */
        if (strncmp(buffer, "QUIT", 4) == 0 && idx >= 0)
        {
            printf("[%s] - %s left.\n", ts, clients[idx].username);

            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (QUIT)
             * Build goodbye for sender and departure notice for others.
             * ============================================================ */
            snprintf(out, sizeof(out),
                     "SERVER: %s has left the chat.\n", clients[idx].username);

            /* STEP 4 — SEND REPLY (goodbye to departing client) */
            sendto(sock, "SERVER: Goodbye!\n", 17, 0,
                   (struct sockaddr *)&client_addr, client_len);

            /* ============================================================
             * STEP 5 — EXIT (this request cycle)
             * Deactivate the client record — their address is forgotten.
             * Then broadcast the departure notice and loop back to STEP 1.
             * ============================================================ */
            clients[idx].active = 0;
            broadcast(sock, clients, MAX_CLIENTS, &client_addr, out);
            continue;
        }

        /* ── LIST command: show online users ── */
        if (strncmp(buffer, "LIST", 4) == 0)
        {
            if (idx < 0)
            {
                sendto(sock,
                       "SERVER: Please register first. Send: REGISTER <username>\n",
                       57, 0, (struct sockaddr *)&client_addr, client_len);
                continue;
            }

            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (LIST)
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
            sendto(sock, list, strlen(list), 0,
                   (struct sockaddr *)&client_addr, client_len);

            /* STEP 5 — EXIT (this request cycle): loop back to STEP 1 */
            continue;
        }

        /* ── Private message: "@username <message>" ── */
        if (buffer[0] == '@' && idx >= 0)
        {
            char target[USERNAME_LEN] = {0};
            char *space = strchr(buffer + 1, ' ');
            if (!space)
            {
                sendto(sock, "SERVER: Usage: @username <message>\n", 35, 0,
                       (struct sockaddr *)&client_addr, client_len);
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
                if (strcasecmp(clients[i].username, target) == 0)
                {
                    target_idx = i;
                    break;
                }
            }

            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (private message)
             * Build the PM datagram for target and confirmation for sender.
             * ============================================================ */
            if (target_idx < 0)
            {
                snprintf(out, sizeof(out),
                         "SERVER: User '%s' not found or not online.\n", target);
                /* STEP 4 — SEND REPLY */
                sendto(sock, out, strlen(out), 0,
                       (struct sockaddr *)&client_addr, client_len);
            }
            else if (target_idx == idx)
            {
                /* STEP 4 — SEND REPLY */
                sendto(sock, "SERVER: You cannot message yourself.\n", 37, 0,
                       (struct sockaddr *)&client_addr, client_len);
            }
            else
            {
                snprintf(out, sizeof(out), "[%s] [PM from %s]: %s\n",
                         ts, clients[idx].username, pm_text);
                /* STEP 4 — SEND REPLY (deliver to target) */
                sendto(sock, out, strlen(out), 0,
                       (struct sockaddr *)&clients[target_idx].addr,
                       sizeof(clients[target_idx].addr));

                snprintf(out, sizeof(out), "[%s] [PM to %s]: %s\n",
                         ts, target, pm_text);
                /* STEP 4 — SEND REPLY (confirm to sender) */
                sendto(sock, out, strlen(out), 0,
                       (struct sockaddr *)&client_addr, client_len);

                printf("[%s] PM: %s -> %s: %s\n",
                       ts, clients[idx].username, target, pm_text);
            }

            /* STEP 5 — EXIT (this request cycle): loop back to STEP 1 */
            continue;
        }

        /* ── Regular broadcast message ── */
        if (idx >= 0)
        {
            /* ============================================================
             * STEP 3 — FORMULATE REPLY  (broadcast)
             * Format the message with timestamp and sender's username.
             * ============================================================ */
            printf("[%s] %s: %s", ts, clients[idx].username, buffer);
            snprintf(out, sizeof(out), "[%s] %s: %s",
                     ts, clients[idx].username, buffer);

            /* STEP 4 — SEND REPLY (echo back to sender) */
            sendto(sock, out, strlen(out), 0,
                   (struct sockaddr *)&client_addr, client_len);
            /* STEP 4 — SEND REPLY (broadcast to everyone else) */
            broadcast(sock, clients, MAX_CLIENTS, &client_addr, out);
        }
        else
        {
            /* Unregistered client — send error, no state change */
            sendto(sock,
                   "SERVER: Please register first. Send: REGISTER <username>\n",
                   57, 0, (struct sockaddr *)&client_addr, client_len);
        }

        /* STEP 5 — EXIT (this request cycle): loop back to STEP 1 */
    }

    close(sock);
    return 0;
}