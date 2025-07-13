# Unix Domain Socket Demo

A simple demonstration of Unix domain sockets for high-performance inter-process communication on the same host. This project shows how Unix domain sockets can replace TCP/IP for local communication, providing lower latency and higher throughput.

## Project Structure

```
unix-domain-socket-demo/
├── include/
│   └── uds_common.h        # Shared header with structures and functions
├── src/
│   ├── uds_common.c        # Common utility functions
│   ├── uds_server.c        # Server application
│   └── uds_client.c        # Client application
├── Makefile                # Build configuration
└── README.md               # This file
```

## Features

- **Connection Data Exchange**: Simulates RDMA-style connection parameter exchange
- **Message Exchange**: Bidirectional communication with timing measurements
- **Error Handling**: Robust error handling and cleanup
- **Performance Metrics**: Latency measurements for operations
- **Signal Handling**: Graceful shutdown on SIGINT/SIGTERM

## Building

```bash
# Build both server and client
make

# Build individual targets
make server
make client

# Clean build files
make clean

# Build and run demo
make test
```

## Usage

### Method 1: Automatic Test
```bash
make test
```

### Method 2: Manual Execution

**Terminal 1 (Server):**
```bash
./bin/uds_server
```

**Terminal 2 (Client):**
```bash
./bin/uds_client
```

## What It Demonstrates

### 1. Connection Data Exchange
Both applications exchange connection metadata similar to RDMA applications:
- Buffer addresses (64-bit device virtual addresses)
- Remote keys for memory access
- Queue pair numbers
- Local/Global IDs

### 2. Performance Measurement
The demo measures:
- Connection establishment time
- Message round-trip latency
- Data exchange overhead

### 3. Unix Domain Socket Benefits
- **No network stack**: Direct kernel communication
- **Lower latency**: Bypasses TCP/IP processing
- **Higher throughput**: No protocol overhead
- **Filesystem-based**: Uses `/tmp/uds_demo_socket` path

## Sample Output

**Server:**
```
Unix Domain Socket Server Starting...
Socket path: /tmp/uds_demo_socket
Server listening for connections...
Client connected!

Sending server connection data:
Connection Data:
  Address: 0x1001001800000000
  Remote Key: 0x12345678
  QP Number: 1001
  LID: 1
  GID: 0102:0304:0506:0708:090a:0b0c:0d0e:0f10

Received client connection data:
Connection Data:
  Address: 0x2002002900000000
  Remote Key: 0x87654321
  QP Number: 2001
  LID: 2
  GID: 1112:1314:1516:1718:191a:1b1c:1d1e:1f20

Connection data exchange completed in 0.234 ms
```

**Client:**
```
Unix Domain Socket Client Starting...
Connecting to: /tmp/uds_demo_socket
Connected to server!

Preparing client connection data:
[Connection data details...]

Connection data exchange completed in 0.234 ms

Starting message exchange...

Sent message 1:
Message ID: 1
Payload: Hello from client - message 1
Data Length: 18
Timestamp: 1642123456.123456789

Received response (roundtrip: 0.567 ms):
Message ID: 1001
Payload: Server response to message 1
Data Length: 16
Timestamp: 1642123456.124567890
```

## Comparison with TCP

### Unix Domain Sockets Advantages:
- **Latency**: 50-90% lower than TCP loopback
- **CPU Usage**: Reduced kernel processing overhead
- **Memory**: No network buffer allocations
- **Reliability**: Same reliability as TCP streams

### Use Cases:
- Database client-server communication
- Microservice inter-communication
- High-frequency trading systems
- Real-time data processing pipelines

## Integration with RDMA Applications

This demo shows how to replace TCP-based out-of-band exchanges in RDMA applications:

1. **Host Detection**: Check if client/server are on same host
2. **Socket Type Selection**: Use Unix domain sockets for local, TCP for remote
3. **Same API**: Minimal code changes required
4. **Fallback Support**: Graceful degradation to TCP when needed

## Error Handling

The implementation includes:
- **Connection retries**: Client retries connection with backoff
- **Signal handling**: Graceful shutdown on interruption
- **Resource cleanup**: Automatic socket file cleanup
- **Error reporting**: Detailed error messages with perror()

## Dependencies

- **C99 compiler** (gcc, clang)
- **POSIX-compliant system** (Linux, macOS, BSD)
- **Standard libraries**: socket, unistd, time

## License

This is a demonstration project for educational purposes.