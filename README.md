# Network Programming in C (Chapter 1-6: Basic)

Learning to build network applications and a Mini-Redis clone using C, based on [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/).

### Relation to Mini-Redis

The concepts in this chapter directly apply to the Mini-Redis project:
- TCP stream handling → RESP parsing
- Work buffer → command framing
- Byte order → integer and bulk string length fields
- Encapsulation → Redis protocol design


## Introduction Chapter 1-6 (Basic)

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
├── Makefile                 # Build all targets (examples + miniredis)
│
├── examples/                # Beej's Guide Network Programming Examples
│   ├── tcp/                 # TCP examples (server, client, select/poll servers)
│   ├── udp/                 # UDP examples (listener, talker)
│   └── utils/               # Network utilities (showip)
│
├── lib/                     # Shared Network Library
│   ├── network_lib.h
│   └── network_lib.c
│
└── miniredis/               # Main Project Implementation
    ├── Makefile
    ├── include/             # Header files
    ├── src/                 # Source code
    └── README.md            # Mini-Redis specific documentation
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
./examples/tcp/server
```
(Server will listen on port 3490)

#### Terminal 2: Run Client

```bash
./examples/tcp/client 127.0.0.1
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
./examples/udp/listener
```
(Waiting for incoming packets on port 4950)

#### Terminal 2: Run Talker (Client)
```bash
./examples/udp/talker localhost "Hello UDP server it's Sinu from talker.c"
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
./examples/utils/showip google.com
```

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
./examples/tcp/selectserver
```

**The Modern poll()**
```bash
./examples/tcp/pollserver
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
pollserver: new connection from 127.0.0.1 on socket 4
pollserver: new connection from 127.0.0.1 on socket 5
pollserver: recv from fd 4: Sinu: Networking is cool!
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
    
## Serialization—How to Pack Data
this topic is about how to pack data to send it over the network.
Before go straight to the point i will explain byte order problem.

### [Byte Order](https://beej.us/guide/bgnet/)
so there are two ways to represent the byte order of an integer.

1. Big-endian
2. Little-endian

The thing is, everyone in the Internet world has generally agreed that if you want to represent the two-byte hex number say 0x1234, 

- you should send it as 0x12 0x34 call big-endian

- unfortunately, few computer that use little-endian it will be represented as 0x34 0x12.

So we need to convert the byte order of an integer to network byte order before sending it over the network.
And the functions to convert the byte order of an integer are:
1. htonl: host to network long
2. ntohl: network to host long
3. htons: host to network short
4. ntohs: network to host short

Stand for :h = host, n = network, s = short(2 bytes / 16-bit), l = longss(4 bytes / 32-bit)

#### We now turn to the main topic. 
This section intentionally dives deeper into IEEE-754 to clarify how floating-point values are serialized at the bit level.

This source code is from [beej.us/guide/bgnet/](https://beej.us/guide/bgnet/)

```c
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))
/*
unsigned bits: the number of bits in the floating point number
unsigned expbits: the number of bits in the exponent
*/
uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    /*
    long double fnorm; Normalized form of f is a number between 1 and 2
    int shift; Exponent shift tell us how many times we need to shift the number to get the normalized form
    long long sign, exp, significand;
    Sign: Sign bit is 1 if the number is negative, 0 if the number is positive
    Exp: Biased exponent is the exponent of the number + bias
    Significand: Mantissa is the significand of the number
    */
    long double fnorm; 
    int shift;
    long long sign, exp, significand;

    // -1 for sign bit and expbits for exponent we're left with bits for significand
    unsigned significandbits = bits - expbits - 1;

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    /* IMPORTANT 
    get the normalized form of f and track the exponent
    the number should be in 1.xxx * 2^exp form
    */
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    //we know that the number will be in [1,2) range so we can subtract 1 to get the normalized form and maximize the precision
    fnorm = fnorm - 1.0;

    /* calculate the binary form (non-float) of the significand data
    we will shift 1 to the left to get the significand and multiply it by the significand fraction to get the significand in integer form then add 0.5 to round it
    */
    significand = fnorm * ((1LL<<significandbits) + 0.5f);
    /*  
    = (fnorm*2^significandbits) + (fnorm * 0.5f)
    note: In my opinion this line of code is not correct. The Correct Way for significand is:
    significand = (fnorm * (1LL<<significandbits)) + 0.5f;
    but I think beej's though fnorm is in [1,2) range when we multiply it with 0.5f we will got 0.5 -1.0 range and get the almost same result anyway.
    */

    /* get the biased exponent
    shift will be negative if the number is less than 1 and positive if the number is greater than 1
    we don't want exponent to be negative so we add bias to it   
    (1<<(expbits-1)) - 1 = 2^(expbits-1) - 1 = bias 
    left shift 1 by n bits is equivalent to multiplying by 2^n
    */
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    /*
    pack the sign, exponent, and significand into a 64-bit integer
    sign is the most significant bit
    exponent is the next expbits bits
    significand is the remaining bits
    */
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    /*
    result: the result of the unpack operation
    shift: the exponent shift
    bias: the exponent bias
    */
    long double result;
    long long shift;
    unsigned bias;

    // -1 for sign bit and expbits for exponent
    unsigned significandbits = bits - expbits - 1;

    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask using bitwise AND to get the significand
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    /*
    i>>significandbits: shift the bits to the right by significandbits to get the exponent but we still have sign bit
    ex 1 << 8 = 100000000 (binary)
    (1 << 8) - 1 = 011111111 (binary)
    get rid of sign bit and get the exponent(+bias) so don't forget to subtract bias and we will get the shift ex we got (2) means exponent is 2^2
    */
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;
```
    
#### More advanced implementations can be found at Beej’s Guide to Network Programming With these fundamentals established, we now proceed to the next topic.

## Son of Data Encapsulation
### The TCP Stream Problem (Why Do We Need This?)

- TCP (SOCK_STREAM) behaves like a continuous byte stream. When multiple messages are sent back-to-back, the receiver has no inherent way to know where one message ends and the next one begins.

#### Example Scenario

- User A (name: tom) sends: "Hi"

- User B (name: Benjamin) sends: "Hey guys what is up?"

#### What the Server May Receive

Because TCP is a stream, data may arrive concatenated, for example:

- t o m H i B e n j a m i n H e y g u y s w h a t i s u p ?

#### The Problem
The server cannot determine: Whether tomHi is a single message, or
tom is the sender name and Hi is the message body

TCP does not preserve message boundaries.

### The Solution: Defining a Packet Structure
To solve this, we must define our own packet format by adding a header in front of every message.
This technique is called Data Encapsulation.

Think of the header as the address on an envelope, it tells the receiver how to interpret the data that follows.

#### Example Protocol Design
- Length (1 byte)
    - Indicates the total packet length (Name + Chat Data, excluding the Length byte itself)
- Name (8 bytes)
    - Sender name
    - If shorter than 8 characters, pad with \0 (null bytes)
- Chat Data (n bytes)
    - Message content
    - Maximum length: 128 bytes

#### Example Encapsulated Packet (Hex Dump)
- Packet 1: tom says "Hi"
    - Length: 0A (10 bytes = 8 bytes name + 2 bytes message)
    - Name: "tom" + padding
    - Data: "Hi"
```bash
0A
74 6F 6D 00 00 00 00 00
48 69
```
Combined:
0A 74 6F 6D 00 00 00 00 00 48 69

### Sending & Receiving Data
Sending Side 
Use sendall() to ensure the entire packet (header + body) is transmitted 
Always convert numeric fields (e.g., Length, integers) to network byte order (Big-Endian) Receiving Side (The Hard Part)

Receiving data with recv() introduces two major challenges:

1. Partial Packet

    - A single recv() call may return:
        - Only the header
        - Header + name
        - Incomplete message body

2. Multiple Packets at Once

    - A single recv() call may return:
        - One full packet plus
        - Part (or all) of the next packet

### Work Buffer Strategy
To handle these issues, Beej recommends using a Work Buffer (a large temporary buffer) on the receiving side.

#### Processing Steps

-  Receive Data
Use recv() to append incoming bytes to the end of the work buffer.

- Check Packet Completeness
Read the first byte (Length header) → expected packet size = L
Check if buffer size ≥ L + 1 (the +1 accounts for the Length byte)

- Process 
    - If incomplete: Do nothing and wait for more data.
    - If complete: Extract (unpack) L + 1 bytes
    - Process the packet Shift remaining bytes (which may belong to the next packet) to the front of the buffer

- Repeat
Continue steps 2–3 until the buffer no longer contains a full packet

### Best Practices 
- Work Buffer Size
If a packet is larger than the work buffer, it can never be received
Always define a maximum packet size
Example: max packet = 137 byte
Allocate a buffer large enough to hold multiple packets
e.g., 4096 bytes

- memmove() vs Circular Buffer
The article uses buffer shifting with memmove()
Simple but CPU-expensive for large buffers
In production systems:
Prefer a Circular Buffer (Ring Buffer)
Avoids costly memory moves

- Sanity Checks (Security)
If the Length field is:
0, or
larger than the defined maximum (e.g., 255 when max is 128)
This may indicate malformed data or an attack
Recommended: Immediately drop the client connection

### Key Observations
- The Header Is a Contract
The header does not have to contain only Length. It can also include:
    - Packet ID
    - Message type (e.g., 1 = Login, 2 = Chat, 3 = Logout)
    - Timestamp
    - Checksum

## Broadcast Packets
So far, we have been using Unicast (one-to-one communication). But sometimes, you want to send a message to everyone on the network at once. This is called Broadcasting.

- Supported Protocol: UDP only (TCP cannot broadcast because it requires a specific connection state).

- Supported IP Version: Standard IPv4 only (IPv6 uses Multicast instead).

### The Safety Switch (SO_BROADCAST)
You cannot simply send data to a broadcast address by default. The OS protects against accidental network flooding. You must enable the SO_BROADCAST socket option first, like flipping a safety switch on a missile launcher.

```c
int broadcast = 1;
// "1" enables the option
if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
    perror("setsockopt (SO_BROADCAST)");
    exit(1);
}
```

### Destination: Where to send?
There are two ways to specify the destination address for a broadcast:
- Specific Subnet Broadcast (Recommended) Send to the last IP address of your subnet.
    - Logic: Network Number OR (NOT Netmask)
    - Example:
        ```
        IP: 192.168.1.10
        Netmask: 255.255.255.0
        Broadcast Address: 192.168.1.255
        Result: Every machine on the 192.168.1.x network receives the packet.
        ```
- Global Broadcast (Not Recommended) Address: 255.255.255.255 (INADDR_BROADCAST)
- Behavior: Sends to everyone on the local LAN.
- Limitation: Routers typically block these packets from leaving the local network to prevent Internet-wide flooding.

### The Dangers of Broadcasting
- Performance Hit: Every machine on the LAN must receive the packet and pass it up to the CPU to check if it's listening on that port.
- Network Load: Excessive broadcasting can slow down the entire network (a common complaint with early multiplayer games like DOOM).
- Firewalls: Local firewalls often block broadcast packets by default.

### Summary
- Use Broadcast for service discovery (finding a server) or time synchronization.
- Always use UDP.
- Enable SO_BROADCAST.
- Use sparingly to avoid flooding the network.

## End