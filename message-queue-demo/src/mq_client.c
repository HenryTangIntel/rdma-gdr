#include "../include/mq_common.h"

int main(void) {
    printf("Message Queue OOB Client Starting...\n");
    printf("Server queue: %s\n", SERVER_QUEUE_NAME);
    printf("Client queue: %s\n", CLIENT_QUEUE_NAME);
    
    // Wait for server queue to be available
    printf("Waiting for server queue to be available...\n");
    mqd_t server_mq = (mqd_t)-1;
    int retry_count = 0;
    const int max_retries = 50;
    
    while (retry_count < max_retries) {
        server_mq = open_message_queue(SERVER_QUEUE_NAME, O_WRONLY);
        if (server_mq != (mqd_t)-1) {
            break;
        }
        usleep(100000); // 100ms
        retry_count++;
    }
    
    if (server_mq == (mqd_t)-1) {
        printf("Failed to connect to server queue after %d retries\n", max_retries);
        exit(1);
    }
    
    // Create client message queue for receiving server messages
    mqd_t client_mq = create_message_queue(CLIENT_QUEUE_NAME, O_RDONLY);
    if (client_mq == (mqd_t)-1) {
        mq_close(server_mq);
        exit(1);
    }
    
    printf("Connected to server and created client message queue\n");
    
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
    
    // Wait for server connection data
    printf("Waiting for server connection data...\n");
    connection_data_t server_conn_data;
    uint32_t server_sequence;
    if (receive_connection_data(client_mq, &server_conn_data, &server_sequence) == -1) {
        mq_close(server_mq);
        mq_close(client_mq);
        cleanup_message_queue(CLIENT_QUEUE_NAME);
        exit(1);
    }
    
    // Send client connection data to server
    printf("Sending client connection data to server...\n");
    if (send_connection_data(server_mq, &client_conn_data, 2001) == -1) {
        mq_close(server_mq);
        mq_close(client_mq);
        cleanup_message_queue(CLIENT_QUEUE_NAME);
        exit(1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double exchange_time = get_time_diff_ms(start_time, end_time);
    
    printf("\nReceived server connection data (sequence: %u):\n", server_sequence);
    print_connection_data(&server_conn_data);
    printf("\nConnection data exchange completed in %.3f ms\n", exchange_time);
    
    printf("\nOOB exchange completed successfully!\n");
    printf("Message Queue Advantages:\n");
    printf("1. Message boundaries preserved automatically\n");
    printf("2. Priority-based message delivery support\n");
    printf("3. Blocking/non-blocking receive options\n");
    printf("4. Built-in message size validation\n");
    
    printf("\nIn a real RDMA application, you would now:\n");
    printf("1. Use this data to establish RDMA QP connections\n");
    printf("2. Transition QP states: INIT -> RTR -> RTS\n");
    printf("3. Begin high-performance RDMA operations\n");
    
    // Participate in additional message exchange
    printf("\nParticipating in additional message exchange...\n");
    for (int i = 0; i < 3; i++) {
        // Receive server's additional QP data
        connection_data_t server_extra;
        uint32_t seq;
        if (receive_connection_data(client_mq, &server_extra, &seq) == -1) {
            break;
        }
        printf("Received server QP data (QP: %u, seq: %u)\n", server_extra.qp_num, seq);
        
        // Send corresponding client QP data
        connection_data_t client_extra = client_conn_data;
        client_extra.qp_num += i + 100;  // Simulate additional QPs
        
        if (send_connection_data(server_mq, &client_extra, 2001 + i + 1) == -1) {
            break;
        }
        printf("Sent client QP data (QP: %u)\n", client_extra.qp_num);
        
        usleep(50000); // 50ms delay
    }
    
    printf("\nMessage exchange completed. Client shutting down.\n");
    
    // Cleanup
    mq_close(server_mq);
    mq_close(client_mq);
    cleanup_message_queue(CLIENT_QUEUE_NAME);
    
    printf("Client shutdown complete.\n");
    return 0;
}