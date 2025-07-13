#ifndef MQ_COMMON_H
#define MQ_COMMON_H

#include <stdint.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SERVER_QUEUE_NAME "/rdma_oob_server_mq"
#define CLIENT_QUEUE_NAME "/rdma_oob_client_mq"
#define MAX_MSG_SIZE 1024
#define MAX_MESSAGES 10

// Message types for OOB exchange
typedef enum {
    MSG_CONNECTION_DATA = 1,
    MSG_ACK = 2,
    MSG_ERROR = 3
} message_type_t;

// Same connection data structure as RDMA
typedef struct {
    uint64_t addr;      // Buffer address
    uint32_t rkey;      // Remote key
    uint32_t qp_num;    // Queue pair number
    uint16_t lid;       // Local ID
    uint8_t gid[16];    // Global ID
} __attribute__((packed)) connection_data_t;

// Message structure for queue communication
typedef struct {
    message_type_t type;
    uint32_t sequence;
    uint32_t data_len;
    connection_data_t conn_data;
    char padding[MAX_MSG_SIZE - sizeof(message_type_t) - sizeof(uint32_t) - sizeof(uint32_t) - sizeof(connection_data_t)];
} __attribute__((packed)) mq_message_t;

// Function declarations
mqd_t create_message_queue(const char *name, int flags);
mqd_t open_message_queue(const char *name, int flags);
int send_connection_data(mqd_t mq, const connection_data_t *data, uint32_t sequence);
int receive_connection_data(mqd_t mq, connection_data_t *data, uint32_t *sequence);
int cleanup_message_queue(const char *name);
void print_connection_data(const connection_data_t *data);
double get_time_diff_ms(struct timespec start, struct timespec end);

#endif // MQ_COMMON_H