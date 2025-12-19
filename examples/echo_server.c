#include "tcp_server.h"

int main(int argc, char *argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    TCPServer server;
    
    // Initialize server
    if (tcp_server_init(&server, port) < 0) {
        fprintf(stderr, "Failed to initialize server\n");
        return 1;
    }

    // Start listening
    if (tcp_server_listen(&server) < 0) {
        fprintf(stderr, "Failed to listen\n");
        tcp_server_close(&server);
        return 1;
    }

    printf("Echo server running on port %d\n", port);
    printf("Press Ctrl+C to stop\n\n");

    // Main server loop
    while (1) {
        struct sockaddr_in client_addr;
        int client_fd = tcp_server_accept(&server, &client_addr);
        
        if (client_fd < 0) {
            continue;
        }

        // Handle client in a simple loop (single-threaded for simplicity)
        char buffer[BUFFER_SIZE];
        while (1) {
            ssize_t bytes_received = tcp_server_recv(client_fd, buffer, BUFFER_SIZE);
            
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Client disconnected\n");
                }
                break;
            }

            printf("Received: %s", buffer);
            
            // Echo back to client
            ssize_t bytes_sent = tcp_server_send(client_fd, buffer, bytes_received);
            if (bytes_sent < 0) {
                fprintf(stderr, "Failed to send response\n");
                break;
            }
            
            printf("Echoed back %zd bytes\n", bytes_sent);
        }

        close(client_fd);
    }

    tcp_server_close(&server);
    return 0;
}
