#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_PENDING_CONNECTIONS 5

/**
 * TCP Server structure
 */
typedef struct {
    int socket_fd;
    int port;
    struct sockaddr_in address;
} TCPServer;

/**
 * Initialize TCP server
 * @param server Pointer to TCPServer structure
 * @param port Port number to bind to
 * @return 0 on success, -1 on failure
 */
int tcp_server_init(TCPServer *server, int port);

/**
 * Start listening for connections
 * @param server Pointer to TCPServer structure
 * @return 0 on success, -1 on failure
 */
int tcp_server_listen(TCPServer *server);

/**
 * Accept a client connection
 * @param server Pointer to TCPServer structure
 * @param client_addr Pointer to store client address
 * @return Client socket file descriptor on success, -1 on failure
 */
int tcp_server_accept(TCPServer *server, struct sockaddr_in *client_addr);

/**
 * Send data to client
 * @param client_fd Client socket file descriptor
 * @param data Data to send
 * @param length Length of data
 * @return Number of bytes sent, -1 on failure
 */
ssize_t tcp_server_send(int client_fd, const char *data, size_t length);

/**
 * Receive data from client
 * @param client_fd Client socket file descriptor
 * @param buffer Buffer to store received data
 * @param buffer_size Size of buffer
 * @return Number of bytes received, -1 on failure, 0 on connection close
 */
ssize_t tcp_server_recv(int client_fd, char *buffer, size_t buffer_size);

/**
 * Close TCP server
 * @param server Pointer to TCPServer structure
 */
void tcp_server_close(TCPServer *server);

#endif // TCP_SERVER_H
