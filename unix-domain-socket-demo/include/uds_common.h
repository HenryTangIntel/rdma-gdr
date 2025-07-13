#ifndef UDS_COMMON_H
#define UDS_COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/uds_demo_socket"
#define BUFFER_SIZE 1024
#define MAX_RETRIES 3

typedef struct {
    uint64_t addr;
    uint32_t rkey;
    uint32_t qp_num;
    uint16_t lid;
    uint8_t gid[16];
} connection_data_t;

typedef struct {
    int msg_id;
    char payload[256];
    size_t data_len;
    struct timespec timestamp;
} message_t;

int create_unix_socket(void);
int bind_unix_socket(int sockfd, const char *path);
int connect_unix_socket(int sockfd, const char *path);
int send_data(int sockfd, const void *data, size_t len);
int recv_data(int sockfd, void *data, size_t len);
void cleanup_socket_path(const char *path);
void print_connection_data(const connection_data_t *conn_data);
void print_message(const message_t *msg);
double get_time_diff_ms(struct timespec start, struct timespec end);

#endif