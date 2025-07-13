# Message Queue OOB Exchange Demo

A demonstration of POSIX message queues for structured out-of-band (OOB) data exchange in RDMA applications. This project shows how message queues provide reliable, structured communication with built-in message boundaries and priority support.

## Project Structure

```
message-queue-demo/
├── include/
│   └── mq_common.h         # Shared header with structures and functions
├── src/
│   ├── mq_common.c         # Message queue utility functions
│   ├── mq_server.c         # Server application
│   └── mq_client.c         # Client application
├── Makefile                # Build configuration
└── README.md               # This file
```

## Features

- **Structured Messaging**: Fixed-size messages with type identification
- **Message Boundaries**: Automatic message framing and validation
- **Priority Support**: Built-in message priority queuing
- **Kernel Buffering**: System-managed message queues with overflow protection
- **POSIX Compliance**: Standard interface across Unix-like systems
- **Bidirectional Queues**: Separate server/client message queues

## Building

```bash
# Build both server and client
make

# Build individual targets
make server
make client

# Clean build files and message queues
make clean

# Build and run demo
make test

# Run performance benchmark
make benchmark

# Check system limits
make limits
```

## Usage

### Method 1: Automatic Test
```bash
make test
```

### Method 2: Manual Execution

**Terminal 1 (Server):**
```bash
./bin/mq_server
```

**Terminal 2 (Client):**
```bash
./bin/mq_client
```

## What It Demonstrates

### 1. RDMA Connection Data Exchange
Exchanges structured RDMA connection metadata:
- **Buffer Addresses**: 64-bit device virtual addresses
- **Remote Keys**: Memory region access authorization
- **Queue Pair Numbers**: RDMA communication endpoints
- **LID/GID**: InfiniBand network addressing

### 2. Message Queue Architecture
- **Separate Queues**: `/rdma_oob_server_mq` and `/rdma_oob_client_mq`
- **Message Structure**: Type + Sequence + Data + Padding
- **Size Validation**: Enforced message size limits
- **Atomic Operations**: Complete message delivery or failure

### 3. Message Types
```c
typedef enum {
    MSG_CONNECTION_DATA = 1,
    MSG_ACK = 2, 
    MSG_ERROR = 3
} message_type_t;
```

### 4. Queue Characteristics
- **Max Messages**: 10 per queue
- **Message Size**: 1024 bytes (configurable)
- **Delivery**: FIFO with optional priority
- **Persistence**: Survives process crashes

## Sample Output

**Server:**
```
Message Queue OOB Server Starting...
Server queue: /rdma_oob_server_mq
Client queue: /rdma_oob_client_mq
Server message queue created successfully
Waiting for client to create its queue...
Connected to client message queue

Preparing server connection data:
  Address: 0x1001001800000000
  Remote Key: 0x12345678
  QP Number: 1001
  LID: 1
  GID: 0102:0304:0506:0708:090a:0b0c:0d0e:0f10

Sending server connection data to client...
Waiting for client connection data...

Received client connection data (sequence: 2001):
  Address: 0x2002002900000000
  Remote Key: 0x87654321
  QP Number: 2001
  LID: 2
  GID: 1112:1314:1516:1718:191a:1b1c:1d1e:1f20

Connection data exchange completed in 1.234 ms

OOB exchange completed successfully!
Message Queue Benefits:
1. Structured message delivery with priorities
2. Built-in message boundaries and ordering
3. Kernel-managed buffering and flow control
4. POSIX standard interface across platforms

Demonstrating additional message capabilities...
Sent additional QP data (QP: 1101)
Received client QP data (QP: 2101, seq: 2002)
Sent additional QP data (QP: 1102)
Received client QP data (QP: 2102, seq: 2003)
```

**Client:**
```
Message Queue OOB Client Starting...
Server queue: /rdma_oob_server_mq
Client queue: /rdma_oob_client_mq
Waiting for server queue to be available...
Connected to server and created client message queue

Preparing client connection data:
[Connection data details...]

Waiting for server connection data...
Sending client connection data to server...

Received server connection data (sequence: 1001):
[Server connection data...]

Connection data exchange completed in 1.234 ms

OOB exchange completed successfully!
Message Queue Advantages:
1. Message boundaries preserved automatically
2. Priority-based message delivery support
3. Blocking/non-blocking receive options
4. Built-in message size validation

Participating in additional message exchange...
Received server QP data (QP: 1101, seq: 1002)
Sent client QP data (QP: 2101)
Received server QP data (QP: 1102, seq: 1003)
Sent client QP data (QP: 2102)
```

## Performance Comparison

| Method | Latency | Structure | Ordering | Reliability | Use Case |
|--------|---------|-----------|----------|-------------|-----------|
| **Shared Memory** | **~10ns** | Manual | Manual | Manual | **Fastest same-host** |
| **Unix Sockets** | **~1-5μs** | Stream | Ordered | Reliable | **General same-host** |
| **Message Queues** | **~5-15μs** | **Built-in** | **FIFO/Priority** | **Reliable** | **Structured exchange** |
| **TCP Loopback** | ~10-50μs | Stream | Ordered | Reliable | Network-compatible |

### Message Queue Advantages:
- ✅ **Structured Communication**: Built-in message framing
- ✅ **Priority Queuing**: Important messages can jump ahead
- ✅ **Size Validation**: Automatic message size checking
- ✅ **Persistence**: Messages survive process restarts
- ✅ **Flow Control**: Kernel manages queue overflow

### Message Queue Disadvantages:
- ❌ **Higher Latency**: More overhead than shared memory/sockets
- ❌ **Size Limits**: Fixed maximum message and queue sizes
- ❌ **System Resources**: Limited number of queues per process
- ❌ **Platform Setup**: Requires mqueue filesystem mount

## When to Use Message Queues for RDMA OOB

### Ideal Scenarios:
- **Complex Handshakes**: Multiple message types and sequences
- **Priority Requirements**: Critical connection data needs priority
- **Reliable Delivery**: Cannot tolerate message loss
- **Structured Data**: Well-defined message formats
- **Multiple QPs**: Need to exchange many connection endpoints

### Not Ideal For:
- **Simple Exchange**: Single connection data exchange
- **Lowest Latency**: Where every microsecond matters
- **High Frequency**: Very frequent connection establishments
- **Resource Constrained**: Limited system message queue resources

## Integration with RDMA Applications

```c
// Example integration pattern
typedef enum {
    OOB_SHARED_MEMORY,    // Fastest, same-host only
    OOB_UNIX_SOCKET,      // Fast, same-host, stream-based
    OOB_MESSAGE_QUEUE,    // Structured, reliable, priority
    OOB_TCP_SOCKET        // Network-compatible fallback
} oob_method_t;

oob_method_t select_oob_method(connection_requirements_t *req) {
    if (req->same_host && req->need_structure && req->multiple_qps) {
        return OOB_MESSAGE_QUEUE;
    } else if (req->same_host && req->need_speed) {
        return OOB_SHARED_MEMORY;
    } else if (req->same_host) {
        return OOB_UNIX_SOCKET;
    } else {
        return OOB_TCP_SOCKET;
    }
}
```

## System Requirements

### Message Queue Filesystem
```bash
# Check if mqueue is mounted
mount | grep mqueue

# Mount if necessary (usually automatic)
sudo mount -t mqueue none /dev/mqueue
```

### System Limits
```bash
# Check current limits
cat /proc/sys/fs/mqueue/queues_max     # Max queues per process
cat /proc/sys/fs/mqueue/msgsize_max    # Max message size
cat /proc/sys/fs/mqueue/msg_max        # Max messages per queue

# Adjust if needed (as root)
echo 256 > /proc/sys/fs/mqueue/queues_max
```

## Dependencies

- **POSIX-compliant system** (Linux, macOS, BSD)
- **RT library**: Link with `-lrt` for message queues
- **Mounted mqueue filesystem**: Usually automatic on modern systems
- **C99 compiler**: gcc, clang with C99 support

## Troubleshooting

### Queue Creation Errors
```bash
# Check available space
df -h /dev/mqueue

# Clean up stale queues
ls /dev/mqueue/
rm /dev/mqueue/rdma_oob_*
```

### Permission Issues
```bash
# Check queue permissions
ls -la /dev/mqueue/rdma_oob_*

# Verify process limits
ulimit -q    # POSIX message queue size
```

### Performance Optimization
```bash
# Reduce message size for better performance
# Increase queue depth for burst handling
# Use non-blocking operations where appropriate
```

## License

This is a demonstration project for educational and development purposes.