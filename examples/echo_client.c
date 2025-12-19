#include "tcp_client.h"

int main(int argc, char *argv[]) {
    const char *server_ip = "127.0.0.1";
    int port = 8080;

    if (argc > 1) {
        server_ip = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }

    TCPClient client;

    // Initialize client
    if (tcp_client_init(&client) < 0) {
        fprintf(stderr, "Failed to initialize client\n");
        return 1;
    }

    // Connect to server
    if (tcp_client_connect(&client, server_ip, port) < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        tcp_client_close(&client);
        return 1;
    }

    printf("Connected to echo server. Type messages (Ctrl+D to quit):\n");

    char send_buffer[BUFFER_SIZE];
    char recv_buffer[BUFFER_SIZE];

    // Main client loop
    while (1) {
        printf("> ");
        fflush(stdout);

        // Read input from user
        if (fgets(send_buffer, BUFFER_SIZE, stdin) == NULL) {
            printf("\nExiting...\n");
            break;
        }

        // Send message to server
        size_t msg_len = strlen(send_buffer);
        ssize_t bytes_sent = tcp_client_send(&client, send_buffer, msg_len);
        if (bytes_sent < 0) {
            fprintf(stderr, "Failed to send message\n");
            break;
        }

        // Receive echo from server
        ssize_t bytes_received = tcp_client_recv(&client, recv_buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Server closed connection\n");
            }
            break;
        }

        printf("Echo: %s", recv_buffer);
    }

    tcp_client_close(&client);
    return 0;
}
