📄 README.md
# Network Voting System (Client-Server in C)

## Overview

This is a simple network-based voting system implemented in C using TCP sockets.

The system consists of:
- A server that processes votes
- A client that sends votes

Votes are cast for:
- Candidate A
- Candidate B

---

## Features

- Client-server communication using TCP
- Real-time vote counting
- Continuous voting session
- Input validation
- Cross-platform support (Linux & Windows)

---

## Project Structure


network-voting/
│
├── servervote.c
├── clientvote.c
├── Makefile
├── README.md


---

## Compilation

Compile server:


gcc servervote.c -o server


Compile client:


gcc clientvote.c -o client


---

## Running the Application

Start server:


./server


Start client (in another terminal):


./client


---

## Usage

- Enter `A` → Vote for Candidate A
- Enter `B` → Vote for Candidate B
- Enter `Q` → Quit and display final results

---

## Example


Enter Vote (A/B) or Q:
A
Server says: VOTE CAST: Candidate A (Total A: 1, B: 0)


---

## Limitations

- Single-threaded server
- No persistent storage
- No authentication

---

## Possible Improvements

- Multi-client concurrency (threads)
- Database storage
- GUI interface
- Secure communication

-