#include "../include/mq_common.h"
#include <signal.h>

static mqd_t server_mq = (mqd_t)-1;
static mqd_t client_mq = (mqd_t)-1;

void cleanup_and_exit(int sig) {
    printf("\nCleaning up and exiting...\n");
    
    if (server_mq != (mqd_t)-1) {
        mq_close(server_mq);
    }
    if (client_mq != (mqd_t)-1) {
        mq_close(client_mq);
    }
    
    cleanup_message_queue(SERVER_QUEUE_NAME);
    cleanup_message_queue(CLIENT_QUEUE_NAME);
    
    exit(0);
}

int main(void) {
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);
    
    printf("Message Queue OOB Server Starting...\n");
    printf("Server queue: %s\n", SERVER_QUEUE_NAME);
    printf("Client queue: %s\n", CLIENT_QUEUE_NAME);
    
    // Clean up any existing message queues
    cleanup_message_queue(SERVER_QUEUE_NAME);
    cleanup_message_queue(CLIENT_QUEUE_NAME);
    
    // Create server message queue for receiving client messages
    server_mq = create_message_queue(SERVER_QUEUE_NAME, O_RDONLY);
    if (server_mq == (mqd_t)-1) {
        exit(1);
    }
    
    printf("Server message queue created successfully\n");
    printf("Waiting for client to create its queue...\n");
    
    // Wait for client queue to be available
    int retry_count = 0;
    const int max_retries = 50;
    while (retry_count < max_retries) {
        client_mq = open_message_queue(CLIENT_QUEUE_NAME, O_WRONLY);
        if (client_mq != (mqd_t)-1) {
            break;
        }
        usleep(100000); // 100ms
        retry_count++;
    }
    
    if (client_mq == (mqd_t)-1) {
        printf("Failed to connect to client queue after %d retries\n", max_retries);
        cleanup_and_exit(0);
    }
    
    printf("Connected to client message queue\n");
    
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
    
    // Send server connection data to client
    printf("Sending server connection data to client...\n");
    if (send_connection_data(client_mq, &server_conn_data, 1001) == -1) {
        cleanup_and_exit(0);
    }
    
    // Wait for client connection data
    printf("Waiting for client connection data...\n");
    connection_data_t client_conn_data;
    uint32_t client_sequence;
    if (receive_connection_data(server_mq, &client_conn_data, &client_sequence) == -1) {
        cleanup_and_exit(0);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double exchange_time = get_time_diff_ms(start_time, end_time);
    
    printf("\nReceived client connection data (sequence: %u):\n", client_sequence);
    print_connection_data(&client_conn_data);
    printf("\nConnection data exchange completed in %.3f ms\n", exchange_time);
    
    printf("\nOOB exchange completed successfully!\n");
    printf("Message Queue Benefits:\n");
    printf("1. Structured message delivery with priorities\n");
    printf("2. Built-in message boundaries and ordering\n");
    printf("3. Kernel-managed buffering and flow control\n");
    printf("4. POSIX standard interface across platforms\n");
    
    printf("\nIn a real RDMA application, you would now:\n");
    printf("1. Use this data to establish RDMA QP connections\n");
    printf("2. Transition QP states: INIT -> RTR -> RTS\n");
    printf("3. Begin high-performance RDMA operations\n");
    
    // Demonstrate additional message exchange
    printf("\nDemonstrating additional message capabilities...\n");
    for (int i = 0; i < 3; i++) {
        connection_data_t extra_data = server_conn_data;
        extra_data.qp_num += i + 100;  // Simulate additional QPs
        
        if (send_connection_data(client_mq, &extra_data, 1001 + i + 1) == -1) {
            break;
        }
        printf("Sent additional QP data (QP: %u)\n", extra_data.qp_num);
        
        connection_data_t client_extra;
        uint32_t seq;
        if (receive_connection_data(server_mq, &client_extra, &seq) == -1) {
            break;
        }
        printf("Received client QP data (QP: %u, seq: %u)\n", client_extra.qp_num, seq);
        
        usleep(100000); // 100ms delay
    }
    
    printf("\nMessage exchange completed. Server shutting down.\n");
    
    // Keep server running briefly to ensure all messages are processed
    sleep(1);
    
    cleanup_and_exit(0);
    return 0;
}