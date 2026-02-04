# Network Programming in C

Learning to build network applications and a Mini-Redis clone using C, based on [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/).

## Project Structure

```text
.
├── Makefile           # Build script
├── lib/
│   └── network_lib.h  # Shared headers and helper functions
└── Src/
    ├── server.c       # Concurrent TCP Server
    ├── client.c       # TCP Client
    └── showip.c       # DNS/IP Resolution Tool
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

## End Chapter 1-6