#include "../include/shm_common.h"
#include <signal.h>
#include <time.h>

static int shm_fd = -1;
static shm_oob_data_t *shm_ptr = NULL;
static sem_t *sem_server_ready = NULL;
static sem_t *sem_client_ready = NULL;
static sem_t *sem_data_ready = NULL;

void cleanup_and_exit(int sig) {
    printf("\nCleaning up and exiting...\n");
    
    if (shm_ptr != NULL) {
        munmap(shm_ptr, sizeof(shm_oob_data_t));
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }
    
    cleanup_shared_memory(SHM_NAME);
    cleanup_semaphore(SEM_SERVER_READY);
    cleanup_semaphore(SEM_CLIENT_READY);
    cleanup_semaphore(SEM_DATA_READY);
    
    if (sem_server_ready) sem_close(sem_server_ready);
    if (sem_client_ready) sem_close(sem_client_ready);
    if (sem_data_ready) sem_close(sem_data_ready);
    
    exit(0);
}

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
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);
    
    printf("Shared Memory OOB Server Starting...\n");
    printf("Shared memory name: %s\n", SHM_NAME);
    
    // Clean up any existing shared memory and semaphores
    cleanup_shared_memory(SHM_NAME);
    cleanup_semaphore(SEM_SERVER_READY);
    cleanup_semaphore(SEM_CLIENT_READY);
    cleanup_semaphore(SEM_DATA_READY);
    
    // Create shared memory
    shm_fd = create_shared_memory(SHM_NAME, sizeof(shm_oob_data_t));
    if (shm_fd == -1) {
        exit(1);
    }
    
    // Map shared memory
    shm_ptr = (shm_oob_data_t*)map_shared_memory(shm_fd, sizeof(shm_oob_data_t));
    if (shm_ptr == NULL) {
        close(shm_fd);
        cleanup_shared_memory(SHM_NAME);
        exit(1);
    }
    
    // Initialize shared memory
    memset(shm_ptr, 0, sizeof(shm_oob_data_t));
    
    // Create semaphores
    sem_server_ready = create_semaphore(SEM_SERVER_READY, 0);
    sem_client_ready = create_semaphore(SEM_CLIENT_READY, 0);
    sem_data_ready = create_semaphore(SEM_DATA_READY, 0);
    
    if (!sem_server_ready || !sem_client_ready || !sem_data_ready) {
        cleanup_and_exit(0);
    }
    
    printf("Shared memory and semaphores created successfully\n");
    
    // Prepare server connection data
    connection_data_t server_conn_data = {
        .addr = 0x1001001800000000UL,
        .rkey = 0x12345678,
        .qp_num = 1001,
        .lid = 1,
        .gid = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}
    };
    
    printf("\nPreparing server connection data:\n");
    print_connection_data(&server_conn_data);
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Copy server data to shared memory
    memcpy(&shm_ptr->server_data, &server_conn_data, sizeof(connection_data_t));
    shm_ptr->server_ready = 1;
    
    // Signal that server data is ready
    sem_post(sem_server_ready);
    printf("Server data written to shared memory, waiting for client...\n");
    
    // Wait for client to be ready
    sem_wait(sem_client_ready);
    printf("Client is ready, reading client data...\n");
    
    // Read client data
    connection_data_t client_conn_data;
    memcpy(&client_conn_data, &shm_ptr->client_data, sizeof(connection_data_t));
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double exchange_time = get_time_diff_ms(start_time, end_time);
    
    printf("\nReceived client connection data:\n");
    print_connection_data(&client_conn_data);
    printf("\nConnection data exchange completed in %.3f ms\n", exchange_time);
    
    printf("\nOOB exchange completed successfully!\n");
    printf("In a real RDMA application, you would now:\n");
    printf("1. Use this data to establish RDMA QP connections\n");
    printf("2. Transition QP states: INIT -> RTR -> RTS\n");
    printf("3. Begin high-performance RDMA operations\n");
    
    // Keep server running for a bit to allow client to finish
    sleep(1);
    
    cleanup_and_exit(0);
    return 0;
}