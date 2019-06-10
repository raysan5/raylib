/**********************************************************************************************
*
*   rnet - Provides cross-platform network defines, macros etc
*
*   DEPENDENCIES:
*       <limits.h>    - Used for cross-platform type specifiers
*
*   INSPIRED BY:
*       SFML Sockets - https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1Socket.php
*       SDL_net - https://www.libsdl.org/projects/SDL_net/
*       BSD Sockets - https://www.gnu.org/software/libc/manual/html_node/Sockets.html
*       BEEJ - https://beej.us/guide/bgnet/html/single/bgnet.html
*       Winsock2 - https://docs.microsoft.com/en-us/windows/desktop/api/winsock2
*
*
*   CONTRIBUTORS:
*       Jak Barnes (github: @syphonx) (Feb. 2019) - Initial version
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2019 Jak Barnes (github: @syphonx) and Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RNET_H
#define RNET_H

#include <limits.h>         // Required for limits
#include <inttypes.h>       // Required for platform type sizes

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Undefine any conflicting windows.h symbols
// If defined, the following flags inhibit definition of the indicated items.
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
#define NONLS             // All NLS defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE          // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND              // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM              // COMM driver routines
#define NOKANJI              // Kanji support stuff.
#define NOHELP              // Help engine interface.
#define NOPROFILER          // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions
#define MMNOSOUND

//----------------------------------------------------------------------------------
// Platform type definitions
// From: https://github.com/DFHack/clsocket/blob/master/src/Host.h
//----------------------------------------------------------------------------------
 
#ifdef WIN32
typedef int socklen_t;
#endif  

#ifndef RESULT_SUCCESS
#    define RESULT_SUCCESS 0
#endif // RESULT_SUCCESS

#ifndef RESULT_FAILURE
#    define RESULT_FAILURE 1
#endif // RESULT_FAILURE

#ifndef htonll
#    ifdef _BIG_ENDIAN
#        define htonll(x) (x)
#        define ntohll(x) (x)
#    else
#        define htonll(x) ((((uint64) htonl(x)) << 32) + htonl(x >> 32))
#        define ntohll(x) ((((uint64) ntohl(x)) << 32) + ntohl(x >> 32))
#    endif // _BIG_ENDIAN
#endif     // htonll

//----------------------------------------------------------------------------------
// Platform specific network includes
// From: https://github.com/SDL-mirror/SDL_net/blob/master/SDLnetsys.h
//----------------------------------------------------------------------------------

// Include system network headers
#if defined(_WIN32)
    #define __USE_W32_SOCKETS
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>
    #define IPTOS_LOWDELAY 0x10
#else   // Unix
    #include <sys/types.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/ioctl.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <net/if.h>
    #include <netdb.h>
    #include <netinet/tcp.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

#ifndef INVALID_SOCKET
    #define INVALID_SOCKET ~(0)
#endif

#ifndef __USE_W32_SOCKETS
    #define closesocket close
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#ifdef __USE_W32_SOCKETS
    #ifndef EINTR
        #define EINTR WSAEINTR
    #endif
#endif

//----------------------------------------------------------------------------------
// Module defines
//----------------------------------------------------------------------------------

// Network connection related defines
#define SOCKET_MAX_SET_SIZE                     (32)   // Maximum sockets in a set
#define SOCKET_MAX_QUEUE_SIZE                   (16)   // Maximum socket queue size
#define SOCKET_MAX_SOCK_OPTS (4)        // Maximum socket options
#define SOCKET_MAX_UDPCHANNELS (32)        // Maximum UDP channels
#define SOCKET_MAX_UDPADDRESSES (4)        // Maximum bound UDP addresses 


// Network address related defines
#define ADDRESS_IPV4_ADDRSTRLEN                 (22)   // IPv4 string length
#define ADDRESS_IPV6_ADDRSTRLEN                 (65)   // IPv6 string length
#define ADDRESS_TYPE_ANY                        (0)    // AF_UNSPEC
#define ADDRESS_TYPE_IPV4                       (2)    // AF_INET
#define ADDRESS_TYPE_IPV6                       (23)   // AF_INET6
#define ADDRESS_MAXHOST                         (1025) // Max size of a fully-qualified domain name
#define ADDRESS_MAXSERV                         (32)   // Max size of a service name

// Network address related defines
#define ADDRESS_ANY                             ((unsigned long) 0x00000000)
#define ADDRESS_LOOPBACK                        (0x7f000001)
#define ADDRESS_BROADCAST                       ((unsigned long) 0xffffffff)
#define ADDRESS_NONE                            (0xffffffff)

// Address resolution related defines
#if defined(_WIN32)
    #define ADDRESS_INFO_PASSIVE                (0x00000001)  // Socket address will be used in bind() call
    #define ADDRESS_INFO_CANONNAME              (0x00000002)  // Return canonical name in first ai_canonname
    #define ADDRESS_INFO_NUMERICHOST            (0x00000004)  // Nodename must be a numeric address string
    #define ADDRESS_INFO_NUMERICSERV            (0x00000008)  // Servicename must be a numeric port number
    #define ADDRESS_INFO_DNS_ONLY               (0x00000010)  // Restrict queries to unicast DNS only (no LLMNR, netbios, etc.)
    #define ADDRESS_INFO_ALL                    (0x00000100)  // Query both IP6 and IP4 with AI_V4MAPPED
    #define ADDRESS_INFO_ADDRCONFIG             (0x00000400)  // Resolution only if global address configured
    #define ADDRESS_INFO_V4MAPPED               (0x00000800)  // On v6 failure, query v4 and convert to V4MAPPED format
    #define ADDRESS_INFO_NON_AUTHORITATIVE      (0x00004000)  // LUP_NON_AUTHORITATIVE
    #define ADDRESS_INFO_SECURE                 (0x00008000)  // LUP_SECURE
    #define ADDRESS_INFO_RETURN_PREFERRED_NAMES (0x00010000)  // LUP_RETURN_PREFERRED_NAMES
    #define ADDRESS_INFO_FQDN                   (0x00020000)  // Return the FQDN in ai_canonname
    #define ADDRESS_INFO_FILESERVER             (0x00040000)  // Resolving fileserver name resolution
    #define ADDRESS_INFO_DISABLE_IDN_ENCODING   (0x00080000)  // Disable Internationalized Domain Names handling
    #define ADDRESS_INFO_EXTENDED               (0x80000000)  // Indicates this is extended ADDRINFOEX(2/..) struct
    #define ADDRESS_INFO_RESOLUTION_HANDLE      (0x40000000)  // Request resolution handle
#endif

// Network resolution related defines
#define NAME_INFO_DEFAULT                       (0x00) // No flags set
#define NAME_INFO_NOFQDN                        (0x01) // Only return nodename portion for local hosts
#define NAME_INFO_NUMERICHOST                   (0x02) // Return numeric form of the host's address
#define NAME_INFO_NAMEREQD                      (0x04) // Error if the host's name not in DNS
#define NAME_INFO_NUMERICSERV                   (0x08) // Return numeric form of the service (port #)
#define NAME_INFO_DGRAM                         (0x10) // Service is a datagram service

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Boolean type
#if defined(__STDC__) && __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool)
    typedef enum { false, true } bool;
#endif

// Network typedefs
typedef uint32_t SocketChannel;
typedef struct _AddressInformation *AddressInformation;
typedef struct _SocketAddress *SocketAddress;
typedef struct _SocketAddressIPv4 *SocketAddressIPv4;
typedef struct _SocketAddressIPv6 *SocketAddressIPv6;
typedef struct _SocketAddressStorage *SocketAddressStorage;

// IPAddress definition (in network byte order)
typedef struct IPAddress {
    unsigned long  host; /* 32-bit IPv4 host address */
    unsigned short port; /* 16-bit protocol port */
} IPAddress;

// An option ID, value, sizeof(value) tuple for setsockopt(2).
typedef struct SocketOpt {
    int id;
    void *value;
    int valueLen;
} SocketOpt;

typedef enum {
    SOCKET_TCP = 0, // SOCK_STREAM
    SOCKET_UDP = 1  // SOCK_DGRAM
} SocketType;

typedef struct UDPChannel {
    int numbound; // The total number of addresses this channel is bound to
    IPAddress address[SOCKET_MAX_UDPADDRESSES]; // The list of remote addresses this channel is bound to
} UDPChannel;

typedef struct Socket {
    int  ready;    // Is the socket ready? i.e. has information
    int  status;   // The last status code to have occured using this socket
    bool isServer; // Is this socket a server socket (i.e. TCP/UDP Listen Server)
    SocketChannel channel; // The socket handle id
    SocketType    type;    // Is this socket a TCP or UDP socket?
    bool          isIPv6;  // Is this socket address an ipv6 address?
    SocketAddressIPv4 addripv4; // The host/target IPv4 for this socket (in network byte order)
    SocketAddressIPv6 addripv6; // The host/target IPv6 for this socket (in network byte order)

    struct UDPChannel binding[SOCKET_MAX_UDPCHANNELS]; // The amount of channels (if UDP) this socket is bound to
} Socket;

typedef struct SocketSet {
    int numsockets;
    int maxsockets;
    struct Socket **sockets;
} SocketSet;

typedef struct SocketDataPacket {
    int            channel; // The src/dst channel of the packet
    unsigned char *data;    // The packet data
    int            len;     // The length of the packet data
    int            maxlen;  // The size of the data buffer
    int            status;  // packet status after sending
    IPAddress address; // The source/dest address of an incoming/outgoing packet
} SocketDataPacket;

// Configuration for a socket.
typedef struct SocketConfig {
    char *     host;   // The host address in xxx.xxx.xxx.xxx form
    char *     port;   // The target port/service in the form "http" or "25565"
    bool       server; // Listen for incoming clients?
    SocketType type;   // The type of socket, TCP/UDP
    bool       nonblocking;  // non-blocking operation?
    int        backlog_size; // set a custom backlog size
    SocketOpt  sockopts[SOCKET_MAX_SOCK_OPTS];
} SocketConfig;

// Result from calling open with a given config.
typedef struct SocketResult {
    int status;
    Socket *socket;
} SocketResult;

// Packet type
typedef struct Packet {
    uint32_t size; // The total size of bytes in data
    uint32_t offs; // The offset to data access
    uint32_t maxs; // The max size of data
    uint8_t *data; // Data stored in network byte order
} Packet;


#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Initialisation and cleanup
bool InitNetwork(void);
void CloseNetwork(void);

// Address API
void ResolveIP(const char *ip, const char *service, int flags, char *outhost, char *outserv);
int ResolveHost(const char *address, const char *service, int addressType, int flags, AddressInformation *outAddr);
int GetAddressFamily(AddressInformation address);
int GetAddressSocketType(AddressInformation address);
int GetAddressProtocol(AddressInformation address);
char* GetAddressCanonName(AddressInformation address);
char* GetAddressHostAndPort(AddressInformation address, char *outhost, int *outport);
void PrintAddressInfo(AddressInformation address);

// Address Memory API
AddressInformation AllocAddress();
void FreeAddress(AddressInformation *addressInfo);
AddressInformation *AllocAddressList(int size);

// Socket API
bool SocketCreate(SocketConfig *config, SocketResult *result);
bool SocketBind(SocketConfig *config, SocketResult *result);
bool SocketListen(SocketConfig *config, SocketResult *result);
bool SocketConnect(SocketConfig *config, SocketResult *result);
Socket *SocketAccept(Socket *server, SocketConfig *config);

// UDP Socket API
int SocketSetChannel(Socket *socket, int channel, const IPAddress *address);
void SocketUnsetChannel(Socket *socket, int channel);

// UDP DataPacket API
SocketDataPacket *AllocPacket(int size);
int ResizePacket(SocketDataPacket *packet, int newsize);
void FreePacket(SocketDataPacket *packet);
SocketDataPacket **AllocPacketList(int count, int size);
void FreePacketList(SocketDataPacket **packets);

// General Socket API
int SocketSend(Socket *sock, const void *datap, int len);
int SocketReceive(Socket *sock, void *data, int maxlen);
void SocketClose(Socket *sock);
SocketAddressStorage SocketGetPeerAddress(Socket *sock);
char* GetSocketAddressHost(SocketAddressStorage storage);
short GetSocketAddressPort(SocketAddressStorage storage);

// Socket Memory API
Socket *AllocSocket();
void FreeSocket(Socket **sock);
SocketResult *AllocSocketResult();
void FreeSocketResult(SocketResult **result);
SocketSet *AllocSocketSet(int max);
void FreeSocketSet(SocketSet *sockset);

// Socket I/O API
bool IsSocketReady(Socket *sock);
bool IsSocketConnected(Socket *sock);
int AddSocket(SocketSet *set, Socket *sock);
int RemoveSocket(SocketSet *set, Socket *sock);
int CheckSockets(SocketSet *set, unsigned int timeout);

// Packet API 
void PacketSend(Packet *packet);
void PacketReceive(Packet *packet);
void PacketWrite8(Packet *packet, uint16_t value);
void PacketWrite16(Packet *packet, uint16_t value);
void PacketWrite32(Packet *packet, uint32_t value);
void PacketWrite64(Packet *packet, uint64_t value);
uint16_t PacketRead8(Packet *packet);
uint16_t PacketRead16(Packet *packet);
uint32_t PacketRead32(Packet *packet);
uint64_t PacketRead64(Packet *packet);

#ifdef __cplusplus
}
#endif

#endif  // RNET_H