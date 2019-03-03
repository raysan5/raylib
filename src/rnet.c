/*********************************************************************************************
*
*   rnet - A simple and easy-to-use network module for raylib
*
*   FEATURES:
*       - Manage network stuff
*
*   DEPENDENCIES:
*       raylib.h    - TraceLog
*       sysnet.h    - platform-specific network includes
*
*   CONTRIBUTORS:
*       Jak Barnes (github: @syphonx) (Feb. 2019):
*           - Initial version
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2019 Ramon Santamaria (@raysan5)
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

//----------------------------------------------------------------------------------
// Check if config flags have been externally provided on compilation line
//----------------------------------------------------------------------------------

#if !defined(EXTERNAL_CONFIG_FLAGS)
#	include "config.h" // Defines module configuration flags
#endif

//----------------------------------------------------------------------------------
// Module defines
//----------------------------------------------------------------------------------
#define SOCKET_BACKLOG_SIZE (20)

//----------------------------------------------------------------------------------
// Module dependencies
//----------------------------------------------------------------------------------

#include "raylib.h"
#include "sysnet.h"

//----------------------------------------------------------------------------------
// Module variables
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Global module forward declarations
//----------------------------------------------------------------------------------

static bool  IsSocketValid(Socket* sock);
static void  SocketSetLastError(int err);
static int   SocketGetLastError();
static char* SocketGetLastErrorString();
static char* SocketErrorCodeToString(int err);
static bool  SocketSetDefaults(SocketConfig* config);
static bool  InitSocket(Socket* outsock, struct addrinfo* addr);
static bool  CreateSocket(SocketConfig* config, SocketResult* outresult);
static bool  SocketSetBlocking(Socket* out);
static bool  SocketSetNonBlocking(Socket* out);
static bool  SocketSetOptions(SocketConfig* config, Socket* channel);
static void* GetSocketAddressPtr(struct sockaddr* sa);
static void* GetSocketPortPtr(struct sockaddr* sa);
static void  SocketSetHints(SocketConfig* cfg, struct addrinfo* hints);
static bool  IsIPv4Address(const char* ip);
static bool  IsIPv6Address(const char* ip);
static char* SocketAddressToString(struct sockaddr* sockaddr, char buffer[], int* port);
static void PrintSocket(struct sockaddr* addr, const int family, const int socktype, const int protocol);
static bool FillIPv4SockAddress(struct sockaddr_in* sa, const char* host, unsigned short port);
static bool FillIPv6SockAddress(struct sockaddr_in6* sa, const char* host, unsigned short port);

//----------------------------------------------------------------------------------
// Global module implementationd
//----------------------------------------------------------------------------------

static bool FillIPv4SockAddress(struct sockaddr_in* sa, const char* host, unsigned short port)
{
	sa->sin_family = AF_INET;
	sa->sin_port   = htons(port);
	int result     = inet_pton(AF_INET, host, &(sa->sin_addr));
	if (result <= 0) {
		if (result == 0) {
			TraceLog(LOG_WARNING, "Input not provided in presentation format.");
			return false;
		}
	}
	return true;
}

static bool FillIPv6SockAddress(struct sockaddr_in6* sa, const char* host, unsigned short port)
{
	sa->sin6_family = AF_INET6;
	sa->sin6_port   = htons(port);
	int result      = inet_pton(AF_INET6, host, &(sa->sin6_addr));
	if (result <= 0) {
		if (result == 0) {
			TraceLog(LOG_WARNING, "Input not provided in presentation format.");
			return false;
		}
	}
	return true;
}

// Print socket information
static void PrintSocket(struct sockaddr* addr, const int family, const int socktype, const int protocol)
{
	struct sockaddr* sockaddr_ip;
	char             ip[INET6_ADDRSTRLEN]; // Enough pace to hold a IPv6 string
	int              port;
	switch (family) {
		case AF_UNSPEC: {
			TraceLog(LOG_DEBUG, "\tFamily: Unspecified");
		} break;
		case AF_INET: {
			TraceLog(LOG_DEBUG, "\tFamily: AF_INET (IPv4)");
			TraceLog(LOG_INFO, "\t- IPv4 address %s", SocketAddressToString(addr, ip, &port));
		} break;
		case AF_INET6: {
			TraceLog(LOG_DEBUG, "\tFamily: AF_INET6 (IPv6)");
			TraceLog(LOG_INFO, "\t- IPv6 address %s", SocketAddressToString(addr, ip, &port));
		} break;
		case AF_NETBIOS: {
			TraceLog(LOG_DEBUG, "\tFamily: AF_NETBIOS (NetBIOS)");
		} break;
		default: {
			TraceLog(LOG_DEBUG, "\tFamily: Other %ld", family);
		} break;
	}
	TraceLog(LOG_DEBUG, "\tSocket type:");
	switch (socktype) {
		case 0: TraceLog(LOG_DEBUG, "\t- Unspecified"); break;
		case SOCK_STREAM:
			TraceLog(LOG_DEBUG, "\t- SOCK_STREAM (stream)");
			break;
		case SOCK_DGRAM:
			TraceLog(LOG_DEBUG, "\t- SOCK_DGRAM (datagram)");
			break;
		case SOCK_RAW: TraceLog(LOG_DEBUG, "\t- SOCK_RAW (raw)"); break;
		case SOCK_RDM:
			TraceLog(LOG_DEBUG, "\t- SOCK_RDM (reliable message datagram)");
			break;
		case SOCK_SEQPACKET:
			TraceLog(LOG_DEBUG, "\t- SOCK_SEQPACKET (pseudo-stream packet)");
			break;
		default: TraceLog(LOG_DEBUG, "\t- Other %ld", socktype); break;
	}
	TraceLog(LOG_DEBUG, "\tProtocol:");
	switch (protocol) {
		case 0: TraceLog(LOG_DEBUG, "\t- Unspecified"); break;
		case IPPROTO_TCP: TraceLog(LOG_DEBUG, "\t- IPPROTO_TCP (TCP)"); break;
		case IPPROTO_UDP: TraceLog(LOG_DEBUG, "\t- IPPROTO_UDP (UDP)"); break;
		default: TraceLog(LOG_DEBUG, "\t- Other %ld", protocol); break;
	}
}

// Convert network ordered socket address to human readable string (127.0.0.1)
static char* SocketAddressToString(struct sockaddr* sockaddr)
{
	static ipv6[INET6_ADDRSTRLEN];
	switch (sockaddr->sa_family) {
		case AF_INET: {
			struct sockaddr_in* s = ((struct sockaddr_in*) sockaddr);
			return inet_ntop(AF_INET, &s->sin_addr, ipv6, INET_ADDRSTRLEN);
		} break;
		case AF_INET6: {
			struct sockaddr_in6* s = ((struct sockaddr_in6*) sockaddr);
			return inet_ntop(AF_INET6, &s->sin6_addr, ipv6, INET6_ADDRSTRLEN);
		} break;
		default: {
			return NULL;
		} break;
	}
}

//
static bool IsIPv4Address(const char* ip)
{
	struct sockaddr_in sa;
	int                result = inet_pton(AF_INET, ip, &(sa.sin_addr));
	return result != 0;
}

//
static bool IsIPv6Address(const char* ip)
{
	struct sockaddr_in6 sa;
	int                 result = inet_pton(AF_INET6, ip, &(sa.sin6_addr));
	return result != 0;
}

//
void* GetSocketPortPtr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_port);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_port);
}

//
void* GetSocketAddressPtr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

//
static bool IsSocketValid(Socket* sock)
{
	if (sock != NULL) { return (sock->channel != INVALID_SOCKET); }
	return false;
}

// Sets the error code that can be retrieved through the WSAGetLastError function.
static void SocketSetLastError(int err)
{
#if PLATFORM == PLATFORM_WINDOWS
	WSASetLastError(err);
#else
	errno = err;
#endif
}

// Returns the error status for the last Sockets operation that failed
static int SocketGetLastError()
{
#if PLATFORM == PLATFORM_WINDOWS
	return WSAGetLastError();
#else
	return errno;
#endif
}

// Returns a human-readable string representing the last error message
static char* SocketGetLastErrorString()
{
	return SocketErrorCodeToString(SocketGetLastError());
}

// Returns a human-readable string representing the error message (err)
static char* SocketErrorCodeToString(int err)
{
#if PLATFORM == PLATFORM_WINDOWS
	static char gaiStrErrorBuffer[GAI_STRERROR_BUFFER_SIZE];
	sprintf(gaiStrErrorBuffer, "%ws", gai_strerror(err));
	return gaiStrErrorBuffer;
#else
	return gai_strerror(err);
#endif
}

//
static bool SocketSetDefaults(SocketConfig* config)
{
	if (config->backlog_size == 0) {
		config->backlog_size = SOCKET_BACKLOG_SIZE;
	}

	return true;
}

// Create the socket channel
static bool InitSocket(Socket* sock, struct addrinfo* addr)
{
	switch (sock->type) {
		case SOCKET_TCP: sock->channel = socket(AF_INET, SOCK_STREAM, 0); break;
		case SOCKET_UDP: sock->channel = socket(AF_INET, SOCK_DGRAM, 0); break;
		default: break;
	}
	return IsSocketValid(sock);
}

//	CreateSocket() - Interally called by OpenSocket()
//
//	This here is the bread and butter of the socket API, This function will
//	attempt to open a socket, bind and listen to it based on the config passed in
//
//	SocketConfig* config - Configuration for which socket to open
//	SocketResult* result - The results of this function (if any, including errors)
//
//	e.g.
//	SocketConfig server_cfg = {				SocketConfig client_cfg = {
//		.host        = "127.0.0.1",				.host = "127.0.0.1",
//		.port        = 8080,					.port = 8080,
//		.server      = true,				};
//		.nonblocking = true,
//	};
//	SocketResult server_res;				SocketResult client_res;
static bool CreateSocket(SocketConfig* config, SocketResult* outresult)
{
	bool             success = true;
	int              addrstatus;
	struct addrinfo  hints; // Address flags (IPV4, IPV6, UDP?)
	struct addrinfo* res;   // A pointer to the resulting address list
	outresult->socket->channel = INVALID_SOCKET;
	outresult->status          = RESULT_FAILURE;

	// Set the socket type
	outresult->socket->type = (config->datagram) ? SOCKET_UDP : SOCKET_TCP;

	//	Set the hints based on information in the config
	//
	//	AI_CANONNAME	Causes the ai_canonname of the result to the filled  out with the host's canonical (real) name.
	//	AI_PASSIVE:		Causes the result's IP address to be filled out with INADDR_ANY (IPv4)or in6addr_any (IPv6);
	//	Note:	This causes a subsequent call to bind() to auto-fill the IP address
	//			of the struct sockaddr with the address of the current host.
	//
	SocketSetHints(config, &hints);

	// Populate address information
	addrstatus = getaddrinfo(config->host, // e.g. "www.example.com" or IP (Can be null if AI_PASSIVE flag is set
							 config->port, // e.g. "http" or port number
							 &hints,       // e.g. SOCK_STREAM/SOCK_DGRAM
							 &res          // The struct to populate
	);

	// Did we succeed?
	if (addrstatus != 0) {
		outresult->socket->status = SocketGetLastError();
		TraceLog(LOG_WARNING,
				 "Socket Error: %s",
				 SocketErrorCodeToString(outresult->socket->status));
		SocketSetLastError(0);
		TraceLog(LOG_WARNING,
				 "Failed to get resolve host %s:%s: %s",
				 config->host,
				 config->port,
				 SocketGetLastErrorString());
		return (success = false);
	} else {
		TraceLog(
			LOG_INFO, "Successfully resolved host %s:%s", config->host, config->port);
	}

	// Walk the address information linked-list
	struct addrinfo* it;
	for (it = res; it != NULL; it = it->ai_next) {
		// Initialise the socket
		if (!InitSocket(outresult->socket, it)) {
			outresult->socket->status = SocketGetLastError();
			TraceLog(LOG_WARNING,
					 "Socket Error: %s",
					 SocketErrorCodeToString(outresult->socket->status));
			SocketSetLastError(0);
			continue;
		}

		// Set socket options
		if (!SocketSetOptions(config, outresult->socket->channel)) {
			outresult->socket->status = SocketGetLastError();
			TraceLog(LOG_WARNING,
					 "Socket Error: %s",
					 SocketErrorCodeToString(outresult->socket->status));
			SocketSetLastError(0);
			freeaddrinfo(res);
			return (success = false);
		}
	}

	if (!IsSocketValid(outresult->socket)) {
		outresult->socket->status = SocketGetLastError();
		TraceLog(
			LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(outresult->status));
		SocketSetLastError(0);
		freeaddrinfo(res);
		return (success = false);
	}

	if (success) {
		outresult->status           = RESULT_SUCCESS;
		outresult->socket->ready    = 0;
		outresult->socket->status   = 0;
		outresult->socket->isServer = config->server;
		switch (res->ai_addr->sa_family) {
			case AF_INET: {
				struct sockaddr_in* s = ((struct sockaddr_in*) res->ai_addr);
				outresult->socket->address.host = s->sin_addr.s_addr;
				outresult->socket->address.port = s->sin_port;
			} break;
			case AF_INET6: {
				struct sockaddr_in6* s = ((struct sockaddr_in6*) res->ai_addr);
				outresult->socket->address.host = s->sin6_addr.s6_addr;
				outresult->socket->address.port = s->sin6_port;
			} break;
		}
	}
	freeaddrinfo(res);
	return success;
}

//
static bool SocketSetBlocking(Socket* sock)
{
	bool ret = true;
#if PLATFORM == PLATFORM_WINDOWS
	unsigned long mode = 0;
	ret                = ioctlsocket(sock->channel, FIONBIO, &mode);
#else
	const int flags = fcntl(sock->channel, F_GETFL, 0);
	if (!(flags & O_NONBLOCK)) {
		TraceLog(LOG_DEBUG, "Socket was already in blocking mode");
		return ret;
	}

	ret = (0 == fcntl(sock->channel, F_SETFL, (flags ^ O_NONBLOCK)));
#endif
	return ret;
}

//
static bool SocketSetNonBlocking(Socket* sock)
{
	bool ret = true;
#if PLATFORM == PLATFORM_WINDOWS
	unsigned long mode = 1;
	ret                = ioctlsocket(sock->channel, FIONBIO, &mode);
#else
	const int flags = fcntl(sock->channel, F_GETFL, 0);
	if ((flags & O_NONBLOCK)) {
		TraceLog(LOG_DEBUG, "Socket was already in non-blocking mode");
		return ret;
	}
	ret = (0 == fcntl(sock->channel, F_SETFL, (flags | O_NONBLOCK)));
#endif
	return ret;
}

//
static bool SocketSetOptions(SocketConfig* config, Socket* sock)
{
	for (int i = 0; i < SOCKET_MAX_SOCK_OPTS; i++) {
		SocketOpt* opt = &config->sockopts[i];
		if (opt->id == 0) { break; }

		if (setsockopt(sock->channel, SOL_SOCKET, opt->id, opt->value, opt->valueLen) < 0) {
			return false;
		}
	}

	return true;
}

// Set "hints" in an addrinfo struct, to be passed to getaddrinfo.
static void SocketSetHints(SocketConfig* cfg, struct addrinfo* hints)
{
	if (cfg == NULL || hints == NULL) { return; }
	memset(hints, 0, sizeof(*hints));

	// Check if the ip supplied in the config is a valid ipv4 ip ipv6 address
	if (IsIPv4Address(cfg->host)) {
		hints->ai_family = AF_INET;
		hints->ai_flags |= AI_NUMERICHOST;
	} else {
		if (IsIPv6Address(cfg->host)) {
			hints->ai_family = AF_INET6;
			hints->ai_flags |= AI_NUMERICHOST;
		} else {
			hints->ai_family = AF_UNSPEC;
		}
	}

	if (cfg->datagram) {
		hints->ai_socktype = SOCK_DGRAM;
	} else {
		hints->ai_socktype = SOCK_STREAM;
	}

	// Set passive unless UDP client
	if (!cfg->datagram || cfg->server) { hints->ai_flags = AI_PASSIVE; }
}

//----------------------------------------------------------------------------------
// Module implementation
//----------------------------------------------------------------------------------

//	Initialise the network (requires for windows platforms only)
bool InitNetwork()
{
#if PLATFORM == PLATFORM_WINDOWS
	WORD    wVersionRequested;
	WSADATA wsaData;
	int     err;

	wVersionRequested = MAKEWORD(2, 2);
	err               = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		TraceLog(LOG_WARNING, "WinSock failed to initialise.");
		return false;
	} else {
		TraceLog(LOG_INFO, "WinSock initialised.");
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		TraceLog(LOG_WARNING, "WinSock failed to initialise.");
		WSACleanup();
		return false;
	}

	return true;
#else
	return true;
#endif
}

//	Cleanup, and close the network
void CloseNetwork()
{
#if PLATFORM == PLATFORM_WINDOWS
	if (WSACleanup() == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAEINPROGRESS) {
			WSACancelBlockingCall();
			WSACleanup();
		}
	}
#endif
}

//	Protocol-independent name resolution from an address to an ANSI host name
//	and from a port number to the ANSI service name.
//
//	The flags parameter can be used to customize processing of the getnameinfo function
//
//	The following flags are available:
//
//	NAME_INFO_DEFAULT		0x00	// No flags set
//	NAME_INFO_NOFQDN		0x01	// Only return nodename portion for local hosts
//	NAME_INFO_NUMERICHOST	0x02	// Return numeric form of the host's address
//	NAME_INFO_NAMEREQD		0x04	// Error if the host's name not in DNS
//	NAME_INFO_NUMERICSERV	0x08	// Return numeric form of the service (port #)
//	NAME_INFO_DGRAM			0x10	// Service is a datagram service
char* ResolveIP(const char* ip, const char* port, int flags)
{
	// Variables
	char             host[NI_MAXHOST];
	char             service[NI_MAXSERV];
	int              status;  // Status value to return (0) is success
	struct addrinfo  hints;   // Address flags (IPV4, IPV6, UDP?)
	struct addrinfo* results; // A pointer to the resulting address list

	// Zero out the host buffer
	memset(&host, '\0', sizeof(host));
	memset(&service, '\0', sizeof(service));

	// Set the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // Either IPv4 or IPv6 (AF_INET, AF_INET6)
	hints.ai_protocol = 0; // Automatically select correct protocol (IPPROTO_TCP), (IPPROTO_UDP)

	// Populate address information
	status = getaddrinfo(ip,      // e.g. "www.example.com" or IP
						 port,    // e.g. "http" or port number
						 &hints,  // e.g. SOCK_STREAM/SOCK_DGRAM
						 &results // The struct to populate
	);

	// Did we succeed?
	if (status != 0) {
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %s", ip, port, gai_strerror(errno));
	} else {
		TraceLog(LOG_DEBUG, "Resolving... %s::%s", ip, port);
	}

	// Attempt to resolve network byte order ip to hostname
	switch (results->ai_family) {
		case AF_INET:
			status = getnameinfo(&*((struct sockaddr*) results->ai_addr),
								 sizeof(*((struct sockaddr_in*) results->ai_addr)),
								 host,
								 sizeof(host),
								 service,
								 NI_MAXSERV,
								 flags);
			break;
		case AF_INET6:
			status = getnameinfo(&*((struct sockaddr_in6*) results->ai_addr),
								 sizeof(*((struct sockaddr_in6*) results->ai_addr)),
								 host,
								 sizeof(host),
								 service,
								 NI_MAXSERV,
								 flags);
			break;
		default: break;
	}

	// Did we succeed?
	if (status != 0) {
		TraceLog(LOG_WARNING, "Failed to resolve ip %s: %s", ip, SocketGetLastErrorString());
	} else {
		TraceLog(LOG_INFO, "Successfully resolved %s::%s to %s", ip, port, host);
	}

	// Free the pointer to the data returned by addrinfo
	freeaddrinfo(results);

	// Return the resulting hostname
	return host;
}

//	Protocol-independent translation from an ANSI host name to an address
//
//	e.g.
//	const char* address	= "127.0.0.1" (local address)
//	const char* port		= "80"
char* ResolveHost(const char* address, const char* port)
{
	// Variables
	int              status;  // Status value to return (0) is success
	struct addrinfo  hints;   // Address flags (IPV4, IPV6, UDP?)
	struct addrinfo* results; // A pointer to the resulting address list
	char             ip[INET6_ADDRSTRLEN]; // Enough pace to hold a IPv6 string
	int              portptr;

	// Set the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // Either IPv4 or IPv6 (AF_INET, AF_INET6)
	hints.ai_protocol = 0; // Automatically select correct protocol (IPPROTO_TCP), (IPPROTO_UDP)

	// When the address is NULL, populate the IP for me
	if (address == NULL) { hints.ai_flags = AI_PASSIVE; }

	// Populate address information
	status = getaddrinfo(address, // e.g. "www.example.com" or IP
						 port,    // e.g. "http" or port number
						 &hints,  // e.g. SOCK_STREAM/SOCK_DGRAM
						 &results // The struct to populate
	);

	// Did we succeed?
	if (status != 0) {
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %s", address, port, gai_strerror(errno));
	} else {
		TraceLog(LOG_INFO, "Successfully resolved host %s:%s", address, port);
	}

	struct addrinfo* iterator;
	for (iterator = results; iterator != NULL; iterator = iterator->ai_next) {
		TraceLog(LOG_DEBUG, "GetAddressInformation");
		TraceLog(LOG_DEBUG, "\tFlags: 0x%x", iterator->ai_flags);
		PrintSocket(iterator->ai_addr,
					iterator->ai_family,
					iterator->ai_socktype,
					iterator->ai_protocol);
		TraceLog(LOG_DEBUG, "Length of this sockaddr: %d", iterator->ai_addrlen);
		TraceLog(LOG_DEBUG, "Canonical name: %s", iterator->ai_canonname);
	}

	// Free the pointer to the data returned by addrinfo
	freeaddrinfo(results);

	// Return the resulting hostname
	return SocketAddressToString(results->ai_addr, ip, &portptr);
}

//	This here is the bread and butter of the socket API, This function will
//	attempt to open a socket, bind and listen to it based on the config passed in
//
//	SocketConfig* config - Configuration for which socket to open
//	SocketResult* result - The results of this function (if any, including errors)
//
//	e.g.
//	SocketConfig server_cfg = {				SocketConfig client_cfg = {
//		.host        = "127.0.0.1",				.host = "127.0.0.1",
//		.port        = 8080,					.port = 8080,
//		.server      = true,				};
//		.nonblocking = true,
//	};
//	SocketResult server_res;				SocketResult client_res;
bool SocketCreate(SocketConfig* config, SocketResult* result)
{
	// Socket creation result
	bool success = true;

	// Make sure we've not received a null config or result pointer
	if (config == NULL || result == NULL) { return (success = false); }

	// Set the defaults based on the config
	if (!SocketSetDefaults(config)) {
		TraceLog(LOG_WARNING, "Configuration Error.");
		success = false;
	} else {
		// Create the socket
		if (CreateSocket(config, result)) {
			if (config->nonblocking) {
				SocketSetNonBlocking(result->socket);
			} else {
				SocketSetBlocking(result->socket);
			}
		} else {
			success = false;
		}
	}
	return success;
}

//
bool SocketListen(SocketConfig* config, SocketResult* result)
{
	bool success   = true;
	result->status = RESULT_FAILURE;

	// Only bind to sockets marked as server
	if (config->server) {
		// The sockaddr_in structure specifies the address family,
		// IP address, and port of the server to be connected to.
		struct sockaddr_in clientService;
		clientService.sin_family      = AF_INET;
		clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
		clientService.sin_port        = htons(8080);

		// Attempt to bind the socket
		if (bind(result->socket->channel, (SOCKADDR*) &clientService, sizeof(clientService)) != SOCKET_ERROR) {
			TraceLog(LOG_INFO, "Successfully bound socket.");
		} else {
			result->socket->status = SocketGetLastError();
			TraceLog(LOG_WARNING, "Socket Error: %s",
					 SocketErrorCodeToString(result->socket->status));
			SocketSetLastError(0);
			success = false;
		}

		// Don't listen on UDP sockets
		if (!config->datagram) {
			if (listen(result->socket->channel, config->backlog_size) != 0) {
				success = false;
			} else {
				TraceLog(LOG_INFO, "Started listening on socket...");
				success = true;
			}
		}

	} else {
		TraceLog(LOG_WARNING,
				 "Cannot listen on socket marked as \"Client\" in SocketConfig.");
		success = false;
	}

	if (success) {
		result->status         = RESULT_SUCCESS;
		result->socket->ready  = 0;
		result->socket->status = 0;
	}

	return success;
}

//
bool SocketConnect(SocketConfig* config, SocketResult* result)
{
	bool success   = true;
	result->status = RESULT_FAILURE;

	// Only bind to sockets marked as server
	if (config->server) {
		TraceLog(LOG_WARNING,
				 "Cannot connect to socket marked as \"Server\" in SocketConfig.");
		success = false;
	} else {
		// The sockaddr_in structure specifies the address family,
		// IP address, and port of the server to be connected to.
		struct sockaddr_in clientService;
		clientService.sin_family      = AF_INET;
		clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
		clientService.sin_port        = htons(8080);

		// Did we connect successfully?
		if (connect(result->socket->channel, (SOCKADDR*) &clientService, sizeof(clientService)) != SOCKET_ERROR) {
			TraceLog(LOG_INFO, "Successfully connected to socket.");
		} else {
			result->socket->status = SocketGetLastError();
			TraceLog(LOG_WARNING,
					 "Socket Error: %s",
					 SocketErrorCodeToString(result->socket->status));
			SocketSetLastError(0);
			success = false;
		}
	}

	if (success) {
		result->status         = RESULT_SUCCESS;
		result->socket->ready  = 0;
		result->socket->status = 0;
	}

	return success;
}

//	Closes an existing socket
//
//	SocketChannel socket - The id of the socket to close
void SocketClose(Socket* sock)
{
	if (sock != NULL) {
		if (sock->channel != INVALID_SOCKET) { closesocket(sock->channel); }
	}
}

//	The accept function permits an incoming connection attempt on a socket.
//
//	SocketChannel	listener	- The socket to listen for incoming connections on (i.e. server)
//	SocketResult*	out			- The result of this function (if any, including errors)
//
//	e.g.
//
//	SocketResult connection;
//	bool         connected = false;
//	if (!connected)
//	{
//		if (SocketAccept(server_res.socket.channel, &connection))
//		{
//			connected = true;
//		}
//	}
Socket* SocketAccept(Socket* server, SocketConfig* config)
{
	if (!server->isServer || server->type == SOCKET_UDP) { return NULL; }
	struct sockaddr_storage sock_addr;
	socklen_t               sock_alen;
	Socket*                 sock;
	int                     sock_port;
	sock          = AllocSocket();
	server->ready = 0;
	sock_alen     = sizeof(sock_addr);
	sock->channel = accept(server->channel, (struct sockaddr*) &sock_addr, &sock_alen);
	if (sock->channel == INVALID_SOCKET) {
		sock->status = SocketGetLastError();
		TraceLog(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(sock->status));
		SocketSetLastError(0);
		SocketClose(sock);
		return NULL;
	}
	(config->nonblocking) ? SocketSetNonBlocking(sock) : SocketSetBlocking(sock);
	sock->isServer = false;
	sock->ready    = 0;
	sock->type     = server->type;
	switch (sock_addr.ss_family) {
		case AF_INET: {
			struct sockaddr_in* s = ((struct sockaddr_in*) &sock_addr);
			sock->address.host    = s->sin_addr.s_addr;
			sock->address.port    = s->sin_port;
			TraceLog(LOG_INFO, "Server: Got connection from %s::%hu", SocketAddressToString(s),
					 ntohs(sock->address.port));
		} break;
		case AF_INET6: {
			struct sockaddr_in6* s = ((struct sockaddr_in6*) &sock_addr);
			sock->address.host     = s->sin6_addr.s6_addr;
			sock->address.port     = s->sin6_port;
			TraceLog(LOG_INFO, "Server: Got connection from %s::%hu", SocketAddressToString(s),
					 ntohs(sock->address.port));
		} break;
	}
	return sock;
}

//	Send 'len' bytes of 'data' over the non-server socket 'sock'
//
//	Example
int SocketSend(Socket* sock, const void* datap, int length)
{
	int                  sent    = 0;
	int                  left    = length;
	int                  status  = -1;
	int                  numsent = 0;
	const unsigned char* data    = (const unsigned char*) datap;

	// Server sockets are for accepting connections only
	if (sock->isServer) {
		TraceLog(LOG_WARNING, "Cannot send information on a server socket");
		return -1;
	}

	// Which socket are we trying to send data on
	switch (sock->type) {
		case SOCKET_TCP: {
			SocketSetLastError(0);
			do {
				length = send(sock->channel, (const char*) data, left, 0);
				if (length > 0) {
					sent += length;
					left -= length;
					data += length;
				}
			} while ((left > 0) && // While we still have bytes left to send
					 ((length > 0) || // The amount of bytes we actually sent is > 0
					  (SocketGetLastError() == WSAEINTR)) // The socket was interupted
			);

			if (length == SOCKET_ERROR) {
				sock->status = SocketGetLastError();
				TraceLog(LOG_DEBUG, "Socket Error: %s", SocketErrorCodeToString(sock->status));
				SocketSetLastError(0);
			} else {
				TraceLog(LOG_DEBUG, "Successfully sent \"%s\" (%d bytes)", datap, sent);
			}

			return sent;
		} break;
		case SOCKET_UDP: {
			struct sockaddr_in dest;
			dest.sin_family      = AF_INET;
			dest.sin_port        = sock->address.port;
			dest.sin_addr.s_addr = sock->address.host;
			SocketSetLastError(0);
			status = sendto(sock->channel, (const char*) data, left, 0, (struct sockaddr*) &dest, sizeof(dest));
			if (sent >= 0) {
				sock->status = status;
				++numsent;
			} else {
				sock->status = SocketGetLastError();
				TraceLog(LOG_DEBUG, "Socket Error: %s", SocketGetLastErrorString(sock->status));
				SocketSetLastError(0);
				return 0;
			}
			return numsent;
		} break;
		default: break;
	}
	return -1;
}

//	Receive up to 'maxlen' bytes of data over the non-server socket 'sock',
//	and store them in the buffer pointed to by 'data'.
//	This function returns the actual amount of data received.  If the return
//	value is less than or equal to zero, then either the remote connection was
//	closed, or an unknown socket error occurred.
int SocketReceive(Socket* sock, void* data, int maxlen, int timeout)
{
	int                     len     = 0;
	int                     numrecv = 0;
	int                     status  = 0;
	socklen_t               sock_len;
	struct sockaddr_storage sock_addr;
	char                    ip[INET6_ADDRSTRLEN];

	// Server sockets are for accepting connections only
	if (sock->isServer && sock->type == SOCKET_TCP) {
		sock->status = SocketGetLastError();
		TraceLog(LOG_DEBUG, "Socket Error: %s",
				 "Server sockets cannot be used to receive data");
		SocketSetLastError(0);
		return 0;
	}

	// Which socket are we trying to send data on
	switch (sock->type) {
		case SOCKET_TCP: {
			SocketSetLastError(0);
			do {
				len = recv(sock->channel, (char*) data, maxlen, 0);
			} while (SocketGetLastError() == WSAEINTR);

			if (len > 0) {
				// Who sent the packet?
				if (sock->type == SOCKET_UDP) {
					TraceLog(
						LOG_DEBUG, "Received data from: %s", inet_ntop(sock_addr.ss_family, GetSocketAddressPtr((struct sockaddr*) &sock_addr), ip, sizeof(ip)));
				}
				((unsigned char*) data)[len] = '\0'; // Add null terminating character to the end of the stream
				TraceLog(LOG_DEBUG, "Received \"%s\" (%d bytes)", data, len);
			}
			sock->ready = 0;
			return len;
		} break;
		case SOCKET_UDP: {
			SocketSetLastError(0);
			sock_len = sizeof(sock_addr);
			status   = recvfrom(sock->channel, // The receving channel
                              data,   // A pointer to the data buffer to fill
                              maxlen, // The max length of the data to fill
                              0,      // Flags
                              (struct sockaddr*) &sock_addr, // The address of the recevied data
                              &sock_len // The length of the received data address
            );
			if (status >= 0) {
				++numrecv;
			} else {
				sock->status = SocketGetLastError();
				TraceLog(LOG_DEBUG, "Socket Error: %s", SocketGetLastErrorString(sock->status));
				SocketSetLastError(0);
				return 0;
			}
			sock->ready = 0;
			return numrecv;
		} break;
	}
	return -1;
}

//
bool IsSocketReady(Socket* sock)
{
	return (sock != NULL) && (sock->ready);
}

//
SocketResult* AllocSocketResult()
{
	struct SocketResult* res;
	res = (struct SocketResult*) malloc(sizeof(*res));
	if (res != NULL) {
		memset(res, 0, sizeof(*res));
		if ((res->socket = AllocSocket()) == NULL) {
			free(res);
			res = NULL;
		}
	}
	return res;
}

//
void FreeSocketResult(SocketResult* result)
{
	if (result != NULL) { free(result); }
}

//
Socket* AllocSocket()
{
	// Allocate a socket if one already hasn't been
	struct Socket* sock;
	sock = (Socket*) malloc(sizeof(*sock));
	if (socket != NULL) {
		memset(sock, 0, sizeof(*sock));
	} else {
		TraceLog(
			LOG_WARNING, "Ran out of memory attempting to allocate a socket");
		SocketClose(sock);
		free(sock);
		sock = NULL;
	}
	return sock;
}

//
void FreeSocket(Socket* sock)
{
	SocketClose(sock);
	free(sock);
}

//
SocketSet* AllocSocketSet(int max)
{
	struct SocketSet* set;
	int               i;

	set = (struct SocketSet*) malloc(sizeof(*set));
	if (set != NULL) {
		set->numsockets = 0;
		set->maxsockets = max;
		set->sockets    = (struct Socket**) malloc(max * sizeof(*set->sockets));
		if (set->sockets != NULL) {
			for (i = 0; i < max; ++i) { set->sockets[i] = NULL; }
		} else {
			free(set);
			set = NULL;
		}
	}
	return (set);
}

//
void FreeSocketSet(SocketSet* set)
{
	if (set) {
		free(set->sockets);
		free(set);
	}
}

//
int AddSocket(SocketSet* set, Socket* sock)
{
	if (sock != NULL) {
		if (set->numsockets == set->maxsockets) {
			TraceLog(LOG_DEBUG, "Socket Error: %s", "SocketSet is full");
			SocketSetLastError(0);
			return (-1);
		}
		set->sockets[set->numsockets++] = (struct Socket*) sock;
	} else {
		TraceLog(LOG_DEBUG, "Socket Error: %s", "Socket was null");
		SocketSetLastError(0);
		return (-1);
	}
	return (set->numsockets);
}

//
int RemoveSocket(SocketSet* set, Socket* sock)
{
	int i;

	if (sock != NULL) {
		for (i = 0; i < set->numsockets; ++i) {
			if (set->sockets[i] == (struct Socket*) sock) { break; }
		}
		if (i == set->numsockets) {
			TraceLog(LOG_DEBUG, "Socket Error: %s", "Socket not found");
			SocketSetLastError(0);
			return (-1);
		}
		--set->numsockets;
		for (; i < set->numsockets; ++i) {
			set->sockets[i] = set->sockets[i + 1];
		}
	}
	return (set->numsockets);
}

//
int CheckSockets(SocketSet* set, unsigned int timeout)
{
	int            i;
	SOCKET         maxfd;
	int            retval;
	struct timeval tv;
	fd_set         mask;

	/* Find the largest file descriptor */
	maxfd = 0;
	for (i = set->numsockets - 1; i >= 0; --i) {
		if (set->sockets[i]->channel > maxfd) {
			maxfd = set->sockets[i]->channel;
		}
	}

	/* Check the file descriptors for available data */
	do {
		SocketSetLastError(0);

		/* Set up the mask of file descriptors */
		FD_ZERO(&mask);
		for (i = set->numsockets - 1; i >= 0; --i) {
			FD_SET(set->sockets[i]->channel, &mask);
		} /* Set up the timeout */
		tv.tv_sec  = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;

		/* Look! */
		retval = select(maxfd + 1, &mask, NULL, NULL, &tv);
	} while (SocketGetLastError() == WSAEINTR);

	/* Mark all file descriptors ready that have data available */
	if (retval > 0) {
		for (i = set->numsockets - 1; i >= 0; --i) {
			if (FD_ISSET(set->sockets[i]->channel, &mask)) {
				set->sockets[i]->ready = 1;
			}
		}
	}
	return (retval);
}

//
Packet* AllocPacket(int size)
{
	struct Packet* packet;
	packet = (struct Packet*) malloc(sizeof(*packet));
	if (packet != NULL) {
		memset(packet, 0, sizeof(*packet));
		packet->size = 0;
		packet->offs = 0;
		packet->maxs = size;
		packet->data = (uint8_t*) malloc(size * sizeof(uint8_t));
		memset(packet->data, '\0', size);
		if (packet->data == NULL) {
			free(packet);
			packet = NULL;
		}
	} else {
		free(packet);
		packet = NULL;
	}
	return packet;
}

//
void FreePacket(Packet* packet)
{
	if (packet != NULL) {
		if (packet->data != NULL) {
			free(packet->data);
			packet->data = NULL;
		}
		free(packet);
		packet = NULL;
	}
}

//
void PacketSend(Packet* packet)
{
	printf("Sending packet (%s) with size %d\n", packet->data, packet->size);
}

//
void PacketReceive(Packet* packet)
{
	printf("Receiving packet (%s) with size %d\n", packet->data, packet->size);
}

//
void PacketWrite16(Packet* packet, uint16_t value)
{
	printf("Original: 0x%04" PRIX16 " - %" PRIu16 "\n", value, value);
	uint8_t* data = packet->data + packet->offs;
	*data++       = (uint8_t)(value >> 8);
	*data++       = (uint8_t)(value);
	packet->size += sizeof(uint16_t);
	packet->offs += sizeof(uint16_t);
	printf("Network: 0x%04" PRIX16 " - %" PRIu16 "\n", (uint16_t) *data, (uint16_t) *data);
}

//
void PacketWrite32(Packet* packet, uint32_t value)
{
	printf("Original: 0x%08" PRIX32 " - %" PRIu32 "\n", value, value);
	uint8_t* data = packet->data + packet->offs;
	*data++       = (uint8_t)(value >> 24);
	*data++       = (uint8_t)(value >> 16);
	*data++       = (uint8_t)(value >> 8);
	*data++       = (uint8_t)(value);
	packet->size += sizeof(uint32_t);
	packet->offs += sizeof(uint32_t);
	printf("Network: 0x%08" PRIX32 " - %" PRIu32 "\n",
		   (uint32_t)(((intptr_t) packet->data) - packet->offs),
		   (uint32_t)(((intptr_t) packet->data) - packet->offs));
}

//
void PacketWrite64(Packet* packet, uint64_t value)
{
	printf("Original: 0x%016" PRIX64 " - %" PRIu64 "\n", value, value);
	uint8_t* data = packet->data + packet->offs;
	*data++       = (uint8_t)(value >> 56);
	*data++       = (uint8_t)(value >> 48);
	*data++       = (uint8_t)(value >> 40);
	*data++       = (uint8_t)(value >> 32);
	*data++       = (uint8_t)(value >> 24);
	*data++       = (uint8_t)(value >> 16);
	*data++       = (uint8_t)(value >> 8);
	*data++       = (uint8_t)(value);
	packet->size += sizeof(uint64_t);
	packet->offs += sizeof(uint64_t);
	printf("Network: 0x%016" PRIX64 " - %" PRIu64 "\n",
		   (uint64_t)(packet->data - packet->offs),
		   (uint64_t)(packet->data - packet->offs));
}

//
uint16_t PacketRead16(Packet* packet)
{
	uint8_t* data = packet->data + packet->offs;
	packet->size += sizeof(uint16_t);
	packet->offs += sizeof(uint16_t);
	uint16_t value = ((uint16_t) data[0] << 8) | data[1];
	printf("Original: 0x%04" PRIX16 " - %" PRIu16 "\n", value, value);
	return value;
}

//
uint32_t PacketRead32(Packet* packet)
{
	uint8_t* data = packet->data + packet->offs;
	packet->size += sizeof(uint32_t);
	packet->offs += sizeof(uint32_t);
	uint32_t value = ((uint32_t) data[0] << 24) | ((uint32_t) data[1] << 16) | ((uint32_t) data[2] << 8) | data[3];
	printf("Original: 0x%08" PRIX32 " - %" PRIu32 "\n", value, value);
	return value;
}

//
uint64_t PacketRead64(Packet* packet)
{
	uint8_t* data = packet->data + packet->offs;
	packet->size += sizeof(uint64_t);
	packet->offs += sizeof(uint64_t);
	uint64_t value = ((uint64_t) data[0] << 56) | ((uint64_t) data[1] << 48) | ((uint64_t) data[2] << 40) | ((uint64_t) data[3] << 32) | ((uint64_t) data[4] << 24) | ((uint64_t) data[5] << 16) | ((uint64_t) data[6] << 8) | data[7];
	printf("Original: 0x%016" PRIX64 " - %" PRIu64 "\n", value, value);
	return value;
}
