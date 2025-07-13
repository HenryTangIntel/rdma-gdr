CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -Iinclude
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Link with rt for message queues
LDFLAGS = -lrt

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

SERVER_OBJ = $(OBJDIR)/mq_server.o $(OBJDIR)/mq_common.o
CLIENT_OBJ = $(OBJDIR)/mq_client.o $(OBJDIR)/mq_common.o

SERVER_TARGET = $(BINDIR)/mq_server
CLIENT_TARGET = $(BINDIR)/mq_client

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
	@echo "Cleaning up message queues..."
	@-rm -f /dev/mqueue/rdma_oob_server_mq 2>/dev/null || true
	@-rm -f /dev/mqueue/rdma_oob_client_mq 2>/dev/null || true

test: all
	@echo "Running Message Queue OOB Demo..."
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
	@echo "  clean    - Remove build files and message queues"
	@echo "  help     - Show this help"
	@echo "  limits   - Show system message queue limits"

install: all
	@echo "Binaries available in $(BINDIR)/"
	@echo "To run:"
	@echo "  Terminal 1: ./$(SERVER_TARGET)"
	@echo "  Terminal 2: ./$(CLIENT_TARGET)"

benchmark: all
	@echo "Running performance benchmark..."
	@echo "Message Queue OOB Exchange:"
	@./$(SERVER_TARGET) &
	@sleep 0.5
	@./$(CLIENT_TARGET)
	@echo ""
	@echo "Compare with other IPC methods:"
	@echo "Unix Domain Sockets: cd ../unix-domain-socket-demo && make test"
	@echo "Shared Memory:       cd ../shared-memory-demo && make test"

limits:
	@echo "System Message Queue Limits:"
	@echo "Max queues per process: $(shell cat /proc/sys/fs/mqueue/queues_max 2>/dev/null || echo 'N/A')"
	@echo "Max message size:       $(shell cat /proc/sys/fs/mqueue/msgsize_max 2>/dev/null || echo 'N/A') bytes"
	@echo "Max messages per queue: $(shell cat /proc/sys/fs/mqueue/msg_max 2>/dev/null || echo 'N/A')"
	@echo "Message queue filesystem mounted: $(shell mount | grep mqueue || echo 'Not mounted')"