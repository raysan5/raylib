/**********************************************************************************************
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

#define SOCKET_BACKLOG_SIZE 20

//----------------------------------------------------------------------------------
// Module dependencies
//----------------------------------------------------------------------------------

#include "raylib.h"
#include "rpack.h"
#include "sysnet.h"

//----------------------------------------------------------------------------------
// Module variables
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module methods
//----------------------------------------------------------------------------------

static bool  IsSocketValid(SocketHandle handle);
static void  SocketSetLastError(int err);
static int   SocketGetLastError();
static char* SocketGetLastErrorString();
static char* SocketErrorCodeToString(int err);
static bool  SocketSetDefaults(SocketConfig* config);
static bool  InitSocket(Socket* outsock);
static bool  CreateSocket(SocketConfig* config, SocketResult* outresult);
static int   BindSocket(SocketHandle handle, SocketConfig* config, struct addrinfo* iterator);
static int   ConnectSocket(SocketHandle handle, SocketConfig* config, struct addrinfo* iterator);
static bool  SocketSetNonBlocking(SocketResult* out);
static bool  SocketSetOptions(SocketConfig* config, SocketHandle handle);
static void* GetSocketAddressPtr(struct sockaddr* sa);
static void* GetSocketPortPtr(struct sockaddr* sa);

//
void* GetSocketPortPtr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_port);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_port);
}

//
void* GetSocketAddressPtr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

//
static bool IsSocketValid(SocketHandle handle)
{
	return (handle != INVALID_SOCKET);
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
	if (config->backlog_size == 0)
	{
		config->backlog_size = SOCKET_BACKLOG_SIZE;
	}

	if (config->IPv6 && config->IPv4)
	{
		return false;
	}

	return true;
}

// Create the socket handle
static bool InitSocket(Socket* outsock)
{
	switch (outsock->type)
	{
		case SOCKET_TCP:
			outsock->handle = socket(AF_INET, SOCK_STREAM, 0);
			break;
		case SOCKET_UDP:
			outsock->handle = socket(AF_INET, SOCK_DGRAM, 0);
			break;
		default:
			break;
	}
	return IsSocketValid(outsock->handle);
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
	int              status; // Status value to return (0) is success
	struct addrinfo  hints;  // Address flags (IPV4, IPV6, UDP?)
	struct addrinfo* res;    // A pointer to the resulting address list
	Socket*          outsocket;
	outsocket         = &outresult->socket;
	outsocket->handle = INVALID_SOCKET;
	outresult->status = RESULT_FAILURE;

	// Set the socket type
	outresult->socket.type = (config->datagram) ? SOCKET_UDP : SOCKET_TCP;

	//	Set the hints based on information in the config
	//
	//	AI_CANONNAME	Causes the ai_canonname of the result to the filled  out with the host's canonical (real) name.
	//	AI_PASSIVE:		Causes the result's IP address to be filled out with INADDR_ANY (IPv4)or in6addr_any (IPv6);
	//	Note:	This causes a subsequent call to bind() to auto-fill the IP address
	//			of the struct sockaddr with the address of the current host.
	//
	SocketSetHints(config, &hints);

	// Populate address information
	status = getaddrinfo(config->host, // e.g. "www.example.com" or IP (Can be null if AI_PASSIVE flag is set
						 config->port, // e.g. "http" or port number
						 &hints,       // e.g. SOCK_STREAM/SOCK_DGRAM
						 &res          // The struct to populate
	);

	// Did we succeed?
	if (status != 0)
	{
		outsocket->error = SocketGetLastError();
		TraceLog(
			LOG_DEBUG, "Socket Error: %s", SocketErrorCodeToString(outsocket->error));
		SocketSetLastError(0);
		TraceLog(LOG_WARNING,
				 "Failed to get resolve host %s:%s: %s",
				 config->host,
				 config->port,
				 SocketGetLastErrorString());
		freeaddrinfo(res);
		return false;
	}
	else
	{
		TraceLog(
			LOG_INFO, "Successfully resolved host %s:%s", config->host, config->port);
	}
	memcpy(&outresult->addrinfo, res, sizeof(struct addrinfo));

	// Walk the addrinfo struct
	struct addrinfo* it;
	for (it = res; it != NULL; it = it->ai_next)
	{
		// Initialise the socket
		if (!InitSocket(outsocket))
		{
			outsocket->error = SocketGetLastError();
			TraceLog(
				LOG_DEBUG, "Socket Error: %s", SocketErrorCodeToString(outsocket->error));
			SocketSetLastError(0);
			continue;
		}

		// Set socket options
		if (!SocketSetOptions(config, outsocket->handle))
		{
			outsocket->error = SocketGetLastError();
			TraceLog(
				LOG_DEBUG, "Socket Error: %s", SocketErrorCodeToString(outsocket->error));
			SocketSetLastError(0);
			freeaddrinfo(res);
			return false;
		}

		// Only bind to sockets marked as server
		if (config->server)
		{
			if (BindSocket(outsocket->handle, config, it))
			{
				TraceLog(LOG_INFO, "Successfully bound socket.");
			}
			else
			{
				outsocket->error = SocketGetLastError();
				TraceLog(LOG_DEBUG,
						 "Socket Error: %s",
						 SocketErrorCodeToString(outsocket->error));
				SocketSetLastError(0);
				freeaddrinfo(res);
				return false;
			}
		}
		else
		{
			if (ConnectSocket(outsocket->handle, config, it))
			{
				TraceLog(LOG_INFO, "Successfully connected to socket.");
				break;
			}
			else
			{
				outsocket->error = SocketGetLastError();
				TraceLog(LOG_DEBUG,
						 "Socket Error: %s",
						 SocketErrorCodeToString(outsocket->error));
				SocketSetLastError(0);
				SocketClose(outsocket->handle);
				continue;
			}
		}
	}

	if (!IsSocketValid(outsocket->handle))
	{
		outsocket->error = SocketGetLastError();
		TraceLog(
			LOG_DEBUG, "Socket Error: %s", SocketErrorCodeToString(outresult->status));
		SocketSetLastError(0);
		freeaddrinfo(res);
		return false;
	}

	outresult->status          = RESULT_SUCCESS;
	outresult->socket.ready    = 0;
	outresult->socket.error    = 0;
	outresult->socket.isServer = config->server;
	outresult->socket.address.host = (char*) malloc(INET6_ADDRSTRLEN);
	SocketAddressToString(res->ai_addr, outresult->socket.address.host, &outresult->socket.address.port);
	freeaddrinfo(res);
	return true;
}

//
static bool BindSocket(SocketHandle handle, SocketConfig* config, struct addrinfo* iterator)
{
	// Attempt to bind the socket
	if (bind(handle, iterator->ai_addr, iterator->ai_addrlen) == SOCKET_ERROR)
	{
		SocketClose(handle);
		return false;
	}

	// Don't listen on UDP sockets
	if (!config->datagram)
	{
		if (listen(handle, config->backlog_size) != 0)
		{
			return false;
		}
		else
		{
			TraceLog(LOG_INFO, "Started listening on socket...");
		}
	}

	return true;
}

//
static bool ConnectSocket(SocketHandle handle, SocketConfig* config, struct addrinfo* iterator)
{
	// Don't connect datagram sockets
	if (config->datagram)
	{
		return true;
	}

	// Did we connect successfully?
	if (connect(handle, iterator->ai_addr, iterator->ai_addrlen) != SOCKET_ERROR)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
static bool SocketSetBlocking(SocketResult* out)
{
#if PLATFORM == PLATFORM_WINDOWS
	unsigned long mode = 0;
	ioctlsocket(out->socket.handle, FIONBIO, &mode);
#else
	int flags = fcntl(out->socket.handle, F_GETFL, 0);
	fcntl(out->socket.handle, F_SETFL, flags & ~O_NONBLOCK);
#endif
}

//
static bool SocketSetNonBlocking(SocketResult* out)
{
#if PLATFORM == PLATFORM_WINDOWS
	unsigned long mode = 1;
	ioctlsocket(out->socket.handle, FIONBIO, &mode);
#else
	fcntl(out->socket.handle, F_SETFL, O_NONBLOCK);
#endif
	return true;
}

//
static bool SocketSetOptions(SocketConfig* config, SocketHandle handle)
{
	for (int i = 0; i < MAX_SOCK_OPTS; i++)
	{
		SocketOpt* opt = &config->sockopts[i];
		if (opt->id == 0)
		{
			break;
		}

		if (setsockopt(handle, SOL_SOCKET, opt->id, opt->value, opt->valueLen) < 0)
		{
			return false;
		}
	}

	return true;
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
	if (err != 0)
	{
		TraceLog(LOG_WARNING, "WinSock failed to initialise.");
		return false;
	}
	else
	{
		TraceLog(LOG_INFO, "WinSock initialised.");
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
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
	if (WSACleanup() == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEINPROGRESS)
		{
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
//	NAME_INFO_DEFAULT		0x00	/* No flags set */
//	NAME_INFO_NOFQDN			0x01	/* Only return nodename portion for local hosts */
//	NAME_INFO_NUMERICHOST	0x02	/* Return numeric form of the host's address */
//	NAME_INFO_NAMEREQD		0x04	/* Error if the host's name not in DNS */
//	NAME_INFO_NUMERICSERV	0x08	/* Return numeric form of the service (port #) */
//	NAME_INFO_DGRAM			0x10	/* Service is a datagram service */
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
	if (status != 0)
	{
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %s", ip, port, gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_DEBUG, "Resolving... %s::%s", ip, port);
	}

	// Attempt to resolve network byte order ip to hostname
	switch (results->ai_family)
	{
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
		default:
			break;
	}

	// Did we succeed?
	if (status != 0)
	{
		TraceLog(LOG_WARNING, "Failed to resolve ip %s: %s", ip, SocketGetLastErrorString());
	}
	else
	{
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
//	AddressInformation*	outaddr - Results of host resolution (getaddrinfo)
char* ResolveHost(const char* address, const char* port, AddressInformation* outaddr)
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
	if (address == NULL)
	{
		hints.ai_flags = AI_PASSIVE;
	}

	// Populate address information
	status = getaddrinfo(address, // e.g. "www.example.com" or IP
						 port,    // e.g. "http" or port number
						 &hints,  // e.g. SOCK_STREAM/SOCK_DGRAM
						 &results // The struct to populate
	);

	memcpy(outaddr, results, sizeof(struct addrinfo));

	// Did we succeed?
	if (status != 0)
	{
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %s", address, port, gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully resolved host %s:%s", address, port);
	}

	struct addrinfo* iterator;
	for (iterator = results; iterator != NULL; iterator = iterator->ai_next)
	{
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
	return SocketAddressToString(outaddr->sockaddr, ip, &portptr);
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
bool SocketOpen(SocketConfig* config, SocketResult* result)
{
	// Make sure we've not received a null config or result pointer
	if (config == NULL || result == NULL)
	{
		return false;
	}
	memset(result, 0, sizeof(*result));

	// Set the defaults based on the config
	if (!SocketSetDefaults(config))
	{
		TraceLog(LOG_DEBUG, "Configuration Error.");
		return false;
	}

	// Create the socket
	if (!CreateSocket(config, result))
	{
		return false;
	}

	// If the config states non-blocking, set the socket to non-blocking
	if (config->nonblocking)
	{
		if (!SocketSetNonBlocking(result))
		{
			return false;
		}
	}

	return true;
}

//	Closes an existing socket
//
//	SocketHandle socket - The id of the socket to close
void SocketClose(SocketHandle socket)
{
	if (socket)
	{
		closesocket(socket);
	}
}

//	The accept function permits an incoming connection attempt on a socket.
//
//	SocketHandle	listener	- The socket to listen for incoming connections on (i.e. server)
//	SocketResult*	out			- The result of this function (if any, including errors)
//
//	e.g.
//
//	SocketResult connection;
//	bool         connected = false;
//	if (!connected)
//	{
//		if (SocketAccept(server_res.socket.handle, &connection))
//		{
//			connected = true;
//		}
//	}
bool SocketAccept(SocketHandle listener, SocketResult* out)
{
	char                    ip[INET6_ADDRSTRLEN];
	struct sockaddr_storage sockAddr;
	socklen_t               sockAddrLen;
	sockAddrLen        = sizeof(sockAddr);
	out->socket.handle = accept(listener, (struct sockaddr*) &sockAddr, &sockAddrLen);
	if (out->socket.handle == INVALID_SOCKET)
	{
		out->socket.error = SocketGetLastError();
		TraceLog(
			LOG_DEBUG, "Socket Error: %s", SocketErrorCodeToString(out->socket.error));
		SocketSetLastError(0);
		return false;
	}
	memcpy(&out->addrinfo, &sockAddr, sizeof(struct sockaddr));
	out->socket.address.host = inet_ntop(sockAddr.ss_family, GetSocketAddressPtr((struct sockaddr*) &sockAddr), ip, sizeof ip);
	out->socket.address.port = ntohs(GetSocketPortPtr((struct sockaddr*) &sockAddr));
	TraceLog(LOG_DEBUG, "Server: Got connection from %s\n", out->socket.address.host);
	return true;
}

//	Send 'len' bytes of 'data' over the non-server socket 'sock'
//
//	Example
int SocketSend(Socket* socket, const char* buffer, int length)
{
	int                sentTotal = 0;      // How many bytes we've sent
	int                bytesleft = length; // How many do we have left to send
	int                actuallySent; // How many bytes did we send this tick?
	struct sockaddr_in dest;         // The datagram (UDP) destination
	dest.sin_addr.s_addr = socket->address.host;
	dest.sin_port        = socket->address.port;

	// Which socket are we trying to send data on
	switch (socket->type)
	{
		case SOCKET_TCP:
			if (IsSocketValid(socket->handle))
			{
				if ((length > 0) && (buffer != NULL))
				{
					SocketSetLastError(0);
					do
					{
						actuallySent = send(socket->handle, buffer + sentTotal, bytesleft, 0);
						if (actuallySent > 0)
						{
							sentTotal += actuallySent;
							bytesleft -= actuallySent;
						}
					} while ((bytesleft > 0) && // While we still have bytes left to send
							 ((actuallySent > 0) || // The amount of bytes we actually sent is > 0
							  (SocketGetLastError() == EINTR)) // The socket was interupted
					);
				}
			}
			break;
		case SOCKET_UDP:
			if (IsSocketValid(socket->handle))
			{
				if ((length > 0) && (buffer != NULL))
				{
					SocketSetLastError(0);
					do
					{
						actuallySent = sendto(socket->handle, buffer + sentTotal, bytesleft, 0, (struct sockaddr*) &dest, sizeof dest);
						if (actuallySent > 0)
						{
							sentTotal += actuallySent;
							bytesleft -= actuallySent;
						}
					} while ((bytesleft > 0) && // While we still have bytes left to send
							 ((actuallySent > 0) || // The amount of bytes we actually sent is > 0
							  (SocketGetLastError() == EINTR)) // The socket was interupted
					);
				}
			}
			break;
		default:
			break;
	}

	// Server sockets are for accepting connections only
	if (socket->isServer)
	{
		TraceLog(LOG_WARNING, "Cannot send information on a server socket");
		return -1;
	}

	return sentTotal;
}

//	Receive up to 'maxlen' bytes of data over the non-server socket 'sock',
//	and store them in the buffer pointed to by 'data'.
//	This function returns the actual amount of data received.  If the return
//	value is less than or equal to zero, then either the remote connection was
//	closed, or an unknown socket error occurred.
int SocketReceive(Socket* socket, void* data, int maxlen)
{
	int len;

	/* Server sockets are for accepting connections only */
	if (socket->isServer)
	{
		// out->status = SOCKET_ERROR_RECEIVE;
		// out->saved_errno = SocketGetLastError();
		// SocketSetLastError(0);
		return (-1);
	}

	SocketSetLastError(0);
	do
	{
		len = recv(socket->handle, (char*) data, maxlen, 0);
	} while (SocketGetLastError() == EINTR);

	// sock->ready = 0;
	return (len);
}

// Set "hints" in an addrinfo struct, to be passed to getaddrinfo.
void SocketSetHints(SocketConfig* cfg, struct addrinfo* hints)
{
	if (cfg == NULL || hints == NULL)
	{
		return;
	}
	memset(hints, 0, sizeof(*hints));

	/* if .IPv4 or .IPv6 are used, set and use that instead of *host */
	if (cfg->IPv6)
	{
		hints->ai_family = AF_INET6;
	}
	else if (cfg->IPv4)
	{
		hints->ai_family = AF_INET;
	}
	else
	{
		hints->ai_family = AF_UNSPEC;
	}

	if (cfg->datagram)
	{
		hints->ai_socktype = SOCK_DGRAM;
	}
	else
	{
		hints->ai_socktype = SOCK_STREAM;
	}

	/* Set passive unless UDP client */
	if (!cfg->datagram || cfg->server)
	{
		hints->ai_flags = AI_PASSIVE;
	}

	if (cfg->IPv6 || cfg->IPv4)
	{
		hints->ai_flags |= AI_NUMERICHOST;
	}
}

// Print socket information
void PrintSocket(struct SocketAddress* addr, const int family, const int socktype, const int protocol)
{
	struct sockaddr* sockaddr_ip;
	char             ip[INET6_ADDRSTRLEN]; // Enough pace to hold a IPv6 string
	int              port;
	switch (family)
	{
		case AF_UNSPEC:
		{
			TraceLog(LOG_DEBUG, "\tFamily: Unspecified");
		}
		break;
		case AF_INET:
		{
			TraceLog(LOG_DEBUG, "\tFamily: AF_INET (IPv4)");
			TraceLog(LOG_INFO, "\t- IPv4 address %s", SocketAddressToString(addr, ip, &port));
		}
		break;
		case AF_INET6:
		{
			TraceLog(LOG_DEBUG, "\tFamily: AF_INET6 (IPv6)");
			TraceLog(LOG_INFO, "\t- IPv6 address %s", SocketAddressToString(addr, ip, &port));
		}
		break;
		case AF_NETBIOS:
		{
			TraceLog(LOG_DEBUG, "\tFamily: AF_NETBIOS (NetBIOS)");
		}
		break;
		default:
		{
			TraceLog(LOG_DEBUG, "\tFamily: Other %ld", family);
		}
		break;
	}
	TraceLog(LOG_DEBUG, "\tSocket type:");
	switch (socktype)
	{
		case 0:
			TraceLog(LOG_DEBUG, "\t- Unspecified");
			break;
		case SOCK_STREAM:
			TraceLog(LOG_DEBUG, "\t- SOCK_STREAM (stream)");
			break;
		case SOCK_DGRAM:
			TraceLog(LOG_DEBUG, "\t- SOCK_DGRAM (datagram)");
			break;
		case SOCK_RAW:
			TraceLog(LOG_DEBUG, "\t- SOCK_RAW (raw)");
			break;
		case SOCK_RDM:
			TraceLog(LOG_DEBUG, "\t- SOCK_RDM (reliable message datagram)");
			break;
		case SOCK_SEQPACKET:
			TraceLog(LOG_DEBUG, "\t- SOCK_SEQPACKET (pseudo-stream packet)");
			break;
		default:
			TraceLog(LOG_DEBUG, "\t- Other %ld", socktype);
			break;
	}
	TraceLog(LOG_DEBUG, "\tProtocol:");
	switch (protocol)
	{
		case 0:
			TraceLog(LOG_DEBUG, "\t- Unspecified");
			break;
		case IPPROTO_TCP:
			TraceLog(LOG_DEBUG, "\t- IPPROTO_TCP (TCP)");
			break;
		case IPPROTO_UDP:
			TraceLog(LOG_DEBUG, "\t- IPPROTO_UDP (UDP)");
			break;
		default:
			TraceLog(LOG_DEBUG, "\t- Other %ld", protocol);
			break;
	}
}

// Convert network ordered socket address to human readable string (127.0.0.1)
char* SocketAddressToString(struct SocketAddress* sockaddr, char buffer[], int* port)
{
	switch (sockaddr->family)
	{
		case AF_INET:
		{
			struct sockaddr_in* s = ((struct sockaddr_in*) sockaddr);
			*port                 = ntohs(s->sin_port);
			return inet_ntop(AF_INET, &s->sin_addr, buffer, INET_ADDRSTRLEN);
		}
		break;
		case AF_INET6:
		{
			struct sockaddr_in6* s = ((struct sockaddr_in6*) sockaddr);
			*port                  = ntohs(s->sin6_port);
			return inet_ntop(AF_INET6, &s->sin6_addr, buffer, INET6_ADDRSTRLEN);
		}
		break;
		default:
		{
			return NULL;
		}
		break;
	}
}

//	PackData() -- store data dictated by the format string in the buffer
//
//	  bits |signed   unsigned   float   string
//	  -----+----------------------------------
//	     8 |   c        C
//	    16 |   h        H         f
//	    32 |   l        L         d
//	    64 |   q        Q         g
//	     - |                               s
//
//	(16-bit unsigned length is automatically prepended to strings)
unsigned int PackData(unsigned char* buf, char* format, ...)
{
	va_list ap;

	signed char   c; // 8-bit
	unsigned char C;

	int          h; // 16-bit
	unsigned int H;

	long int          l; // 32-bit
	unsigned long int L;

	long long int          q; // 64-bit
	unsigned long long int Q;

	float                  f; // floats
	double                 d;
	long double            g;
	unsigned long long int fhold;

	char*        s; // strings
	unsigned int len;

	unsigned int size = 0;

	va_start(ap, format);

	for (; *format != '\0'; format++)
	{
		switch (*format)
		{
			case 'c': // 8-bit
				size += 1;
				c      = (signed char) va_arg(ap, int); // promoted
				*buf++ = c;
				break;

			case 'C': // 8-bit unsigned
				size += 1;
				C      = (unsigned char) va_arg(ap, unsigned int); // promoted
				*buf++ = C;
				break;

			case 'h': // 16-bit
				size += 2;
				h = va_arg(ap, int);
				packi16(buf, h);
				buf += 2;
				break;

			case 'H': // 16-bit unsigned
				size += 2;
				H = va_arg(ap, unsigned int);
				packi16(buf, H);
				buf += 2;
				break;

			case 'l': // 32-bit
				size += 4;
				l = va_arg(ap, long int);
				packi32(buf, l);
				buf += 4;
				break;

			case 'L': // 32-bit unsigned
				size += 4;
				L = va_arg(ap, unsigned long int);
				packi32(buf, L);
				buf += 4;
				break;

			case 'q': // 64-bit
				size += 8;
				q = va_arg(ap, long long int);
				packi64(buf, q);
				buf += 8;
				break;

			case 'Q': // 64-bit unsigned
				size += 8;
				Q = va_arg(ap, unsigned long long int);
				packi64(buf, Q);
				buf += 8;
				break;

			case 'f': // float-16
				size += 2;
				f     = (float) va_arg(ap, double); // promoted
				fhold = pack754_16(f);              // convert to IEEE 754
				packi16(buf, fhold);
				buf += 2;
				break;

			case 'd': // float-32
				size += 4;
				d     = va_arg(ap, double);
				fhold = pack754_32(d); // convert to IEEE 754
				packi32(buf, fhold);
				buf += 4;
				break;

			case 'g': // float-64
				size += 8;
				g     = va_arg(ap, long double);
				fhold = pack754_64(g); // convert to IEEE 754
				packi64(buf, fhold);
				buf += 8;
				break;

			case 's': // string
				s   = va_arg(ap, char*);
				len = strlen(s);
				size += len + 2;
				packi16(buf, len);
				buf += 2;
				memcpy(buf, s, len);
				buf += len;
				break;
		}
	}

	va_end(ap);

	return size;
}

//	UnpackData() -- unpack data dictated by the format string into the buffer
//
//	  bits |signed   unsigned   float   string
//	  -----+----------------------------------
//	     8 |   c        C
//	    16 |   h        H         f
//	    32 |   l        L         d
//	    64 |   q        Q         g
//	     - |                               s
//
//	(string is extracted based on its stored length, but 's' can be
//	prepended with a max length)
void UnpackData(unsigned char* buf, char* format, ...)
{
	va_list ap;

	signed char*   c; // 8-bit
	unsigned char* C;

	int*          h; // 16-bit
	unsigned int* H;

	long int*          l; // 32-bit
	unsigned long int* L;

	long long int*          q; // 64-bit
	unsigned long long int* Q;

	float*                 f; // floats
	double*                d;
	long double*           g;
	unsigned long long int fhold;

	char*        s;
	unsigned int len, maxstrlen = 0, count;

	va_start(ap, format);

	for (; *format != '\0'; format++)
	{
		switch (*format)
		{
			case 'c': // 8-bit
				c = va_arg(ap, signed char*);
				if (*buf <= 0x7f)
				{
					*c = *buf;
				} // re-sign
				else
				{
					*c = -1 - (unsigned char) (0xffu - *buf);
				}
				buf++;
				break;

			case 'C': // 8-bit unsigned
				C  = va_arg(ap, unsigned char*);
				*C = *buf++;
				break;

			case 'h': // 16-bit
				h  = va_arg(ap, int*);
				*h = unpacki16(buf);
				buf += 2;
				break;

			case 'H': // 16-bit unsigned
				H  = va_arg(ap, unsigned int*);
				*H = unpacku16(buf);
				buf += 2;
				break;

			case 'l': // 32-bit
				l  = va_arg(ap, long int*);
				*l = unpacki32(buf);
				buf += 4;
				break;

			case 'L': // 32-bit unsigned
				L  = va_arg(ap, unsigned long int*);
				*L = unpacku32(buf);
				buf += 4;
				break;

			case 'q': // 64-bit
				q  = va_arg(ap, long long int*);
				*q = unpacki64(buf);
				buf += 8;
				break;

			case 'Q': // 64-bit unsigned
				Q  = va_arg(ap, unsigned long long int*);
				*Q = unpacku64(buf);
				buf += 8;
				break;

			case 'f': // float
				f     = va_arg(ap, float*);
				fhold = unpacku16(buf);
				*f    = unpack754_16(fhold);
				buf += 2;
				break;

			case 'd': // float-32
				d     = va_arg(ap, double*);
				fhold = unpacku32(buf);
				*d    = unpack754_32(fhold);
				buf += 4;
				break;

			case 'g': // float-64
				g     = va_arg(ap, long double*);
				fhold = unpacku64(buf);
				*g    = unpack754_64(fhold);
				buf += 8;
				break;

			case 's': // string
				s   = va_arg(ap, char*);
				len = unpacku16(buf);
				buf += 2;
				if (maxstrlen > 0 && len > maxstrlen)
					count = maxstrlen - 1;
				else
					count = len;
				memcpy(s, buf, count);
				s[count] = '\0';
				buf += len;
				break;

			default:
				if (isdigit(*format))
				{ // track max str len
					maxstrlen = maxstrlen * 10 + (*format - '0');
				}
		}

		if (!isdigit(*format))
			maxstrlen = 0;
	}

	va_end(ap);
}

//
unsigned short HostToNetworkShort(unsigned short value)
{
	return htons(value);
}

//
unsigned long HostToNetworkLong(unsigned long value)
{
	return htonl(value);
}

//
unsigned int HostToNetworkFloat(float value)
{
	return htonf(value);
}

//
unsigned long long HostToNetworkDouble(double value)
{
	return htond(value);
}

//
unsigned long long HostToNetworkLongLong(unsigned long long value)
{
	return htonll(value);
}

//
unsigned short NetworkToHostShort(unsigned short value)
{
	return ntohs(value);
}

//
unsigned long NetworkToHostLong(unsigned long value)
{
	return ntohl(value);
}

//
float NetworkToHostFloat(unsigned int value)
{
	return ntohf(value);
}

//
double NetworkToHostDouble(unsigned long long value)
{
	return ntohd(value);
}

//
double NetworkToHostLongDouble(unsigned long long value)
{
	return ntohd(value);
}

//
unsigned long long NetworkToHostLongLong(unsigned long long value)
{
	return ntohll(value);
}