#ifndef SHM_COMMON_H
#define SHM_COMMON_H

#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define SHM_NAME "/rdma_oob_shm"
#define SEM_SERVER_READY "/rdma_server_ready"
#define SEM_CLIENT_READY "/rdma_client_ready"
#define SEM_DATA_READY "/rdma_data_ready"

// Same connection data structure as RDMA
typedef struct {
    uint64_t addr;      // Buffer address
    uint32_t rkey;      // Remote key
    uint32_t qp_num;    // Queue pair number
    uint16_t lid;       // Local ID
    uint8_t gid[16];    // Global ID
} __attribute__((packed)) connection_data_t;

// Shared memory structure for OOB exchange
typedef struct {
    volatile int server_ready;
    volatile int client_ready;
    volatile int data_ready;
    connection_data_t server_data;
    connection_data_t client_data;
} __attribute__((packed)) shm_oob_data_t;

// Function declarations
int create_shared_memory(const char *name, size_t size);
void* map_shared_memory(int fd, size_t size);
int cleanup_shared_memory(const char *name);
sem_t* create_semaphore(const char *name, int value);
int cleanup_semaphore(const char *name);

#endif // SHM_COMMON_H