#include "uds_common.h"

int create_unix_socket(void) {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation failed");
        return -1;
    }
    return sockfd;
}

int bind_unix_socket(int sockfd, const char *path) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    
    cleanup_socket_path(path);
    
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        return -1;
    }
    return 0;
}

int connect_unix_socket(int sockfd, const char *path) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    
    int retries = 0;
    while (retries < MAX_RETRIES) {
        if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            return 0;
        }
        
        if (errno == ENOENT) {
            printf("Server not ready, retrying in 1 second... (%d/%d)\n", 
                   retries + 1, MAX_RETRIES);
            sleep(1);
        } else {
            perror("connect failed");
            return -1;
        }
        retries++;
    }
    
    printf("Failed to connect after %d retries\n", MAX_RETRIES);
    return -1;
}

int send_data(int sockfd, const void *data, size_t len) {
    ssize_t bytes_sent = send(sockfd, data, len, 0);
    if (bytes_sent == -1) {
        perror("send failed");
        return -1;
    }
    if ((size_t)bytes_sent != len) {
        printf("Warning: sent %zd bytes, expected %zu\n", bytes_sent, len);
    }
    return 0;
}

int recv_data(int sockfd, void *data, size_t len) {
    ssize_t bytes_received = recv(sockfd, data, len, 0);
    if (bytes_received == -1) {
        perror("recv failed");
        return -1;
    }
    if (bytes_received == 0) {
        printf("Connection closed by peer\n");
        return -1;
    }
    if ((size_t)bytes_received != len) {
        printf("Warning: received %zd bytes, expected %zu\n", bytes_received, len);
    }
    return 0;
}

void cleanup_socket_path(const char *path) {
    if (unlink(path) == -1 && errno != ENOENT) {
        perror("unlink failed");
    }
}

void print_connection_data(const connection_data_t *conn_data) {
    printf("Connection Data:\n");
    printf("  Address: 0x%016lx\n", conn_data->addr);
    printf("  Remote Key: 0x%08x\n", conn_data->rkey);
    printf("  QP Number: %u\n", conn_data->qp_num);
    printf("  LID: %u\n", conn_data->lid);
    printf("  GID: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", conn_data->gid[i]);
        if (i % 2 == 1 && i < 15) printf(":");
    }
    printf("\n");
}

void print_message(const message_t *msg) {
    printf("Message ID: %d\n", msg->msg_id);
    printf("Payload: %s\n", msg->payload);
    printf("Data Length: %zu\n", msg->data_len);
    printf("Timestamp: %ld.%09ld\n", msg->timestamp.tv_sec, msg->timestamp.tv_nsec);
}

double get_time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 + 
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}