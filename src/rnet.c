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
// Module dependencies
//----------------------------------------------------------------------------------

#include "raylib.h"
#include "rpack.h"
#include "sysnet.h"

//----------------------------------------------------------------------------------
// Module defines
//----------------------------------------------------------------------------------

#if PLATFORM_WINDOWS
#	define errno WSAGetLastError() // Support UNIX socket error codes
#endif

#define DEF_BACKLOG_SIZE SOMAXCONN
#define PORT_STR_BUFSZ 6

static bool SocketSetDefaults(SocketConfig* cfg);
static bool CreateSocket(SocketConfig* cfg, SocketResult* out);
static bool SocketSetNonBlocking(SocketResult* out);
static bool SocketSetOptions(SocketConfig* cfg, SocketResult* out, int fd);
static const char* SocketStatusToString(enum SocketStatus s);

/* Static network API methods */

// Sets the error code that can be retrieved through the WSAGetLastError function.
static void SocketSetError(int err)
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
	return gai_strerror(SocketGetLastError());
}

static bool SocketSetDefaults(SocketConfig* cfg)
{
	if (cfg->backlog_size == 0)
	{
		cfg->backlog_size = DEF_BACKLOG_SIZE;
	}

	/* Screen out contradictory settings */
	if (cfg->IPv6 && cfg->IPv4)
	{
		return false;
	}
	return true;
}

static bool SocketSaveError(SocketResult* out, enum SocketStatus status)
{
	out->status      = status;
	out->saved_errno = SocketGetLastError();
	SocketSetError(0);
	return false;
}

static bool CreateSocket(SocketConfig* cfg, SocketResult* out)
{
	struct addrinfo  hints;
	struct addrinfo* res = NULL;

	int  fd = -1;
	char port_str[PORT_STR_BUFSZ];
	memset(port_str, 0, PORT_STR_BUFSZ);

	SocketSetHints(cfg, &hints);

	if (PORT_STR_BUFSZ < snprintf(port_str, PORT_STR_BUFSZ, "%u", cfg->port))
	{
		return SocketSaveError(out, SOCKET_ERROR_SNPRINTF);
	}

	struct addrinfo* ai       = NULL;
	int              addr_res = getaddrinfo(cfg->host, port_str, &hints, &res);
	if (addr_res != 0)
	{
		out->getaddrinfo_error = addr_res;
		freeaddrinfo(res);
		return SocketSaveError(out, SOCKET_ERROR_GETADDRINFO);
	}
	memcpy(&out->addrinfo, res, sizeof(struct addrinfo));

	for (ai = res; ai != NULL; ai = ai->ai_next)
	{
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd == -1)
		{
			/* Save errno, but will be clobbered if others succeed. */
			out->status      = SOCKET_ERROR_SOCKET;
			out->saved_errno = SocketGetLastError();
			SocketSetError(0);
			continue;
		}

		if (!SocketSetOptions(cfg, out, fd))
		{
			freeaddrinfo(res);
			return false;
		}

		if (cfg->server)
		{
			int bind_res = bind(fd, res->ai_addr, res->ai_addrlen);
			if (bind_res == -1)
			{
				freeaddrinfo(res);
				return SocketSaveError(out, SOCKET_ERROR_BIND);
			}

			if (!cfg->datagram)
			{
				int listen_res = listen(fd, cfg->backlog_size);
				if (listen_res == -1)
				{
					freeaddrinfo(res);
					return SocketSaveError(out, SOCKET_ERROR_LISTEN);
				}
			}
			break;
		}
		else /* client */
		{
			if (cfg->datagram)
			{
				break;
			}

			int connect_res = connect(fd, ai->ai_addr, ai->ai_addrlen);
			if (connect_res == 0)
			{
				break;
			}
			else
			{
				close(fd);
				fd          = -1;
				out->status = SOCKET_ERROR_CONNECT;
				continue;
			}
		}
	}

	if (fd == -1)
	{
		if (out->status == SOCKET_OK)
		{
			freeaddrinfo(res);
			return SocketSaveError(out, SOCKET_ERROR_UNKNOWN);
		}
		else
		{
			out->saved_errno = SocketGetLastError();
			SocketSetError(0);
			freeaddrinfo(res);
			return false;
		}
	}

	out->status = SOCKET_OK;
	freeaddrinfo(res);
	out->saved_errno      = 0;
	out->socket.handle    = fd;
	out->socket.ready     = 0;
	out->socket.host.host = ((struct sockaddr_in*) res->ai_addr)->sin_addr.s_addr;
	out->socket.host.port = ((struct sockaddr_in*) res->ai_addr)->sin_port;
	out->socket.sflag     = cfg->server;
	return true;
}

static bool SocketSetNonBlocking(SocketResult* out)
{
#if PLATFORM == PLATFORM_WINDOWS
	unsigned long mode = 1;
	if (ioctlsocket(out->socket.handle, FIONBIO, &mode) != 0)
	{
		return SocketSaveError(out, SOCKET_ERROR_FCNTL);
	}
#else
	int flags = fcntl(out->socket.handle, F_GETFL, 0);
	if (flags == -1)
	{
		return SocketSaveError(out, SOCKET_ERROR_FCNTL);
	}
	if (fcntl(out->socket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		return SocketSaveError(out, SOCKET_ERROR_FCNTL);
	}
#endif
	return true;
}

static bool SocketSetOptions(SocketConfig* cfg, SocketResult* out, int fd)
{
	for (int i = 0; i < MAX_SOCK_OPTS; i++)
	{
		SocketOpt* opt = &cfg->sockopts[i];
		if (opt->id == 0)
		{
			break;
		}

		if (setsockopt(fd, SOL_SOCKET, opt->id, opt->value, opt->valueLen) < 0)
		{
			return SocketSaveError(out, SOCKET_ERROR_SETSOCKOPT);
		}
	}

	return true;
}

static const char* SocketStatusToString(enum SocketStatus s)
{
	switch (s)
	{
		case SOCKET_OK:
			return "ok";
		case SOCKET_ERROR_GETADDRINFO:
			return "getaddrinfo";
		case SOCKET_ERROR_SOCKET:
			return "socket";
		case SOCKET_ERROR_BIND:
			return "bind";
		case SOCKET_ERROR_LISTEN:
			return "listen";
		case SOCKET_ERROR_CONNECT:
			return "connect";
		case SOCKET_ERROR_FCNTL:
			return "fcntl";
		case SOCKET_ERROR_SNPRINTF:
			return "snprintf";
		case SOCKET_ERROR_CONFIGURATION:
			return "configuration";
		case SOCKET_ERROR_SETSOCKOPT:
			return "setsockopt";
		case SOCKET_ERROR_UNKNOWN:
		default:
			return "unknown";
	}
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
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %ls", ip, port, gai_strerror(errno));
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
		TraceLog(LOG_WARNING, "Failed to resolve ip %s: %ls", ip, SocketGetLastErrorString());
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
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %ls", address, port, gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully resolved host %s:%s", address, port);
	}

	struct addrinfo* iterator;
	for (iterator = outaddr; iterator != NULL; iterator = iterator->ai_next)
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
	return SocketAddressToString(outaddr->ai_addr);
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
	if (config == NULL || result == NULL)
	{
		return false;
	}
	memset(result, 0, sizeof(*result));

	if (!SocketSetDefaults(config))
	{
		result->status = SOCKET_ERROR_CONFIGURATION;
		return false;
	}

	if (!CreateSocket(config, result))
	{
		return false;
	}

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

// The accept function permits an incoming connection attempt on a socket.
//
bool SocketAccept(SocketHandle listener, SocketResult* out)
{
	struct sockaddr_in sock_addr;
	socklen_t          sock_alen;
	sock_alen          = sizeof(sock_addr);
	out->socket.handle = accept(listener, (struct sockaddr*) &sock_addr, &sock_alen);
	if (out->socket.handle == INVALID_SOCKET)
	{
		/* Save errno, but will be clobbered if others succeed. */
		out->status      = SOCKET_ERROR_ACCEPT;
		out->saved_errno = SocketGetLastError();
		SocketSetError(0);
		return false;
	}
	memcpy(&out->addrinfo, &sock_addr, sizeof(struct sockaddr));
	out->socket.host.host = sock_addr.sin_addr.s_addr;
	out->socket.host.port = sock_addr.sin_port;

	return true;
}

/* Send 'len' bytes of 'data' over the non-server socket 'sock'
   This function returns the actual amount of data sent.  If the return value
   is less than the amount of data sent, then either the remote connection was
   closed, or an unknown socket error occurred.
*/
int SocketSend(Socket* socket, const void* datap, int len)
{
	const unsigned char* data = (const unsigned char*) datap; /* For pointer arithmetic */
	int                  sent, left;

	// /* Server sockets are for accepting connections only */
	if (socket->sflag)
	{
		// out->status = SOCKET_ERROR_SEND;
		// out->saved_errno = SocketGetLastError();
		// SocketSetError(0);
		return (-1);
	}

	/* Keep sending data until it's sent or an error occurs */
	left = len;
	sent = 0;
	SocketSetError(0);
	do
	{
		len = send(socket->handle, (const char*) data, left, 0);
		if (len > 0)
		{
			sent += len;
			left -= len;
			data += len;
		}
	} while ((left > 0) && ((len > 0) || (SocketGetLastError() == EINTR)));

	return (sent);
}

/* Receive up to 'maxlen' bytes of data over the non-server socket 'sock',
   and store them in the buffer pointed to by 'data'.
   This function returns the actual amount of data received.  If the return
   value is less than or equal to zero, then either the remote connection was
   closed, or an unknown socket error occurred.
*/
int SocketReceive(Socket* socket, void* data, int maxlen)
{
	int len;

	/* Server sockets are for accepting connections only */
	if (socket->sflag)
	{
		// out->status = SOCKET_ERROR_RECEIVE;
		// out->saved_errno = SocketGetLastError();
		// SocketSetError(0);
		return (-1);
	}

	SocketSetError(0);
	do
	{
		len = recv(socket->handle, (char*) data, maxlen, 0);
	} while (SocketGetLastError() == EINTR);

	// sock->ready = 0;
	return (len);
}

/* Construct an error message in BUF, based on the status codes
 * in *RES. This has the same return value and general behavior
 * as snprintf -- if the return value is >= buf_size, the string
 * has been truncated. Returns -1 if either BUF or RES are NULL. */
int SocketGetError(char* buf, size_t buf_size, SocketResult* res)
{
	if (buf == NULL || res == NULL)
	{
		return 0;
	}
	return snprintf(buf, buf_size, "%s: %ls", SocketStatusToString(res->status), (res->status == SOCKET_ERROR_GETADDRINFO ? gai_strerror(res->getaddrinfo_error) : strerror(res->saved_errno)));
}

/* Print an error message based on the status contained in *RES. */
void SocketPrintError(SocketResult* res)
{
	if (res == NULL)
	{
		return;
	}
	printf("%s: %ls\n", SocketStatusToString(res->status), (res->status == SOCKET_ERROR_GETADDRINFO ? gai_strerror(res->getaddrinfo_error) : strerror(res->saved_errno)));
}

/* Set "hints" in an addrinfo struct, to be passed to getaddrinfo. */
void SocketSetHints(SocketConfig* cfg, struct addrinfo* hints)
{
	if (cfg == NULL || hints == NULL)
	{
		return;
	}
	memset(hints, 0, sizeof(*hints));

	/* if .IPv4 or .IPv6 are used, set and use that instead of *host */
	if (cfg->path)
	{
		hints->ai_family = AF_UNIX;
	}
	else if (cfg->IPv6)
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
			TraceLog(LOG_INFO, "\t- IPv4 address %s", SocketAddressToString(addr, ip));
		}
		break;
		case AF_INET6:
		{
			TraceLog(LOG_DEBUG, "\tFamily: AF_INET6 (IPv6)");
			TraceLog(LOG_INFO, "\t- IPv6 address %s", SocketAddressToString(addr, ip));
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
char* SocketAddressToString(struct SocketAddress* sockaddr, char buffer[])
{
	switch (sockaddr->family)
	{
		case AF_INET:
		{
			return inet_ntop(AF_INET, &((struct sockaddr_in*) sockaddr)->sin_addr, buffer, INET_ADDRSTRLEN);
		}
		break;
		case AF_INET6:
		{
			return inet_ntop(AF_INET6, &((struct sockaddr_in6*) sockaddr)->sin6_addr, buffer, INET6_ADDRSTRLEN);
		}
		break;
		default:
		{
			return NULL;
		}
		break;
	}
}

/*
** PackData() -- store data dictated by the format string in the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (16-bit unsigned length is automatically prepended to strings)
*/
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

/*
** UnpackData() -- unpack data dictated by the format string into the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (string is extracted based on its stored length, but 's' can be
**  prepended with a max length)
*/
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