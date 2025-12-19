# NP-G10-TheQuizGame
Network Programming Project of Group 10.

## Framework

This project uses **POSIX Sockets (BSD Sockets)** as the client-server framework for TCP communication on Linux.

### Framework Features

- ✅ Native Linux support (no external dependencies)
- ✅ Direct TCP protocol implementation
- ✅ Clean, easy-to-use API
- ✅ Example echo server and client
- ✅ Comprehensive documentation

### Quick Start

```bash
# Build the project
make

# Run the echo server (Terminal 1)
./build/echo_server

# Run the echo client (Terminal 2)
./build/echo_client
```

### Documentation

For detailed framework documentation, see [docs/FRAMEWORK.md](docs/FRAMEWORK.md)

### Project Structure

```
.
├── include/          # Header files (API definitions)
├── src/             # Implementation files
├── examples/        # Example programs
├── docs/            # Documentation
├── build/           # Build output (generated)
├── Makefile         # Build configuration
└── README.md        # This file
```

### Building

```bash
make              # Build all targets
make examples     # Build example programs only
make clean        # Clean build artifacts
make help         # Show available targets
```

### Requirements

- GCC compiler
- Make build tool
- Linux/Unix operating system

### Framework Selection Rationale

**POSIX Sockets** was chosen for this project because:

1. **Native to Linux**: No external dependencies required
2. **TCP Support**: Direct support for TCP protocol via SOCK_STREAM
3. **Standard**: Industry-standard API, well-documented
4. **Lightweight**: Minimal overhead, direct system call access
5. **Educational**: Fundamental to understanding network programming

For more details, see the [Framework Documentation](docs/FRAMEWORK.md).
