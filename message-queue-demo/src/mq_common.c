#include "../include/mq_common.h"

mqd_t create_message_queue(const char *name, int flags) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = sizeof(mq_message_t);
    attr.mq_curmsgs = 0;
    
    mqd_t mq = mq_open(name, flags | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return (mqd_t)-1;
    }
    
    return mq;
}

mqd_t open_message_queue(const char *name, int flags) {
    mqd_t mq = mq_open(name, flags);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return (mqd_t)-1;
    }
    
    return mq;
}

int send_connection_data(mqd_t mq, const connection_data_t *data, uint32_t sequence) {
    mq_message_t msg;
    memset(&msg, 0, sizeof(msg));
    
    msg.type = MSG_CONNECTION_DATA;
    msg.sequence = sequence;
    msg.data_len = sizeof(connection_data_t);
    memcpy(&msg.conn_data, data, sizeof(connection_data_t));
    
    if (mq_send(mq, (const char*)&msg, sizeof(msg), 0) == -1) {
        perror("mq_send failed");
        return -1;
    }
    
    return 0;
}

int receive_connection_data(mqd_t mq, connection_data_t *data, uint32_t *sequence) {
    mq_message_t msg;
    ssize_t bytes_read;
    
    bytes_read = mq_receive(mq, (char*)&msg, sizeof(msg), NULL);
    if (bytes_read == -1) {
        perror("mq_receive failed");
        return -1;
    }
    
    if (msg.type != MSG_CONNECTION_DATA) {
        fprintf(stderr, "Unexpected message type: %d\n", msg.type);
        return -1;
    }
    
    memcpy(data, &msg.conn_data, sizeof(connection_data_t));
    if (sequence) {
        *sequence = msg.sequence;
    }
    
    return 0;
}

int cleanup_message_queue(const char *name) {
    return mq_unlink(name);
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