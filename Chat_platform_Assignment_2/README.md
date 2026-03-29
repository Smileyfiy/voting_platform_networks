# SCS3304
# Assignment 2: Connection-Oriented Iterative Server1. 
Project OverviewThis project demonstrates the implementation of a Connection-Oriented Iterative Server using the C programming language. The goal was to transition a standalone application (Assignment 1) into a networked environment where a client and server communicate over a stable TCP/IP connection on a single machine.
# 2. How it Works (Socket API)
The core of this application relies on the standard Socket API. Below are the primary functions used to establish the connection-oriented communication:
| Function | Purpose |
|----------|---------|
| `socket()` | Creates the socket descriptor (the "plug" for the network). |
| `bind()` | Hooks the program to a specific local port (e.g., 8080). |
| `listen()` | Puts the server in passive mode, waiting for clients to connect. |
| `accept()` | Pulls the first connection request off the queue and creates a new socket for that specific client. |
| `recv() / send()` | Used to exchange the actual application data between the client and server. |
# 3. Key Features
TCP/IP Socket Communication: Uses a stable, connection-oriented stream for reliable data transfer.

Iterative Design: Handles multiple sequential client requests by processing one at a time, preventing race conditions.

Cross-Platform Support: Fully compatible with both Windows (via ws2_32 library) and Linux systems using conditional preprocessor directives.

Modular "Conceptual Server" Algorithm: Code is strictly organized into stages: Accept Request, Process Request, Formulate Reply, and Send Reply.

Robust Error Handling: Includes checks for socket creation and connection failures to ensure system stability.

# 4. Technical Workflow
The communication follows a standard 4-step handshake:Handshake: The Client initiates a connection to the local loopback address 127.0.0.1 on Port 8080.

Request: The Client sends a data packet containing the application-specific instructions.

Process: The Server receives the packet, performs the required calculation/logic, and generates a response string.

Close: Once the reply is sent, the server closes the specific connection socket but keeps the master listening socket open for the next client.

# 5. How to RunWindows (Command Prompt / PowerShell)

To run this on Windows, you must link the Windows Socket library using the -lws2_32 flag during compilation.Compile Server:Bashgcc server2.c -o server2 -lws2_32


 # Compile Client: 
Bashgcc client2.c -o client2 -lws2_32
  # Execution:
Start the server first: ./server2
In a separate terminal window, start the client: ./client2

# Linux / WSL
  # Compile:
Bashgcc server2.c -o server2
gcc client2.c -o client2

# 6. File Structure

server2.c: The main iterative server logic including WSAStartup for Windows compatibility.
client2.c: The client application used to initiate requests and display server responses.
README.md: Project documentation and execution guide.