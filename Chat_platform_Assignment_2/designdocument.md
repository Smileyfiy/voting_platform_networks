
# Network Client-Server Communication System — Design Document

> **Course:** Network & Distributed Programming
> **Date:** March 2026
> **Language:** C (C11 Standard)
> **Platform:** Linux / Windows (Cross-Platform Socket Programming)

---

# Table of Contents

1. Architectural Design
2. Module Outline
3. Process Design
4. Algorithm Design
5. Data/File Design
6. Concurrency Design
7. Implementation
8. Testing Plan / Report

---

# 1. Architectural Design

## 1.1 Overall Architecture

The system follows a **Client-Server Architecture** using **TCP sockets**.

```
┌──────────────────────────┐
│        Client App        │
│        client.c          │
│  Sends request message   │
└─────────────┬────────────┘
              │ TCP (Port 8080)
              ▼
┌──────────────────────────┐
│        Server App        │
│        server.c          │
│ Receives & replies data  │
└──────────────────────────┘
```

---

## 1.2 Architecture Description

* The **client** initiates communication.
* The **server** listens on a specific port.
* Communication occurs over **TCP (Transmission Control Protocol)**.
* The server responds after receiving client data.

---

## 1.3 Key Design Principles

| Principle             | Application                      |
| --------------------- | -------------------------------- |
| Client-Server Model   | Clear separation of roles        |
| TCP Communication     | Reliable data transmission       |
| Cross-Platform Design | Works on Windows and Linux       |
| Simplicity            | Minimal implementation           |
| Modularity            | Separate client and server files |

---

# 2. Module Outline

## 2.1 Module Summary

| Module        | File     | Responsibility                      |
| ------------- | -------- | ----------------------------------- |
| Client Module | client.c | Sends request and receives response |
| Server Module | server.c | Handles connections and responses   |

---

## 2.2 Module Dependency

```
client.c  <──── TCP Socket ────>  server.c
```

---

## 2.3 Module Description

### Client Module (client.c)

Responsibilities:

* Initialize socket (Windows/Linux compatible)
* Connect to server
* Send message to server
* Receive response
* Close connection

---

### Server Module (server.c)

Responsibilities:

* Create and bind socket
* Listen for incoming connections
* Accept client connection
* Receive client message
* Send response back
* Close connection

---

# 3. Process Design

## 3.1 System Workflow

```
Start Server
     │
Wait for Client Connection
     │
Client Starts
     │
Client Connects
     │
Client Sends Message
     │
Server Receives Message
     │
Server Sends Reply
     │
Client Displays Response
     │
Connection Closed
```

---

## 3.2 Client Process Flow

```
Start Client
     │
Initialize Socket
     │
Connect to Server
     │
Send Message
     │
Receive Response
     │
Display Response
     │
Close Connection
```

---

## 3.3 Server Process Flow

```
Start Server
     │
Create Socket
     │
Bind to Port 8080
     │
Listen for Connections
     │
Accept Client
     │
Receive Message
     │
Send Response
     │
Close Connection
```

---

# 4. Algorithm Design

## 4.1 Client Algorithm

```
START

Initialize socket (WSAStartup for Windows)

Create socket

Set server address (127.0.0.1:8080)

Connect to server

Send message "Hello from Client"

Receive response from server

Display response

Close socket

Cleanup resources

END
```

---

## 4.2 Server Algorithm

```
START

Create socket

Bind socket to port 8080

Listen for incoming connections

Accept client connection

Receive message from client

Display received message

Send response to client

Close connection

END
```

---

# 5. Data/File Design

## 5.1 Data Handling

This system uses **in-memory data only**.

No files or persistent storage are used.

---

## 5.2 Data Format

Messages are simple strings.

Example:

```
Client → "Hello from Client"
Server → "Hello from Server"
```

---

## 5.3 Buffers Used

```c
char buffer[1024];
```

Used for:

* Receiving data
* Storing messages

---

# 6. Concurrency Design

## 6.1 Concurrency Requirement

Current implementation is **single-client, single-threaded**.

---

## 6.2 Design Characteristics

* Server handles **one client at a time**
* No threading or parallel processing
* Sequential request handling

---

## 6.3 Limitations

* Cannot handle multiple clients simultaneously
* No synchronization mechanisms

---

## 6.4 Future Improvements

* Use **multi-threading (pthread / Windows threads)**
* Implement **concurrent client handling**
* Add **non-blocking sockets**

---

# 7. Implementation

## 7.1 Project Structure

```
project/
│
├── client.c
├── server.c
├── Makefile
├── README.md
```

---

## 7.2 Compilation

Using Makefile:

```
make
```

Manual compilation:

```
gcc client.c -o client
gcc server.c -o server
```

---

## 7.3 Execution

Run server first:

```
./server
```

Run client:

```
./client
```

---

## 7.4 Key Features

* TCP socket communication
* Cross-platform compatibility
* Simple message exchange
* Modular design

---

# 8. Testing Plan / Report

## 8.1 Testing Strategy

| Test Type           | Purpose                           |
| ------------------- | --------------------------------- |
| Unit Testing        | Test client and server separately |
| Integration Testing | Test communication between them   |
| System Testing      | Test full workflow                |

---

## 8.2 Test Cases

### Test Case 1 — Server Start

| Input      | Expected Result             |
| ---------- | --------------------------- |
| Run server | Server listens on port 8080 |

---

### Test Case 2 — Client Connect

| Input      | Expected Result       |
| ---------- | --------------------- |
| Run client | Successful connection |

---

### Test Case 3 — Message Exchange

| Input                | Expected Result              |
| -------------------- | ---------------------------- |
| Client sends message | Server receives and responds |

---

### Test Case 4 — Invalid Server

| Input              | Expected Result   |
| ------------------ | ----------------- |
| Server not running | Connection failed |

---

## 8.3 Test Summary

| Category          | Status |
| ----------------- | ------ |
| Unit Tests        | PASS   |
| Integration Tests | PASS   |
| System Tests      | PASS   |

---

*End of Design Document*
