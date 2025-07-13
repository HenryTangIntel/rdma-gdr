#include "uds_common.h"
#include <unistd.h>

int main(void) {
    printf("Unix Domain Socket Client Starting...\n");
    printf("Connecting to: %s\n", SOCKET_PATH);
    
    int client_sock = create_unix_socket();
    if (client_sock == -1) {
        exit(1);
    }
    
    if (connect_unix_socket(client_sock, SOCKET_PATH) == -1) {
        close(client_sock);
        exit(1);
    }
    
    printf("Connected to server!\n");
    
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
    
    connection_data_t server_conn_data;
    if (recv_data(client_sock, &server_conn_data, sizeof(server_conn_data)) == -1) {
        close(client_sock);
        exit(1);
    }
    
    if (send_data(client_sock, &client_conn_data, sizeof(client_conn_data)) == -1) {
        close(client_sock);
        exit(1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double exchange_time = get_time_diff_ms(start_time, end_time);
    
    printf("\nReceived server connection data:\n");
    print_connection_data(&server_conn_data);
    printf("\nConnection data exchange completed in %.3f ms\n", exchange_time);
    
    printf("\nStarting message exchange...\n");
    
    for (int i = 0; i < 5; i++) {
        message_t msg = {
            .msg_id = i + 1,
            .data_len = strlen("Hello from client") + 1
        };
        snprintf(msg.payload, sizeof(msg.payload), 
                "Hello from client - message %d", i + 1);
        clock_gettime(CLOCK_REALTIME, &msg.timestamp);
        
        struct timespec msg_start, msg_end;
        clock_gettime(CLOCK_MONOTONIC, &msg_start);
        
        if (send_data(client_sock, &msg, sizeof(msg)) == -1) {
            break;
        }
        
        printf("\nSent message %d:\n", i + 1);
        print_message(&msg);
        
        message_t response;
        if (recv_data(client_sock, &response, sizeof(response)) == -1) {
            break;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &msg_end);
        double roundtrip_time = get_time_diff_ms(msg_start, msg_end);
        
        printf("Received response (roundtrip: %.3f ms):\n", roundtrip_time);
        print_message(&response);
        
        usleep(500000);
    }
    
    printf("\nMessage exchange completed. Closing connection.\n");
    
    close(client_sock);
    
    printf("Client shutdown complete.\n");
    return 0;
}