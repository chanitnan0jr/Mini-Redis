# Network Programming in C (Chapter 1-6: Basic)

Learning to build network applications and a Mini-Redis clone using C, based on [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/).

## Project Structure

```text
.
├── Makefile           # Build script
├── lib/
│   └── network_lib.h  # Shared headers and helper functions
└── Src/
    ├── server.c       # TCP Server (Concurrent)
    ├── client.c       # TCP Client
    ├── listener.c     # UDP Server (Receiver)
    ├── talker.c       # UDP Client (Sender)
    └── showip.c       # DNS/IP Resolution Tool
    └── pollserver.c   # TCP Chat Server (I/O Multiplexing using poll())
```
## How to compile
```bash 
make
```
## Usage

### 1. TCP Server & Client
- Properties: Reliable, Connection-oriented.
- Guarantees that data arrives in order and without errors. The server uses fork() to handle multiple clients simultaneously.

#### Terminal 1: Start Server

```bash
./Src/server
```
(Server will listen on port 3490)

#### Terminal 2: Run Client

```bash
./Src/client 127.0.0.1
```

#### The result should be
```
Hello from Sinu Server
```

### 2. UDP Listener & Talker (Datagram Sockets)
- Properties: connectionless and just fire packets off into the ether with callous disregard for success
- Faster than TCP but does not guarantee delivery or order. 
- If IP versions mismatch (e.g. sender uses IPv4 but receiver expects IPv6), the data sent would not be received.

#### Terminal 1: Start Listener (Server)
```bash
./Src/listener
```
(Waiting for incoming packets on port 4950)

#### Terminal 2: Run Talker (Client)
```bash
./Src/talker localhost "Hello UDP server it's Sinu from talker.c"
```
(Note: You can run talker without listener; the packet will simply be lost in the ether!)
#### The result should be
```bash
listener: got packet from 127.0.0.1
listener: packet is 36 bytes long
listener: packet contains 
"Hello UDP server it's Sinu talker.c"
```

### 3. IP Lookup Tool (ShowIP)
Resolve a hostname to IP addresses.

```bash
./Src/showip google.com
```

## End Chapter 1-6 (Basic)

## Next (Chapter 7 - Slightly Advanced Techniques)

### 4. Chat Server with poll()
- Properties: Uses I/O Multiplexing to manage multiple clients within a single process.

- Efficiency: Unlike fork(), it uses poll() to monitor an array of file descriptors. The process "sleeps" and consumes zero CPU cycles until the OS signals that data is ready to be read.

- Summary:
    - Dynamic array management with realloc() for scalable connections.
    - Broadcasting: Messages sent by one client are relayed to all others.ss
    - Non-blocking feel without the "busy-wait" overhead.

#### Terminal 1: Start Poll Server
```bash
./Src/pollserver
```
(Listening on port 9034)

#### Terminal 2, 3, 4...: Connect multiple clients
You can use telnet or netcat to simulate multiple users:
```bash
telnet localhost 9034
```
or 

```bash
nc localhost 9034
```
Then you can try type in the terminal
```bash
Sinu: Networking is cool!
```
#### The result on terminal 1 should be
```bash
pollserver: waiting for connections...
pollserver: new connection from 127.0.0.1 on socket 4
pollserver: recv from fd 4: Sinu: Networking is cool!
```
#### The result on terminal the other terminal should be
```bash
Sinu: Networking is cool!
```

#### And if you wonder wWhy does my client start at Socket 4?
In Unix-like systems, every new process starts with three default File Descriptors (FD):

    0 (stdin): Keyboard input.
    1 (stdout): Standard output to screen.
    2 (stderr): Error output to screen.

When our server runs:

    FD 3 is assigned to the Listener Socket (the "Front Door").
    FD 4 is assigned to the First Client that connects.
    Subsequent clients will be assigned FD 5, 6, and so on.