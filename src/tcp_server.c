#include "tcp_server.h"

int tcp_server_init(TCPServer *server, int port) {
    if (server == NULL) {
        fprintf(stderr, "Error: NULL server pointer\n");
        return -1;
    }

    // Create socket
    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd < 0) {
        perror("Error creating socket");
        return -1;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(server->socket_fd);
        return -1;
    }

    // Initialize server address structure
    memset(&server->address, 0, sizeof(server->address));
    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(port);
    server->port = port;

    // Bind socket to address
    if (bind(server->socket_fd, (struct sockaddr *)&server->address, sizeof(server->address)) < 0) {
        perror("Error binding socket");
        close(server->socket_fd);
        return -1;
    }

    printf("TCP Server initialized on port %d\n", port);
    return 0;
}

int tcp_server_listen(TCPServer *server) {
    if (server == NULL) {
        fprintf(stderr, "Error: NULL server pointer\n");
        return -1;
    }

    if (listen(server->socket_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("Error listening on socket");
        return -1;
    }

    printf("Server listening on port %d...\n", server->port);
    return 0;
}

int tcp_server_accept(TCPServer *server, struct sockaddr_in *client_addr) {
    if (server == NULL) {
        fprintf(stderr, "Error: NULL server pointer\n");
        return -1;
    }

    socklen_t client_len = sizeof(*client_addr);
    int client_fd = accept(server->socket_fd, (struct sockaddr *)client_addr, &client_len);
    
    if (client_fd < 0) {
        perror("Error accepting connection");
        return -1;
    }

    printf("Client connected from %s:%d\n", 
           inet_ntoa(client_addr->sin_addr), 
           ntohs(client_addr->sin_port));

    return client_fd;
}

ssize_t tcp_server_send(int client_fd, const char *data, size_t length) {
    if (data == NULL) {
        fprintf(stderr, "Error: NULL data pointer\n");
        return -1;
    }

    ssize_t bytes_sent = send(client_fd, data, length, 0);
    if (bytes_sent < 0) {
        perror("Error sending data");
        return -1;
    }

    return bytes_sent;
}

ssize_t tcp_server_recv(int client_fd, char *buffer, size_t buffer_size) {
    if (buffer == NULL) {
        fprintf(stderr, "Error: NULL buffer pointer\n");
        return -1;
    }

    memset(buffer, 0, buffer_size);
    ssize_t bytes_received = recv(client_fd, buffer, buffer_size - 1, 0);
    
    if (bytes_received < 0) {
        perror("Error receiving data");
        return -1;
    }

    return bytes_received;
}

void tcp_server_close(TCPServer *server) {
    if (server != NULL && server->socket_fd >= 0) {
        close(server->socket_fd);
        printf("Server closed\n");
    }
}
