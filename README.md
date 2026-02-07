# Network Programming in C (Chapter 1-6: Basic)

Learning to build network applications and a Mini-Redis clone using C, based on [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/).

## Data Encapsulation
Beej explains that data is never sent "naked." It goes through a process called Encapsulation, which is like wrapping a letter inside multiple envelopes.

When a packet is born, it gets wrapped in a Header by the first protocol, then wrapped again by the next protocol, and so on.

- Step 1 (Application): Your data gets a protocol header (e.g., TFTP, HTTP).

- Step 2 (Transport): That bundle gets a UDP or TCP header.

- Step 3 (Internet): That bundle gets an IP header (addressing).

- Step 4 (Hardware): Finally, it gets a Hardware header (e.g., Ethernet) to go out on the wire.

Visual Structure: 
```bash
[ Ethernet Header [ IP Header [ TCP/UDP Header [ App(TFTP, HPPP) Header [ DATA ] ] ] ] ]
```
#### Receiving (Stripping): When the computer receives the packet, it does the reverse:
- Hardware strips the Ethernet header.
- Kernel strips the IP and UDP/TCP headers.
- Program strips the Application header.
- Finally, you are left with the Raw Data.

#### The Layered Network Model (ISO/OSI vs. TCP/IP)

Beej mentions two main models used to describe this structure.

- ISO/OSI Model (7 Layers): The theoretical standard (Application, Presentation, Session, Transport, Network, Data Link, Physical). It is very detailed and often used in academic courses.

- Unix/TCP/IP Model (4 Layers): The practical model used in real-world programming (Application, Host-to-Host Transport, Internet, Network Access).

#### About application header layer
here are common protocols and how they choose their Transport (Step 2):

- TFTP (Trivial File Transfer Protocol):

    - Transport: Uses UDP (Port 69).

    - Characteristics: Very simple, sends data in blocks and waits for an ACK. It is insecure (no login), but great for learning UDP basics or flashing firmware on routers.

- FTP (File Transfer Protocol):

    - Transport: Uses TCP (Ports 20 & 21).

    - Characteristics: Reliable but complex. It separates commands (Port 21) from data transfer (Port 20). Rarely used for new projects due to firewall issues.

- HTTP (Hypertext Transfer Protocol):

    - Transport: Uses TCP (Port 80).

    - Characteristics: The foundation of the Web. It uses a text-based "Request-Response" model (similar to the Mini-Redis project you are building).

- HTTPS (HTTP Secure):

    - Transport: Uses TCP (Port 443).

    - Characteristics: Same as HTTP but encrypted using TLS/SSL to prevent eavesdropping.

#### Why do we use layers?
- **Hardware Independence:** As a socket programmer, you don't need to know how the data is physically transmitted (whether it's via copper wire, fiber optics, satellite, or Wi-Fi).

- **Abstraction:** The lower layers (Kernel and Hardware) handle the messy details. You just write code to talk to the socket, and the network stack handles the rest.

## Project Structure

```text
.
├── Makefile           # Build script
├── lib/
│   └── network_lib.h  # Shared headers and helper functions
└── Src/
    ├── server.c       # TCP Server (Concurrent via fork)
    ├── client.c       # TCP Client
    ├── listener.c     # UDP Server (Receiver)
    ├── talker.c       # UDP Client (Sender)
    ├── showip.c       # DNS/IP Resolution Tool
    └── selectserver.c # TCP Chat Server (Multiplexing via select)
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
"Hello UDP server it's Sinu from talker.c"
```

### 3. IP Lookup Tool (ShowIP)
Resolve a hostname to IP addresses.

```bash
./Src/showip google.com
```

## End Chapter 1-6 (Basic)

## Next (Chapter 7 - Slightly Advanced Techniques)

### 4&5. Chat Server with poll() and select()
Both implementations achieve the same goal using different internal mechanisms.

- **Properties:** Uses **I/O Multiplexing** to manage multiple clients within a **single process**.
- **Efficiency:** Unlike `fork()`, they monitor an array/set of file descriptors. The process "sleeps" and consumes **zero CPU cycles** until the OS signals that data is ready to be read.
- **Common Behavior:**
    - **Broadcasting:** Messages sent by one client are relayed to all others (except the sender).
    - **UX:** Non-blocking feel without the "busy-wait" overhead.

#### Implementation Summary
| Feature | **select()** | **poll()** |
| :--- | :--- | :--- |
| **Mechanism** | Uses **bitwise operations** (`FD_SET`, `FD_ISSET`) to manage connection states. | Uses **dynamic array management** with `realloc()` for scalable connections. |
| **Pros** | Simple, standard, highly portable. | No fixed limit on connections (only system memory). |

---

#### Terminal 1: Start the Server (Choose one)

**The Classic select()**
```bash
./Src/selectserver
```

**The Modern poll()**
```bash
./Src/pollserver
```
(Both servers listen on port 9034)

#### Terminal 2, 3, 4...: Connect multiple clients
You can use telnet or nc (netcat) to simulate multiple users:

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

#### The result on Terminal 1 (Server) should be

selectserver
```bash
selectserver: new connection from 127.0.0.1 on socket 4
selectserver: new connection from 127.0.0.1 on socket 5
selectserver: recv from fd 4: Sinu: Networking is cool!
```

pollserver

```bash
server: waiting for connections...
server: new connection from 127.0.0.1 on socket 4
server: new connection from 127.0.0.1 on socket 5
server: recv from fd 4: Sinu: Networking is cool!
```

#### The result on other Client Terminals (Terminal 1, 2, 3, 4...) should be

```bash
Sinu: Networking is cool!
```

### And if you wonder Why does my client start at Socket 4?
In Unix-like systems, every new process starts with three default File Descriptors (FD):

    0 (stdin): Keyboard input.
    1 (stdout): Standard output to screen.
    2 (stderr): Error output to screen.

When our server runs:

    FD 3 is assigned to the Listener Socket (the "Front Door").
    FD 4 is assigned to the First Client that connects.
    Subsequent clients will be assigned FD 5, 6, and so on.
    