# RDMA Client-Server Demo

A clean client-server implementation demonstrating RDMA (Remote Direct Memory Access) communication using standard InfiniBand verbs.

## Overview

This project demonstrates core RDMA concepts including both two-sided (Send/Receive) and one-sided (RDMA Write) operations. The implementation uses standard host memory and InfiniBand verbs, making it portable across different RDMA-capable hardware.

## Features Demonstrated

✅ **Two-sided operations**: Send/Receive with coordination between client and server  
✅ **One-sided operations**: RDMA Write with zero software involvement on target side  
✅ **Memory registration**: Host memory registration with lkey/rkey management  
✅ **Connection management**: TCP handshake + QP state transitions  
✅ **Data verification**: Visible patterns to confirm successful data transfer

## System Architecture

The system demonstrates standard RDMA communication patterns:
- **Host Memory**: Standard aligned memory allocation and registration
- **InfiniBand Verbs**: Direct use of libibverbs for RDMA operations
- **Queue Pairs**: Reliable Connected (RC) transport for guaranteed delivery

## Communication Flow

### 1. Initialization Phase

Both client and server perform the following initialization steps:

```
1. Allocate aligned host memory buffer
2. Initialize InfiniBand device and resources
3. Register memory with RDMA permissions
4. Establish RDMA connection over TCP
5. Exchange connection parameters (addresses, keys, QP numbers)
```

### 2. Memory Management Flow

The memory setup process follows this sequence:

```c
init_rdma_buffer()
  ├─ aligned_alloc()       // Allocate page-aligned host memory
  └─ memset()              // Initialize buffer to zeros

init_rdma_resources()
  ├─ ibv_alloc_pd()        // Create protection domain
  ├─ ibv_create_cq()       // Create completion queue  
  ├─ ibv_reg_mr()          // Register host memory with RDMA
  │   └─ NIC gets permission to DMA from/to host memory
  └─ ibv_create_qp()       // Create queue pair for communication
```

**Key Points:**
- Host memory is allocated and CPU-accessible for verification
- Memory registration creates lkey (local) and rkey (remote) for access control
- All memory operations use standard host RAM

### 3. Connection Establishment

The connection setup uses TCP for initial handshake:

```
TCP Socket Exchange (Client ←→ Server):
  - Exchange buffer addresses (host memory pointers)
  - Exchange remote keys (rkeys) for memory access
  - Exchange Queue Pair (QP) numbers
  - Exchange Local IDs (LIDs) and Global IDs (GIDs)
  - Transition QPs through states: INIT → RTR → RTS
```

**Connection Data Structure:**
```c
struct cm_con_data_t {
    uint64_t addr;      // Buffer address
    uint32_t rkey;      // Remote key
    uint32_t qp_num;    // Queue pair number
    uint16_t lid;       // Local ID (InfiniBand)
    uint8_t gid[16];    // Global ID (RoCE)
} __attribute__((packed));
```

### 4. Two-Sided Operations (Send/Receive)

#### Client Side Operations:
```
1. Prepare data in host memory buffer
2. post_send() → NIC reads from local host memory
3. NIC → Network → Remote NIC → Remote host memory
4. poll_completion() → Wait for send completion
```

#### Server Side Operations:
```
1. post_receive() → Prepare buffer for incoming data
2. Remote NIC → DMA write to local host memory
3. poll_completion() → Data arrival notification
4. Process received data (CPU can access host memory)
5. Send response back to client
```

### 5. One-Sided Operations (RDMA Write)

Client or server can perform direct writes to remote memory:

```
RDMA Write Flow:
1. Initiator: Prepare data in local host memory
2. Initiator: post_send(IBV_WR_RDMA_WRITE)
3. Local NIC: DMA read from local host memory
4. Network transfer
5. Remote NIC: DMA write directly to remote host memory
6. No remote CPU/software involvement (true one-sided)
```

### 6. Memory Access Keys

**lkey (Local Key)**: Used for ALL operations to access local memory
- Send operations: lkey for local source buffer
- Receive operations: lkey for local target buffer  
- RDMA operations: lkey for local buffer

**rkey (Remote Key)**: Used only for one-sided operations
- RDMA Write: rkey to access remote target memory
- RDMA Read: rkey to access remote source memory

### 7. Network Addressing (LID/GID)

The implementation automatically adapts to different network types:

**LID (Local ID)**: Used for InfiniBand networks
- 16-bit identifier assigned by subnet manager
- Used for local subnet communication
- Automatically extracted from port attributes

**GID (Global ID)**: Used for RoCE (RDMA over Converged Ethernet) networks  
- 128-bit IPv6-based identifier
- Used for routed/global communication
- Automatically queried when Ethernet link layer is detected

```c
// Network type detection
if (ctx->port_attr.link_layer == IBV_LINK_LAYER_ETHERNET) {
    // RoCE network - use GID for addressing
    ibv_query_gid(ctx->ib_ctx, 1, 0, &my_gid);
} else {
    // InfiniBand network - use LID for addressing
    // LID obtained from ctx->port_attr.lid
}
```

Both LID and GID are exchanged during the TCP handshake and used in the QP address handle configuration.

## Key Implementation Details

### Data Verification and Patterns

The demo uses recognizable integer patterns to verify successful data transfer:

1. **Client patterns**: 2000, 2100, 2200... (different per iteration)
2. **Server patterns**: 1000, 1100, 1200... (different per iteration)  
3. **RDMA Write patterns**: 3000, 4000... (distinguishable from Send/Receive)

### Queue Pair State Machine

Standard InfiniBand connection establishment:
```
RESET → INIT → RTR (Ready to Receive) → RTS (Ready to Send)
```

### Memory Registration Flags

The implementation uses comprehensive access permissions:
```c
IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | 
IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_ATOMIC
```

### Address Handle Configuration

The QP address handle adapts to network type:

**For InfiniBand (LID-based):**
```c
.ah_attr = {
    .is_global = 0,           // Local routing
    .dlid = remote_lid,       // Use remote LID
    .port_num = 1
}
```

**For RoCE (GID-based):**
```c
.ah_attr = {
    .is_global = 1,           // Global routing
    .dlid = remote_lid,       // Still need LID
    .grh.dgid = remote_gid,   // Use remote GID
    .grh.sgid_index = 0,
    .grh.hop_limit = 1,
    .port_num = 1
}
```

## Prerequisites

- RDMA-capable network infrastructure (InfiniBand or RoCE)
- Ubuntu Linux (tested on Ubuntu 22.04 LTS)
- CMake (3.10+)
- C compiler with C99 support
- RDMA verbs library (`libibverbs`)

## Building the Project

```bash
# Create build directory and build
mkdir -p build && cd build

# Configure build (Release by default)
cmake ..

# For debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build executables
make
```

This creates two executables:
- `rdma_server` - RDMA server application
- `rdma_client` - RDMA client application

## Usage

### Running the Server

```bash
# Basic server (listens on port 20000)
./rdma_server

# Server with custom options
./rdma_server [-p port] [-d ib_device] [-s buffer_size]

# Example with custom port and buffer size
./rdma_server -p 30000 -s 8388608
```

### Running the Client

```bash
# Connect to server
./rdma_client <server_ip>

# Client with custom options  
./rdma_client <server_ip> [-p port] [-d ib_device] [-s buffer_size]

# Example connecting to remote server
./rdma_client 192.168.1.100 -p 30000 -s 8388608
```

### Command Line Options

- `-p <port>`: TCP port for initial connection handshake (default: 20000)
- `-d <device>`: InfiniBand device name (auto-detected if not specified)
- `-s <size>`: Buffer size in bytes (default: 4194304 = 4MB)
- `-h`: Display help message

## What the Demo Shows

When you run the client and server, you'll see:

### **Two-Sided Communication (Send/Receive)**
```
[Client] Sending data (first 10 of 256 ints): 2000 2001 2002 2003 2004 2005 2006 2007 2008 2009 ...
✓ Message sent successfully
[Client] Received from server (first 10 of 256 ints): 1000 1001 1002 1003 1004 1005 1006 1007 1008 1009 ...
```

### **One-Sided Communication (RDMA Write)**
```
Demonstrating RDMA Write...
[Client] RDMA Write data (first 10 of 256 ints): 3000 3001 3002 3003 3004 3005 3006 3007 3008 3009 ...
✓ RDMA Write completed (one-sided operation)
```

The server will detect when its memory has been modified by the client's RDMA Write operation, demonstrating true one-sided communication.

## Project Structure

```
rdma-gdr/
├── include/
│   └── rdma_common.h          # RDMA data structures and function declarations
├── src/
│   ├── rdma_common.c          # Core RDMA implementation  
│   ├── rdma_client.c          # Client application
│   └── rdma_server.c          # Server application
├── CMakeLists.txt             # Build configuration
└── README.md                  # This file
```

## License

This project is licensed under the MIT License.




