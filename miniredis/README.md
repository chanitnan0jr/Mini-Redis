# Mini-Redis

This is the main Mini-Redis project implementation.

## Structure

- `include/`: Header files definitions.
- `src/`: Source code implementation.
- `tests/`: Unit tests (TODO).

## Compatibility & Requirements

This project is written in C and uses POSIX-compliant libraries (e.g., `<sys/socket.h>`, `<poll.h>`, `<unistd.h>`). 
It is designed to run on **Linux** or **macOS**. Windows users should use **WSL (Windows Subsystem for Linux)**.

## Building

Run `make` in this directory to build the `miniredis-server`.

## Usage

1. **Start the Server**:
   ```bash
   ./bin/miniredis-server
   ```
   Server will start on port `3490`.

2. **Connect with redis-cli**:
   Ideally, use `redis-cli` (installed via `sudo apt install redis-tools`):
   ```bash
   redis-cli -p 3490
   ```

3. **Supported Commands**:
   
   - **SET** (store a key-value pair):
     ```bash
     SET User Sinu
     OK
     ```
   
   - **GET** (retrieve a value):
     ```bash
     GET User
     "Sinu"
     ```
   
   - **DEL** (delete a key):
     ```bash
     DEL User
     1
     ```

   - **PING** (check connection):
     ```bash
     PING
     PING A RAI KUB
     ```

## Features

- **In-Memory Storage**: Uses a Hash Map (O(1) average).
- **Persistence (AOF)**: Commands are logged to `database.aof`. If you restart the server, data is restored automatically.
