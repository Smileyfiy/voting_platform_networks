# Compiler
CC = gcc

# Detect OS
ifeq ($(OS),Windows_NT)
    CFLAGS = -Wall -Wextra -lws2_32
    RM = del /Q
    EXE = .exe
else
    CFLAGS = -Wall -Wextra
    RM = rm -f
    EXE =
endif

# Targets
CLIENT = client$(EXE)
SERVER = server$(EXE)

# Source files
CLIENT_SRC = client2.c
SERVER_SRC = server2.c

# Default target
all: $(CLIENT) $(SERVER)

# Build client
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CLIENT_SRC) -o $(CLIENT) $(CFLAGS)

# Build server
$(SERVER): $(SERVER_SRC)
	$(CC) $(SERVER_SRC) -o $(SERVER) $(CFLAGS)

# Clean
clean:
	$(RM) $(CLIENT) $(SERVER) *.o

# Rebuild
rebuild: clean all