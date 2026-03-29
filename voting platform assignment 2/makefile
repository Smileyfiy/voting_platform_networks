# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra

# Executables
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = servervote.c
CLIENT_SRC = clientvote.c

# Default target
all: $(SERVER) $(CLIENT)

# Build server
$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER)

# Build client
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT)

# Clean build files
clean:
	rm -f $(SERVER) $(CLIENT) *.o

# Rebuild everything
rebuild: clean all