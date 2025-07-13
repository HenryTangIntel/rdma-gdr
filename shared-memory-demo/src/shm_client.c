#include "../include/shm_common.h"
#include <time.h>

void print_connection_data(const connection_data_t *data) {
    printf("  Address: 0x%016lx\n", data->addr);
    printf("  Remote Key: 0x%08x\n", data->rkey);
    printf("  QP Number: %u\n", data->qp_num);
    printf("  LID: %u\n", data->lid);
    printf("  GID: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", data->gid[i]);
        if (i % 2 == 1 && i < 15) printf(":");
    }
    printf("\n");
}

double get_time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 + 
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main(void) {
    printf("Shared Memory OOB Client Starting...\n");
    printf("Connecting to shared memory: %s\n", SHM_NAME);
    
    // Open existing shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed - is server running?");
        exit(1);
    }
    
    // Map shared memory
    shm_oob_data_t *shm_ptr = (shm_oob_data_t*)map_shared_memory(shm_fd, sizeof(shm_oob_data_t));
    if (shm_ptr == NULL) {
        close(shm_fd);
        exit(1);
    }
    
    // Open semaphores
    sem_t *sem_server_ready = sem_open(SEM_SERVER_READY, 0);
    sem_t *sem_client_ready = sem_open(SEM_CLIENT_READY, 0);
    sem_t *sem_data_ready = sem_open(SEM_DATA_READY, 0);
    
    if (sem_server_ready == SEM_FAILED || sem_client_ready == SEM_FAILED || 
        sem_data_ready == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shm_ptr, sizeof(shm_oob_data_t));
        close(shm_fd);
        exit(1);
    }
    
    printf("Connected to shared memory successfully\n");
    
    // Prepare client connection data
    connection_data_t client_conn_data = {
        .addr = 0x2002002900000000UL,
        .rkey = 0x87654321,
        .qp_num = 2001,
        .lid = 2,
        .gid = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20}
    };
    
    printf("\nPreparing client connection data:\n");
    print_connection_data(&client_conn_data);
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Wait for server to be ready
    printf("Waiting for server data...\n");
    sem_wait(sem_server_ready);
    
    // Read server data
    connection_data_t server_conn_data;
    memcpy(&server_conn_data, &shm_ptr->server_data, sizeof(connection_data_t));
    
    // Write client data
    memcpy(&shm_ptr->client_data, &client_conn_data, sizeof(connection_data_t));
    shm_ptr->client_ready = 1;
    
    // Signal that client data is ready
    sem_post(sem_client_ready);
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double exchange_time = get_time_diff_ms(start_time, end_time);
    
    printf("\nReceived server connection data:\n");
    print_connection_data(&server_conn_data);
    printf("\nConnection data exchange completed in %.3f ms\n", exchange_time);
    
    printf("\nOOB exchange completed successfully!\n");
    printf("In a real RDMA application, you would now:\n");
    printf("1. Use this data to establish RDMA QP connections\n");
    printf("2. Transition QP states: INIT -> RTR -> RTS\n");
    printf("3. Begin high-performance RDMA operations\n");
    
    // Cleanup
    sem_close(sem_server_ready);
    sem_close(sem_client_ready);
    sem_close(sem_data_ready);
    munmap(shm_ptr, sizeof(shm_oob_data_t));
    close(shm_fd);
    
    printf("Client shutdown complete.\n");
    return 0;
}