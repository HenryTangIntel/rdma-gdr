CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -Iinclude
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Link with pthread for semaphores and rt for shared memory
LDFLAGS = -lpthread -lrt

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

SERVER_OBJ = $(OBJDIR)/shm_server.o $(OBJDIR)/shm_common.o
CLIENT_OBJ = $(OBJDIR)/shm_client.o $(OBJDIR)/shm_common.o

SERVER_TARGET = $(BINDIR)/shm_server
CLIENT_TARGET = $(BINDIR)/shm_client

.PHONY: all clean server client dirs

all: dirs $(SERVER_TARGET) $(CLIENT_TARGET)

dirs:
	@mkdir -p $(OBJDIR) $(BINDIR)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(SERVER_OBJ) $(LDFLAGS) -o $@

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CLIENT_OBJ) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

server: dirs $(SERVER_TARGET)

client: dirs $(CLIENT_TARGET)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Cleaning up shared memory and semaphores..."
	@-rm -f /dev/shm/rdma_oob_shm 2>/dev/null || true
	@-rm -f /dev/shm/sem.rdma_server_ready 2>/dev/null || true
	@-rm -f /dev/shm/sem.rdma_client_ready 2>/dev/null || true
	@-rm -f /dev/shm/sem.rdma_data_ready 2>/dev/null || true

test: all
	@echo "Running Shared Memory OOB Demo..."
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
	@echo "  clean    - Remove build files and shared memory"
	@echo "  help     - Show this help"

install: all
	@echo "Binaries available in $(BINDIR)/"
	@echo "To run:"
	@echo "  Terminal 1: ./$(SERVER_TARGET)"
	@echo "  Terminal 2: ./$(CLIENT_TARGET)"

benchmark: all
	@echo "Running performance benchmark..."
	@echo "Shared Memory OOB Exchange:"
	@./$(SERVER_TARGET) &
	@sleep 0.5
	@./$(CLIENT_TARGET)
	@echo ""
	@echo "Compare with Unix Domain Socket version:"
	@echo "cd ../unix-domain-socket-demo && make test"