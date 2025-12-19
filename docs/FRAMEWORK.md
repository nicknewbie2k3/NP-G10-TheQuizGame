# TCP Client-Server Framework Documentation

## Overview

This project implements a TCP client-server framework using **POSIX Sockets (BSD Sockets)** for Linux environments. The framework provides a clean, easy-to-use API for building TCP-based network applications.

## Framework Selection

### Chosen Framework: POSIX Sockets (BSD Sockets)

**Rationale for Selection:**

1. **Native to Linux**: POSIX sockets are built into Linux/Unix systems, requiring no external dependencies
2. **Direct TCP Support**: Native support for TCP protocol through `SOCK_STREAM` socket type
3. **Industry Standard**: Widely used and well-documented across the industry
4. **Lightweight**: Minimal overhead with direct system call access
5. **Portable**: Code works across all POSIX-compliant systems (Linux, Unix, macOS, etc.)
6. **Educational Value**: Understanding sockets is fundamental to network programming

### Alternatives Considered

- **Boost.Asio**: Requires C++ and external dependencies
- **libevent**: Additional library dependency
- **ZeroMQ**: Higher-level abstraction, overkill for basic TCP
- **gRPC**: Complex framework better suited for microservices

## Architecture

The framework consists of two main components:

### 1. TCP Server (`tcp_server.h/c`)

The server component provides:
- Socket creation and binding
- Listening for incoming connections
- Accepting client connections
- Bidirectional data transfer
- Connection management

**Key Functions:**
- `tcp_server_init()`: Initialize server on specified port
- `tcp_server_listen()`: Start listening for connections
- `tcp_server_accept()`: Accept a client connection
- `tcp_server_send()`: Send data to client
- `tcp_server_recv()`: Receive data from client
- `tcp_server_close()`: Close server socket

### 2. TCP Client (`tcp_client.h/c`)

The client component provides:
- Socket creation
- Connection to server
- Bidirectional data transfer
- Connection management

**Key Functions:**
- `tcp_client_init()`: Initialize client socket
- `tcp_client_connect()`: Connect to server
- `tcp_client_send()`: Send data to server
- `tcp_client_recv()`: Receive data from server
- `tcp_client_close()`: Close client connection

## Features

### Current Features
✅ TCP connection establishment  
✅ Bidirectional communication  
✅ Clean error handling  
✅ Socket option configuration (SO_REUSEADDR)  
✅ Simple API design  
✅ Example echo server/client  

### Future Enhancements
- Multi-threaded server support
- Non-blocking I/O
- SSL/TLS support
- Connection pooling
- Protocol buffers integration

## Directory Structure

```
.
├── include/           # Header files
│   ├── tcp_server.h  # Server API
│   └── tcp_client.h  # Client API
├── src/              # Implementation files
│   ├── tcp_server.c  # Server implementation
│   └── tcp_client.c  # Client implementation
├── examples/         # Example programs
│   ├── echo_server.c # Echo server example
│   └── echo_client.c # Echo client example
├── docs/             # Documentation
│   └── FRAMEWORK.md  # This file
├── build/            # Build output (generated)
└── Makefile          # Build configuration
```

## Building the Project

### Prerequisites
- GCC compiler
- Make build tool
- Linux/Unix operating system

### Build Commands

```bash
# Build all targets
make

# Build only examples
make examples

# Clean build artifacts
make clean

# Show help
make help
```

## Usage Examples

### Running the Echo Server

```bash
# Run on default port (8080)
./build/echo_server

# Run on custom port
./build/echo_server 9000
```

### Running the Echo Client

```bash
# Connect to localhost on default port (8080)
./build/echo_client

# Connect to specific server and port
./build/echo_client 192.168.1.100 9000
```

### Example Session

**Terminal 1 (Server):**
```
$ ./build/echo_server
TCP Server initialized on port 8080
Server listening on port 8080...
Echo server running on port 8080
Press Ctrl+C to stop

Client connected from 127.0.0.1:45678
Received: Hello, Server!
Echoed back 15 bytes
```

**Terminal 2 (Client):**
```
$ ./build/echo_client
TCP Client initialized
Connected to server 127.0.0.1:8080
Connected to echo server. Type messages (Ctrl+D to quit):
> Hello, Server!
Echo: Hello, Server!
> 
```

## API Documentation

### Server API

#### tcp_server_init()
```c
int tcp_server_init(TCPServer *server, int port);
```
Initialize a TCP server on the specified port.

**Parameters:**
- `server`: Pointer to TCPServer structure
- `port`: Port number to bind to (1-65535)

**Returns:** 0 on success, -1 on failure

#### tcp_server_listen()
```c
int tcp_server_listen(TCPServer *server);
```
Start listening for incoming connections.

**Parameters:**
- `server`: Pointer to initialized TCPServer

**Returns:** 0 on success, -1 on failure

#### tcp_server_accept()
```c
int tcp_server_accept(TCPServer *server, struct sockaddr_in *client_addr);
```
Accept a client connection (blocking call).

**Parameters:**
- `server`: Pointer to listening TCPServer
- `client_addr`: Pointer to store client address information

**Returns:** Client socket file descriptor on success, -1 on failure

#### tcp_server_send()
```c
ssize_t tcp_server_send(int client_fd, const char *data, size_t length);
```
Send data to a connected client.

**Parameters:**
- `client_fd`: Client socket file descriptor
- `data`: Data to send
- `length`: Number of bytes to send

**Returns:** Number of bytes sent on success, -1 on failure

#### tcp_server_recv()
```c
ssize_t tcp_server_recv(int client_fd, char *buffer, size_t buffer_size);
```
Receive data from a connected client.

**Parameters:**
- `client_fd`: Client socket file descriptor
- `buffer`: Buffer to store received data
- `buffer_size`: Size of buffer

**Returns:** Number of bytes received on success, 0 on connection close, -1 on failure

#### tcp_server_close()
```c
void tcp_server_close(TCPServer *server);
```
Close the server socket and cleanup.

**Parameters:**
- `server`: Pointer to TCPServer

### Client API

#### tcp_client_init()
```c
int tcp_client_init(TCPClient *client);
```
Initialize a TCP client.

**Parameters:**
- `client`: Pointer to TCPClient structure

**Returns:** 0 on success, -1 on failure

#### tcp_client_connect()
```c
int tcp_client_connect(TCPClient *client, const char *server_ip, int port);
```
Connect to a TCP server.

**Parameters:**
- `client`: Pointer to initialized TCPClient
- `server_ip`: Server IP address (e.g., "127.0.0.1")
- `port`: Server port number

**Returns:** 0 on success, -1 on failure

#### tcp_client_send()
```c
ssize_t tcp_client_send(TCPClient *client, const char *data, size_t length);
```
Send data to the server.

**Parameters:**
- `client`: Pointer to connected TCPClient
- `data`: Data to send
- `length`: Number of bytes to send

**Returns:** Number of bytes sent on success, -1 on failure

#### tcp_client_recv()
```c
ssize_t tcp_client_recv(TCPClient *client, char *buffer, size_t buffer_size);
```
Receive data from the server.

**Parameters:**
- `client`: Pointer to connected TCPClient
- `buffer`: Buffer to store received data
- `buffer_size`: Size of buffer

**Returns:** Number of bytes received on success, 0 on connection close, -1 on failure

#### tcp_client_close()
```c
void tcp_client_close(TCPClient *client);
```
Close the client connection and cleanup.

**Parameters:**
- `client`: Pointer to TCPClient

## Protocol Specification

### TCP Protocol Details

- **Transport Protocol**: TCP (Transmission Control Protocol)
- **Socket Type**: SOCK_STREAM (connection-oriented, reliable byte stream)
- **Address Family**: AF_INET (IPv4)
- **Default Port**: 8080 (configurable)

### Connection Flow

1. **Server Side:**
   - Create socket
   - Bind to port
   - Listen for connections
   - Accept client connections
   - Read/Write data
   - Close connection

2. **Client Side:**
   - Create socket
   - Connect to server
   - Read/Write data
   - Close connection

## Error Handling

The framework includes comprehensive error handling:

- **NULL pointer checks**: All functions validate input pointers
- **System call validation**: All socket operations check return values
- **Descriptive error messages**: Uses `perror()` for system errors
- **Return codes**: Consistent return value convention (0 for success, -1 for error)

## Security Considerations

### Current Implementation
- Basic input validation
- Buffer overflow protection (bounded buffer sizes)

### Recommended Enhancements for Production
- Input sanitization
- Rate limiting
- Connection timeouts
- SSL/TLS encryption
- Authentication mechanisms
- DDoS protection

## Performance Considerations

### Single-Threaded Model
The current implementation uses a single-threaded, blocking I/O model suitable for:
- Learning and prototyping
- Low-concurrency applications
- Simple request-response patterns

### Scalability Options
For high-performance applications, consider:
- **Multi-threading**: Handle each client in a separate thread
- **Multi-processing**: Fork child processes for each client
- **Non-blocking I/O**: Use `select()`, `poll()`, or `epoll()`
- **Asynchronous I/O**: Use `io_uring` or libuv

## Testing

### Manual Testing
1. Build the project: `make`
2. Start the server: `./build/echo_server`
3. In another terminal, run the client: `./build/echo_client`
4. Type messages and verify echo functionality

### Testing Checklist
- ✅ Server initialization
- ✅ Client connection
- ✅ Bidirectional communication
- ✅ Multiple messages
- ✅ Graceful disconnection
- ✅ Error handling

## Contributing

To extend this framework:

1. **Adding Features**: Implement new functions in `src/` with declarations in `include/`
2. **Examples**: Add new example programs in `examples/`
3. **Documentation**: Update this file with new features
4. **Testing**: Verify functionality manually or with test scripts

## References

### Documentation
- [POSIX Socket API](https://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html)
- [Linux Socket Programming](https://man7.org/linux/man-pages/man7/socket.7.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### Related Standards
- RFC 793: Transmission Control Protocol (TCP)
- RFC 791: Internet Protocol (IP)
- IEEE Std 1003.1: POSIX.1 Standard

## License

This framework is part of the NP-G10-TheQuizGame project for educational purposes.
