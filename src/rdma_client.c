#include "rdma_common.h"

int main(int argc, char *argv[]) {
    rdma_context_t ctx = {0};
    ctx.sock = -1;
    
    char *server_name = NULL;
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
            printf("Usage: %s <server> [-p port] [-d ib_dev] [-s buffer_size]\n", argv[0]);
            return 0;
        } else if (!server_name) {
            server_name = argv[i];
        }
    }
    
    if (!server_name) {
        fprintf(stderr, "Error: Server name required\n");
        printf("Usage: %s <server> [-p port] [-d ib_dev] [-s buffer_size]\n", argv[0]);
        return 1;
    }
    
    printf("RDMA Client\n");
    printf("===========\n");
    printf("Server: %s:%d\n", server_name, port);
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
    
    // Connect to server
    printf("\nConnecting to server %s:%d...\n", server_name, port);
    if (connect_qp(&ctx, server_name, port) < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        cleanup_resources(&ctx);
        return 1;
    }
    printf("✓ Connected to server\n");
    
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
    
    // Main communication loop
    printf("\nStarting communication...\n");
    
    for (int i = 0; i < 3; i++) {
        printf("\n--- Iteration %d ---\n", i + 1);
        
        // Prepare message data
        int *int_data = (int *)ctx.buffer;
        int count = MSG_SIZE / sizeof(int);
        
        // Write client data pattern
        for (int j = 0; j < count; j++) {
            int_data[j] = 2000 + i * 100 + j;  // Pattern: 2000, 2100, 2200...
        }
        
        display_buffer_data("[Client] Sending data", ctx.buffer, MSG_SIZE);
        
        // Send message to server
        printf("Sending message to server...\n");
        if (post_send(&ctx, IBV_WR_SEND) < 0) {
            fprintf(stderr, "Failed to post send\n");
            break;
        }
        
        if (poll_completion(&ctx) < 0) {
            fprintf(stderr, "Send completion failed\n");
            break;
        }
        printf("✓ Message sent successfully\n");
        
        // Post receive for server response
        printf("Waiting for server response...\n");
        if (post_receive(&ctx) < 0) {
            fprintf(stderr, "Failed to post receive\n");
            break;
        }
        
        if (poll_completion(&ctx) < 0) {
            fprintf(stderr, "Receive completion failed\n");
            break;
        }
        
        display_buffer_data("[Client] Received from server", ctx.buffer, MSG_SIZE);
        printf("✓ Response received successfully\n");
        
        // Demonstrate RDMA Write operation
        if (i == 1) {
            printf("\nDemonstrating RDMA Write...\n");
            
            // Prepare different data for RDMA write
            for (int j = 0; j < count; j++) {
                int_data[j] = 3000 + j;  // Pattern: 3000, 3001, 3002...
            }
            
            display_buffer_data("[Client] RDMA Write data", ctx.buffer, MSG_SIZE);
            
            if (post_send(&ctx, IBV_WR_RDMA_WRITE) < 0) {
                fprintf(stderr, "Failed to post RDMA write\n");
                break;
            }
            
            if (poll_completion(&ctx) < 0) {
                fprintf(stderr, "RDMA write completion failed\n");
                break;
            }
            printf("✓ RDMA Write completed (one-sided operation)\n");
        }
        
        // Small delay between iterations
        usleep(100000);  // 100ms
    }
    
    printf("\nCommunication completed successfully!\n");
    printf("Client demonstrated:\n");
    printf("  ✓ Send/Receive operations\n");
    printf("  ✓ RDMA Write (one-sided)\n");
    printf("  ✓ Memory registration and management\n");
    printf("  ✓ Connection establishment\n");
    
    cleanup_resources(&ctx);
    return 0;
}