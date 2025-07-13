# Shared Memory OOB Exchange Demo

A high-performance demonstration of POSIX shared memory for out-of-band (OOB) data exchange in RDMA applications. This project shows how shared memory can provide the fastest possible local inter-process communication for same-host scenarios.

## Project Structure

```
shared-memory-demo/
├── include/
│   └── shm_common.h        # Shared header with structures and functions
├── src/
│   ├── shm_common.c        # Shared memory utility functions
│   ├── shm_server.c        # Server application
│   └── shm_client.c        # Client application
├── Makefile                # Build configuration
└── README.md               # This file
```

## Features

- **Zero-Copy Communication**: Direct memory access between processes
- **POSIX Shared Memory**: Uses shm_open/mmap for memory sharing
- **Semaphore Synchronization**: POSIX semaphores for coordination
- **RDMA Connection Exchange**: Simulates real RDMA connection parameter exchange
- **Performance Measurement**: Nanosecond-precision timing
- **Robust Cleanup**: Automatic resource cleanup on exit

## Building

```bash
# Build both server and client
make

# Build individual targets
make server
make client

# Clean build files and shared resources
make clean

# Build and run demo
make test

# Run performance benchmark
make benchmark
```

## Usage

### Method 1: Automatic Test
```bash
make test
```

### Method 2: Manual Execution

**Terminal 1 (Server):**
```bash
./bin/shm_server
```

**Terminal 2 (Client):**
```bash
./bin/shm_client
```

## What It Demonstrates

### 1. RDMA OOB Data Exchange
Exchanges the same connection metadata as real RDMA applications:
- **Buffer Addresses**: 64-bit device virtual addresses
- **Remote Keys**: Memory region access keys
- **Queue Pair Numbers**: RDMA communication endpoints
- **LID/GID**: InfiniBand addressing information

### 2. Shared Memory Architecture
- **Shared Memory Object**: `/rdma_oob_shm` - contains connection data
- **Synchronization Semaphores**:
  - `/rdma_server_ready` - Server data available
  - `/rdma_client_ready` - Client data available
  - `/rdma_data_ready` - Exchange complete
- **Memory Layout**: Single shared structure with server/client data

### 3. Performance Benefits
- **Latency**: ~10ns access time (fastest possible IPC)
- **Throughput**: Memory bandwidth limited
- **CPU Overhead**: Minimal kernel involvement
- **Scalability**: No socket file descriptors consumed

## Sample Output

**Server:**
```
Shared Memory OOB Server Starting...
Shared memory name: /rdma_oob_shm
Shared memory and semaphores created successfully

Preparing server connection data:
  Address: 0x1001001800000000
  Remote Key: 0x12345678
  QP Number: 1001
  LID: 1
  GID: 0102:0304:0506:0708:090a:0b0c:0d0e:0f10

Server data written to shared memory, waiting for client...
Client is ready, reading client data...

Received client connection data:
  Address: 0x2002002900000000
  Remote Key: 0x87654321
  QP Number: 2001
  LID: 2
  GID: 1112:1314:1516:1718:191a:1b1c:1d1e:1f20

Connection data exchange completed in 0.023 ms

OOB exchange completed successfully!
In a real RDMA application, you would now:
1. Use this data to establish RDMA QP connections
2. Transition QP states: INIT -> RTR -> RTS
3. Begin high-performance RDMA operations
```

**Client:**
```
Shared Memory OOB Client Starting...
Connecting to shared memory: /rdma_oob_shm
Connected to shared memory successfully

Preparing client connection data:
[Connection data details...]

Waiting for server data...

Received server connection data:
[Server connection data...]

Connection data exchange completed in 0.023 ms

OOB exchange completed successfully!
[Integration instructions...]
```

## Performance Comparison

| Method | Latency | CPU Overhead | Memory Usage | Use Case |
|--------|---------|--------------|--------------|-----------|
| **Shared Memory** | **~10ns** | **Minimal** | **Lowest** | **Same-host only** |
| Unix Domain Socket | ~1-5μs | Low | Low | Same-host preferred |
| TCP Loopback | ~10-50μs | High | High | Network-compatible |

### When to Use Shared Memory OOB:
- ✅ **Same-host RDMA clusters**: Maximum performance required
- ✅ **High-frequency trading**: Every nanosecond matters
- ✅ **Real-time systems**: Deterministic low latency
- ✅ **Memory-constrained**: Minimal resource usage

### When NOT to Use:
- ❌ **Cross-network**: Shared memory is local-only
- ❌ **Simple applications**: Complexity not justified
- ❌ **Unknown deployment**: May need network fallback

## Integration with RDMA Applications

Replace TCP/UDS OOB exchange with shared memory:

```c
// 1. Host detection
if (is_same_host(client, server)) {
    // Use shared memory OOB
    exchange_via_shm(&local_data, &remote_data);
} else {
    // Fallback to TCP/UDS
    exchange_via_socket(&local_data, &remote_data);
}

// 2. Use exchanged data for RDMA setup
setup_rdma_qp(&remote_data);
```

## Technical Details

### Shared Memory Structure
```c
typedef struct {
    volatile int server_ready;
    volatile int client_ready;
    volatile int data_ready;
    connection_data_t server_data;
    connection_data_t client_data;
} shm_oob_data_t;
```

### Synchronization Flow
1. **Server**: Create shared memory → Write data → Signal ready
2. **Client**: Open shared memory → Wait for server → Read data
3. **Client**: Write own data → Signal ready
4. **Server**: Read client data → Exchange complete

### Error Handling
- **Resource cleanup**: Automatic shm_unlink() and sem_unlink()
- **Signal handling**: Graceful shutdown on SIGINT/SIGTERM
- **Permission errors**: Clear error messages for debugging
- **Race conditions**: Proper semaphore ordering

## Dependencies

- **POSIX-compliant system** (Linux, macOS, BSD)
- **RT library**: Link with `-lrt` for shared memory
- **Pthread library**: Link with `-lpthread` for semaphores
- **C99 compiler**: gcc, clang with C99 support

## Troubleshooting

### Permission Errors
```bash
# Check shared memory permissions
ls -la /dev/shm/rdma_*

# Manual cleanup if needed
rm -f /dev/shm/rdma_oob_shm
rm -f /dev/shm/sem.rdma_*
```

### Resource Limits
```bash
# Check system limits
cat /proc/sys/kernel/shmmni    # Max shared memory segments
cat /proc/sys/kernel/shmmax    # Max shared memory size
```

## License

This is a demonstration project for educational and development purposes.