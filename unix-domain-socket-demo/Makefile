CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -Iinclude
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

SERVER_OBJ = $(OBJDIR)/uds_server.o $(OBJDIR)/uds_common.o
CLIENT_OBJ = $(OBJDIR)/uds_client.o $(OBJDIR)/uds_common.o

SERVER_TARGET = $(BINDIR)/uds_server
CLIENT_TARGET = $(BINDIR)/uds_client

.PHONY: all clean server client dirs

all: dirs $(SERVER_TARGET) $(CLIENT_TARGET)

dirs:
	@mkdir -p $(OBJDIR) $(BINDIR)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(SERVER_OBJ) -o $@

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CLIENT_OBJ) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

server: dirs $(SERVER_TARGET)

client: dirs $(CLIENT_TARGET)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	rm -f /tmp/uds_demo_socket

test: all
	@echo "Running Unix Domain Socket Demo..."
	@echo "Starting server in background..."
	@./$(SERVER_TARGET) &
	@sleep 1
	@echo "Starting client..."
	@./$(CLIENT_TARGET)
	@echo "Demo completed."

help:
	@echo "Available targets:"
	@echo "  all      - Build both server and client"
	@echo "  server   - Build only server"
	@echo "  client   - Build only client" 
	@echo "  test     - Build and run demo"
	@echo "  clean    - Remove build files and socket"
	@echo "  help     - Show this help"

install: all
	@echo "Binaries available in $(BINDIR)/"
	@echo "To run:"
	@echo "  Terminal 1: ./$(SERVER_TARGET)"
	@echo "  Terminal 2: ./$(CLIENT_TARGET)"