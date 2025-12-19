#include "tcp_client.h"

int tcp_client_init(TCPClient *client) {
    if (client == NULL) {
        fprintf(stderr, "Error: NULL client pointer\n");
        return -1;
    }

    // Create socket
    client->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket_fd < 0) {
        perror("Error creating socket");
        return -1;
    }

    printf("TCP Client initialized\n");
    return 0;
}

int tcp_client_connect(TCPClient *client, const char *server_ip, int port) {
    if (client == NULL) {
        fprintf(stderr, "Error: NULL client pointer\n");
        return -1;
    }

    if (server_ip == NULL) {
        fprintf(stderr, "Error: NULL server_ip pointer\n");
        return -1;
    }

    // Initialize server address structure
    memset(&client->server_address, 0, sizeof(client->server_address));
    client->server_address.sin_family = AF_INET;
    client->server_address.sin_port = htons(port);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, server_ip, &client->server_address.sin_addr) <= 0) {
        perror("Error: Invalid address or address not supported");
        return -1;
    }

    // Connect to server
    if (connect(client->socket_fd, (struct sockaddr *)&client->server_address, 
                sizeof(client->server_address)) < 0) {
        perror("Error connecting to server");
        return -1;
    }

    printf("Connected to server %s:%d\n", server_ip, port);
    return 0;
}

ssize_t tcp_client_send(TCPClient *client, const char *data, size_t length) {
    if (client == NULL) {
        fprintf(stderr, "Error: NULL client pointer\n");
        return -1;
    }

    if (data == NULL) {
        fprintf(stderr, "Error: NULL data pointer\n");
        return -1;
    }

    ssize_t bytes_sent = send(client->socket_fd, data, length, 0);
    if (bytes_sent < 0) {
        perror("Error sending data");
        return -1;
    }

    return bytes_sent;
}

ssize_t tcp_client_recv(TCPClient *client, char *buffer, size_t buffer_size) {
    if (client == NULL) {
        fprintf(stderr, "Error: NULL client pointer\n");
        return -1;
    }

    if (buffer == NULL) {
        fprintf(stderr, "Error: NULL buffer pointer\n");
        return -1;
    }

    memset(buffer, 0, buffer_size);
    ssize_t bytes_received = recv(client->socket_fd, buffer, buffer_size - 1, 0);
    
    if (bytes_received < 0) {
        perror("Error receiving data");
        return -1;
    }

    return bytes_received;
}

void tcp_client_close(TCPClient *client) {
    if (client != NULL && client->socket_fd >= 0) {
        close(client->socket_fd);
        printf("Client connection closed\n");
    }
}
