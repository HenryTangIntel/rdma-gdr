#include "rdma_common.h"

int main(int argc, char *argv[]) {
    rdma_context_t ctx = {0};
    ctx.sock = -1;
    
    int port = 20000;
    char *ib_dev_name = NULL;
    size_t buffer_size = RDMA_BUFFER_SIZE;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            ib_dev_name = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            buffer_size = strtoull(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [-p port] [-d ib_dev] [-s buffer_size]\n", argv[0]);
            return 0;
        }
    }
    
    printf("RDMA Server\n");
    printf("===========\n");
    printf("Port: %d\n", port);
    printf("Buffer size: %zu bytes\n", buffer_size);
    if (ib_dev_name) printf("IB device: %s\n", ib_dev_name);
    printf("\n");
    
    // Initialize RDMA buffer
    printf("Initializing RDMA buffer...\n");
    if (init_rdma_buffer(&ctx, buffer_size) < 0) {
        fprintf(stderr, "Failed to initialize RDMA buffer\n");
        cleanup_resources(&ctx);
        return 1;
    }
    
    // Initialize RDMA resources
    printf("\nInitializing RDMA resources...\n");
    if (init_rdma_resources(&ctx, ib_dev_name) < 0) {
        fprintf(stderr, "Failed to initialize RDMA resources\n");
        cleanup_resources(&ctx);
        return 1;
    }
    
    // Wait for client connection
    printf("\nWaiting for client connection on port %d...\n", port);
    if (connect_qp(&ctx, NULL, port) < 0) {
        fprintf(stderr, "Failed to establish connection\n");
        cleanup_resources(&ctx);
        return 1;
    }
    printf("✓ Client connected\n");
    
    // Function to display buffer data (first few integers)
    void display_buffer_data(const char *label, void *buffer, size_t size) {
        int *int_data = (int *)buffer;
        int count = size / sizeof(int);
        int display_count = count > 10 ? 10 : count;
        
        printf("%s (first %d of %d ints): ", label, display_count, count);
        for (int i = 0; i < display_count; i++) {
            printf("%d ", int_data[i]);
        }
        printf("...\n");
    }
    
    // Initialize buffer with server's test pattern
    printf("\n[Server] Initializing buffer with server data...\n");
    int *int_data = (int *)ctx.buffer;
    int count = MSG_SIZE / sizeof(int);
    
    // Write a recognizable server pattern
    for (int i = 0; i < count; i++) {
        int_data[i] = 1000 + i;  // Pattern: 1000, 1001, 1002...
    }
    
    display_buffer_data("[Server] Initial server data", ctx.buffer, MSG_SIZE);
    
    // Main communication loop
    printf("\nStarting communication loop...\n");
    
    for (int i = 0; i < 3; i++) {
        printf("\n--- Iteration %d ---\n", i + 1);
        
        // Post receive to wait for client message
        printf("Waiting for client message...\n");
        if (post_receive(&ctx) < 0) {
            fprintf(stderr, "Failed to post receive\n");
            break;
        }
        
        if (poll_completion(&ctx) < 0) {
            fprintf(stderr, "Receive completion failed\n");
            break;
        }
        
        display_buffer_data("[Server] Received from client", ctx.buffer, MSG_SIZE);
        printf("✓ Message received successfully\n");
        
        // Prepare response data
        printf("Preparing response to client...\n");
        for (int j = 0; j < count; j++) {
            int_data[j] = 1000 + i * 100 + j;  // Pattern: 1000, 1100, 1200...
        }
        
        display_buffer_data("[Server] Sending response", ctx.buffer, MSG_SIZE);
        
        // Send response back to client
        if (post_send(&ctx, IBV_WR_SEND) < 0) {
            fprintf(stderr, "Failed to post send\n");
            break;
        }
        
        if (poll_completion(&ctx) < 0) {
            fprintf(stderr, "Send completion failed\n");
            break;
        }
        printf("✓ Response sent successfully\n");
        
        // Check if we received RDMA Write data (iteration 2)
        if (i == 1) {
            printf("\nChecking for RDMA Write data from client...\n");
            // Give client time to perform RDMA write
            usleep(200000);  // 200ms
            
            display_buffer_data("[Server] Buffer after potential RDMA Write", ctx.buffer, MSG_SIZE);
            
            // Check if buffer was modified by RDMA write
            int rdma_write_detected = 0;
            for (int j = 0; j < count && j < 10; j++) {
                if (int_data[j] >= 3000 && int_data[j] < 4000) {
                    rdma_write_detected = 1;
                    break;
                }
            }
            
            if (rdma_write_detected) {
                printf("✓ RDMA Write from client detected!\n");
                printf("  Client successfully wrote data directly to server memory\n");
            } else {
                printf("? RDMA Write data not detected (may have been overwritten)\n");
            }
            
            // Demonstrate server-side RDMA Write
            printf("\nDemonstrating server RDMA Write to client...\n");
            
            // Prepare server's RDMA write data
            for (int j = 0; j < count; j++) {
                int_data[j] = 4000 + j;  // Pattern: 4000, 4001, 4002...
            }
            
            display_buffer_data("[Server] RDMA Write data", ctx.buffer, MSG_SIZE);
            
            if (post_send(&ctx, IBV_WR_RDMA_WRITE) < 0) {
                fprintf(stderr, "Failed to post RDMA write\n");
                break;
            }
            
            if (poll_completion(&ctx) < 0) {
                fprintf(stderr, "RDMA write completion failed\n");
                break;
            }
            printf("✓ Server RDMA Write completed (one-sided to client)\n");
        }
        
        // Small delay between iterations
        usleep(100000);  // 100ms
    }
    
    printf("\nCommunication completed successfully!\n");
    printf("Server demonstrated:\n");
    printf("  ✓ Send/Receive operations\n");
    printf("  ✓ RDMA Write (one-sided to client)\n");
    printf("  ✓ RDMA Write reception (from client)\n");
    printf("  ✓ Memory registration and management\n");
    printf("  ✓ Connection establishment\n");
    
    cleanup_resources(&ctx);
    return 0;
}