#include "uds_common.h"

static int server_sock = -1;
static int client_sock = -1;

void cleanup_and_exit(int sig) {
    printf("\nCleaning up and exiting...\n");
    if (client_sock != -1) close(client_sock);
    if (server_sock != -1) close(server_sock);
    cleanup_socket_path(SOCKET_PATH);
    exit(0);
}

int main(void) {
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);
    
    printf("Unix Domain Socket Server Starting...\n");
    printf("Socket path: %s\n", SOCKET_PATH);
    
    server_sock = create_unix_socket();
    if (server_sock == -1) {
        exit(1);
    }
    
    if (bind_unix_socket(server_sock, SOCKET_PATH) == -1) {
        close(server_sock);
        exit(1);
    }
    
    if (listen(server_sock, 1) == -1) {
        perror("listen failed");
        close(server_sock);
        cleanup_socket_path(SOCKET_PATH);
        exit(1);
    }
    
    printf("Server listening for connections...\n");
    
    client_sock = accept(server_sock, NULL, NULL);
    if (client_sock == -1) {
        perror("accept failed");
        close(server_sock);
        cleanup_socket_path(SOCKET_PATH);
        exit(1);
    }
    
    printf("Client connected!\n");
    
    connection_data_t server_conn_data = {
        .addr = 0x1001001800000000UL,
        .rkey = 0x12345678,
        .qp_num = 1001,
        .lid = 1,
        .gid = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}
    };
    
    printf("\nSending server connection data:\n");
    print_connection_data(&server_conn_data);
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    if (send_data(client_sock, &server_conn_data, sizeof(server_conn_data)) == -1) {
        close(client_sock);
        close(server_sock);
        cleanup_socket_path(SOCKET_PATH);
        exit(1);
    }
    
    connection_data_t client_conn_data;
    if (recv_data(client_sock, &client_conn_data, sizeof(client_conn_data)) == -1) {
        close(client_sock);
        close(server_sock);
        cleanup_socket_path(SOCKET_PATH);
        exit(1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double exchange_time = get_time_diff_ms(start_time, end_time);
    
    printf("\nReceived client connection data:\n");
    print_connection_data(&client_conn_data);
    printf("\nConnection data exchange completed in %.3f ms\n", exchange_time);
    
    printf("\nStarting message exchange...\n");
    
    for (int i = 0; i < 5; i++) {
        message_t msg;
        if (recv_data(client_sock, &msg, sizeof(msg)) == -1) {
            break;
        }
        
        printf("\nReceived message %d:\n", i + 1);
        print_message(&msg);
        
        message_t response = {
            .msg_id = msg.msg_id + 1000,
            .data_len = strlen("Server response") + 1
        };
        snprintf(response.payload, sizeof(response.payload), 
                "Server response to message %d", msg.msg_id);
        clock_gettime(CLOCK_REALTIME, &response.timestamp);
        
        if (send_data(client_sock, &response, sizeof(response)) == -1) {
            break;
        }
        
        printf("Sent response with ID %d\n", response.msg_id);
    }
    
    printf("\nMessage exchange completed. Closing connection.\n");
    
    close(client_sock);
    close(server_sock);
    cleanup_socket_path(SOCKET_PATH);
    
    printf("Server shutdown complete.\n");
    return 0;
}