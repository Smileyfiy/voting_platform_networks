# DESIGN DOCUMENT

# Network Voting System — System Design Document

> **Course:** Network & Distributed Programming
> **Date:** March 2026
> **Language:** C (C11 Standard)
> **Platform:** Linux / Windows (Socket Programming)

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

```id="net1"
┌──────────────────────┐
│      Client App      │
│    clientvote.c      │
│  (User Interface)    │
└──────────┬───────────┘
           │ TCP Socket (Port 8081)
           ▼
┌──────────────────────┐
│      Server App      │
│    servervote.c      │
│   (Vote Processing)  │
└──────────────────────┘
```

---

## 1.2 Architecture Description

* **Client**

  * Sends votes (A or B) to server
  * Displays server responses

* **Server**

  * Listens for connections
  * Processes votes
  * Maintains vote tally
  * Sends results back

---

## 1.3 Key Design Principles

| Principle           | Application                 |
| ------------------- | --------------------------- |
| Client-Server Model | Separation of UI and logic  |
| TCP Communication   | Reliable data transfer      |
| Modular Design      | Client and server separated |
| Continuous Service  | Server runs indefinitely    |
| Simplicity          | Lightweight implementation  |

---

# 2. Module Outline

## 2.1 Module Summary

| Module        | File         | Responsibility                          |
| ------------- | ------------ | --------------------------------------- |
| Client Module | clientvote.c | Sends votes and receives responses      |
| Server Module | servervote.c | Handles connections and vote processing |

---

## 2.2 Module Dependency

```id="net2"
clientvote.c  <── TCP ──>  servervote.c
```

---

## 2.3 Module Descriptions

### Client Module (clientvote.c)

Responsibilities:

* Establish connection to server
* Accept user input (A/B/Q)
* Send vote to server
* Display server response

---

### Server Module (servervote.c)

Responsibilities:

* Create socket and bind to port
* Listen for incoming connections
* Accept client connections
* Process votes
* Maintain vote count
* Send response to clients

---

# 3. Process Design

## 3.1 System Workflow

```id="net3"
Start Server
     │
Wait for Client Connection
     │
Client Connects
     │
Client Sends Vote (A/B)
     │
Server Processes Vote
     │
Server Updates Count
     │
Server Sends Response
     │
Repeat until Q is sent
```

---

## 3.2 Client Process Flow

```id="net4"
Start Client
     │
Connect to Server
     │
Display Input Prompt
     │
User Inputs Vote
     │
Send Vote to Server
     │
Receive Response
     │
Display Response
     │
Repeat until Q
```

---

## 3.3 Server Process Flow

```id="net5"
Start Server
     │
Create Socket
     │
Bind to Port 8081
     │
Listen for Clients
     │
Accept Connection
     │
Receive Vote
     │
Process Vote
     │
Send Response
     │
Repeat
```

---

# 4. Algorithm Design

## 4.1 Server Algorithm

```
START

Initialize vote counters (A = 0, B = 0)

Create socket
Bind socket to port
Listen for connections

WHILE true
    Accept client connection

    WHILE connection active
        Receive message

        IF message == 'A'
            Increment A
            Send response
        ELSE IF message == 'B'
            Increment B
            Send response
        ELSE IF message == 'Q'
            Send final tally
            BREAK
        ELSE
            Send error message
    END WHILE

    Close connection
END WHILE

END
```

---

## 4.2 Client Algorithm

```
START

Create socket
Connect to server

WHILE true
    Prompt user for input

    IF input == 'Q'
        Send 'Q'
        Receive response
        BREAK
    ELSE
        Send vote
        Receive response
        Display response
END WHILE

Close connection

END
```

---

# 5. Data/File Design

## 5.1 Data Storage

This system uses **in-memory storage only**.

Variables:

```c
int candidate_A;
int candidate_B;
```

---

## 5.2 Data Format

Votes are transmitted as:

```
"A" → Vote for Candidate A  
"B" → Vote for Candidate B  
"Q" → Quit and request final tally
```

---

## 5.3 Communication Format

Example:

```
Client → Server: A
Server → Client: VOTE CAST: Candidate A (Total A: 1, B: 0)
```

---

# 6. Concurrency Design

## 6.1 Concurrency Requirement

The server supports **multiple client connections sequentially**, not fully parallel.

---

## 6.2 Current Design

* Single-threaded server
* Handles one client session at a time
* Uses loop for continuous service

---

## 6.3 Limitations

* No multithreading
* No parallel client handling
* Shared counters not protected

---

## 6.4 Possible Improvement

* Use threads (`pthread`)
* Add mutex for vote counters
* Handle multiple clients concurrently

---

# 7. Implementation

## 7.1 Project Structure

```id="net6"
network-voting/
│
├── servervote.c
├── clientvote.c
├── Makefile
├── README.md
```

---

## 7.2 Compilation

Compile server:

```bash
gcc servervote.c -o server
```

Compile client:

```bash
gcc clientvote.c -o client
```

---

## 7.3 Execution

Run server:

```bash
./server
```

Run client:

```bash
./client
```

---

## 7.4 Key Features

* TCP socket communication
* Continuous server operation
* Real-time vote tally
* Cross-platform (Linux/Windows)

---

# 8. Testing Plan / Report

## 8.1 Testing Strategy

| Type                | Description                                  |
| ------------------- | -------------------------------------------- |
| Unit Testing        | Test client and server individually          |
| Integration Testing | Test communication between client and server |
| System Testing      | Test full voting workflow                    |

---

## 8.2 Test Cases

### Test Case 1 — Server Startup

| Input      | Expected Output          |
| ---------- | ------------------------ |
| Run server | Server listening on port |

---

### Test Case 2 — Client Connection

| Input      | Expected Output       |
| ---------- | --------------------- |
| Run client | Connection successful |

---

### Test Case 3 — Valid Vote

| Input | Expected Output |
| ----- | --------------- |
| A     | Vote recorded   |

---

### Test Case 4 — Invalid Vote

| Input | Expected Output |
| ----- | --------------- |
| X     | Error message   |

---

### Test Case 5 — Quit

| Input | Expected Output       |
| ----- | --------------------- |
| Q     | Final tally displayed |

---

## 8.3 Test Summary

| Category          | Status |
| ----------------- | ------ |
| Unit Tests        | PASS   |
| Integration Tests | PASS   |
| System Tests      | PASS   |

---

*End of Design Document*

