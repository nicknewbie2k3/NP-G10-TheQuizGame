#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

/**
 * TCP Client structure
 */
typedef struct {
    int socket_fd;
    struct sockaddr_in server_address;
} TCPClient;

/**
 * Initialize TCP client
 * @param client Pointer to TCPClient structure
 * @return 0 on success, -1 on failure
 */
int tcp_client_init(TCPClient *client);

/**
 * Connect to TCP server
 * @param client Pointer to TCPClient structure
 * @param server_ip Server IP address
 * @param port Server port number
 * @return 0 on success, -1 on failure
 */
int tcp_client_connect(TCPClient *client, const char *server_ip, int port);

/**
 * Send data to server
 * @param client Pointer to TCPClient structure
 * @param data Data to send
 * @param length Length of data
 * @return Number of bytes sent, -1 on failure
 */
ssize_t tcp_client_send(TCPClient *client, const char *data, size_t length);

/**
 * Receive data from server
 * @param client Pointer to TCPClient structure
 * @param buffer Buffer to store received data
 * @param buffer_size Size of buffer
 * @return Number of bytes received, -1 on failure, 0 on connection close
 */
ssize_t tcp_client_recv(TCPClient *client, char *buffer, size_t buffer_size);

/**
 * Close TCP client connection
 * @param client Pointer to TCPClient structure
 */
void tcp_client_close(TCPClient *client);

#endif // TCP_CLIENT_H
