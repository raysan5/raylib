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
#include "sysnet.h"

#include <errno.h>
#include <stdlib.h>

//----------------------------------------------------------------------------------
// Module defines
//----------------------------------------------------------------------------------

#if PLATFORM_WINDOWS
#	define errno WSAGetLastError() // Support UNIX socket error codes
#endif

//----------------------------------------------------------------------------------
// Module implementation
//----------------------------------------------------------------------------------

// Initialise the network (requires for windows platforms only)
bool InitNetwork()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == NO_ERROR)
	{
		TraceLog(LOG_INFO, "WinSock initialised.");
		return true;
	}
	else
	{
		TraceLog(LOG_WARNING, "WinSock failed to initialise.");
		return false;
	}
#else
	return true;
#endif
}

// Cleanup, and close the network
void CloseNetwork()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

// Get address information
void GetAddressInformation(AddressInformation* outaddr, const char* address, const char* port, int socketType, int protocolType)
{
	// Variables
	int              status; // Status value to return (0) is success
	struct addrinfo  hints; // Address flags (IPV4, IPV6, UDP?)
	struct addrinfo* results; // A pointer to the resulting address list

	// Set the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC; // Either IPv4 or IPv6 (AF_INET, AF_INET6)
	hints.ai_socktype = socketType; // TCP (SOCK_STREAM), UDP (SOCK_DGRAM)
	hints.ai_protocol = protocolType; // TCP (IPPROTO_TCP), UDP (IPPROTO_UDP)

	// Populate address information
	status = getaddrinfo(address, // e.g. "www.example.com" or IP
						 port, // e.g. "http" or port number
						 &hints, // e.g. SOCK_STREAM/SOCK_DGRAM
						 &results // The struct to populate
	);

	memcpy(outaddr, results, sizeof(struct addrinfo));

	// Did we succeed?
	if (status != 0)
	{
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %ls", address, port, gai_strerror(status));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully resolved host %s:%s", address, port);
	}

	struct addrinfo* iterator;
	for (iterator = outaddr; iterator != NULL; iterator = iterator->ai_next)
	{
		TraceLog(LOG_DEBUG, "GetAddressInformation");
		TraceLog(LOG_DEBUG, "Flags: 0x%x", iterator->ai_flags);
		PrintSocket(iterator->ai_addr,
					iterator->ai_family,
					iterator->ai_socktype,
					iterator->ai_protocol);
		TraceLog(LOG_DEBUG, "Length of this sockaddr: %d", iterator->ai_addrlen);
		TraceLog(LOG_DEBUG, "Canonical name: %s", iterator->ai_canonname);
	}

	freeaddrinfo(results);
}

// Create a socket from information provided by the filled Address information struct
bool CreateSocket(Socket* sock, const AddressInformation addr)
{
	// Create a socket from provided address data
	sock->handle = socket(addr.family, addr.socktype, addr.protocol);

	// Did we succeed?
	if (sock->handle == INVALID_SOCKET)
	{
		TraceLog(LOG_WARNING, "Failed to get create socket: %ls", gai_strerror(sock->handle));
		CloseSocket(sock->handle);
		return false;
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully created socket");
		PrintSocket(addr.sockaddr, addr.family, addr.socktype, addr.protocol);
	}

	// Return true if we've successfully created the socket
	return true;
}

// Bind the socket to a specific port, this is usually used if you're going to listen for incoming connections on a specific port
bool BindSocket(Socket sock, const AddressInformation addr)
{
	// Bind the socket handle to the socket address defined in sockaddr
	int status = bind(sock.handle, addr.sockaddr, addr.addrlen);

	// Did we succeed?
	if (status == SOCKET_ERROR)
	{
		TraceLog(LOG_WARNING, "Failed to get bind socket: %ls", gai_strerror(status));
		return false;
	}
	else
	{
		char buff[INET6_ADDRSTRLEN];
		TraceLog(LOG_INFO, "Successfully bound socket to address: %s", SocketAddressToString(addr.sockaddr, buff));
	}

	// Return true if we've successfully bound the socket
	return true;
}

// Connect the socket to an address
bool ConnectSocket(Socket socket, AddressInformation addr)
{
	int status = connect(socket.handle, addr.sockaddr, addr.addrlen);

	// Did we succeed?
	if (status == SOCKET_ERROR)
	{
		TraceLog(LOG_WARNING, "Failed to connect socket: %ls", gai_strerror(status));
		return false;
	}
	else
	{
		char buff[INET6_ADDRSTRLEN];
		TraceLog(LOG_INFO, "Successfully connected socket to address: %s", SocketAddressToString(addr.sockaddr, buff));
	}

	// Return true if we've successfully connected the socket
	return true;
}

// Listen on a socket
bool ListenSocket(Socket socket)
{
	int status = listen(socket.handle, MAX_SOCKET_QUEUE_SIZE);

	// Did we succeed?
	if (status == SOCKET_ERROR)
	{
		TraceLog(LOG_WARNING, "Failed to listen to socket: %ls", gai_strerror(errno));
		return false;
	}
	else
	{
		TraceLog(LOG_INFO, "Success, socket now listening");
	}

	// Set the socket i/o mode to blocking, or non-blocking
	unsigned long blocking = socket.blocking ? 0 : 1;
	status                 = ioctlsocket(socket.handle, FIONBIO, &blocking);

	if (status != NO_ERROR)
	{
		TraceLog(LOG_WARNING, "Failed to set socket io mode to: %s", (blocking ? "non-blocking" : "blocking"));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully set socket io mode to: %s", (blocking ? "non-blocking" : "blocking"));
	}

	// Return true if we've successfully connected the socket
	return true;
}

// Close a socket
void CloseSocket(Socket* socket)
{
#if PLATFORM_WINDOWS
	closesocket(socket);
#elif PLATFORM == PLATFORM_UNIX
	close(socket);
#endif
}

// Create a listen server, this combines Create/Bind/Listen into 1 function
void CreateListenServer(Socket* tcpsock, const char* address, const char* port, SocketType socketType)
{
	// Variables
	int              status; // Status value to return (0) is success
	struct addrinfo  hints; // Address flags (IPV4, IPV6, UDP?)
	struct addrinfo* results; // A pointer to the resulting address list

	// Set the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC; // Either IPv4 or IPv6 (AF_INET, AF_INET6)
	hints.ai_socktype = socketType; // TCP (SOCK_STREAM), UDP (SOCK_DGRAM)

	// Populate address information
	status = getaddrinfo(address, // e.g. "www.example.com" or IP
						 port, // e.g. "http" or port number
						 &hints, // e.g. SOCK_STREAM/SOCK_DGRAM
						 &results // The struct to populate
	);

	// Did we succeed?
	if (status == -1)
	{
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %ls", address, port, gai_strerror(status));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully resolved host %s:%s", address, port);
	}

	// Create our server socket
	int handle = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

	// Bind it to the port we passed in to getaddrinfo():
	status = bind(handle, results->ai_addr, results->ai_addrlen);

	// Did we succeed?
	if (status == -1)
	{
		TraceLog(LOG_WARNING, "Failed to get bind socket to port (%s): %ls", port, gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully bound %s to port (%s)", address, port);
	}

	// Listen on the bound port
	status = listen(handle, 5);

	// Did we succeed?
	if (status == -1)
	{
		TraceLog(LOG_WARNING, "Failed to listen to socket: %ls", gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully started listen server.");
	}

	DWORD nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) == -1)
	{
		TraceLog(LOG_WARNING, "Failed to set socket to non-blocking.");
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully set socket to non-blocking.");
	}

	// Free the linked-list, we're not using it anymore
	freeaddrinfo(results);

	// Finally, return our socket descriptor
	tcpsock->handle = handle;

	return;
}

// Create a simple client, this combines Create/Bind/Connect into 1 function
void CreateClient(Socket* tcpsock, char* address, char* port, SocketType socketType)
{
	int              status;
	int              handle;
	struct addrinfo  hints;
	struct addrinfo* results; // Will point to the results

	memset(&hints, 0, sizeof hints); // Make sure the struct is empty
	hints.ai_family   = AF_UNSPEC; // Don't care IPv4 or IPv6
	hints.ai_socktype = socketType; // TCP stream sockets

	// Get ready to connect
	status = getaddrinfo(address, port, &hints, &results);

	// Did we succeed?
	if (status != 0)
	{
		TraceLog(LOG_WARNING, "Failed to get address information: %ls", gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully created TCP client on port (%s)", port);
	}

	// Create our socket
	handle = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

	// Did it succeed?
	if (handle == -1)
	{
		TraceLog(LOG_WARNING, "Failed to create socket: %ls", gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully created socket");
	}

	// Connect to the server
	status = connect(handle, results->ai_addr, results->ai_addrlen);

	if (status == -1)
	{
		TraceLog(LOG_WARNING, "Failed to connect to server %s:%s", address, port);
	}
	else
	{
		TraceLog(LOG_INFO, "Successfully connected to %s:%s", address, port);
	}

	freeaddrinfo(results);

	// Finally, return our socket descriptor
	tcpsock->handle = handle;
}

// Try to accept connections on ListenSock and store them in NewSock
void AcceptSocket(Socket listenSock, Socket* newSock)
{
	struct sockaddr_storage their_addr;
	socklen_t               addrSize;
	int                     newHandle;
	addrSize = sizeof their_addr;
	newHandle = accept(listenSock.handle, (struct sockaddr*) &their_addr, &addrSize);

	if (newHandle == INVALID_SOCKET)
	{
		TraceLog(LOG_DEBUG, "Failed to accept incoming connection: %ls", gai_strerror(errno));
	}
	else
	{
		newSock->handle = newHandle;
		TraceLog(LOG_INFO, "Successfully accepted a new connection.");
	}
}

// Send data over TCP
int SendTCP(SocketHandle handle, const char* data, int len)
{
	int sentBytes = send(handle, data, len, 0);
	if (sentBytes == SOCKET_ERROR)
	{
		TraceLog(LOG_WARNING, "Failed to send data: %ls", gai_strerror(errno));
	}
	else
	{
		TraceLog(LOG_DEBUG, "Successfully sent %d bytes.", sentBytes);
	}
}

// Receiive data over TCP
int ReceiveTCP(SocketHandle handle, const char* data, int len)
{
	int status = recv(handle, data, len, 0);
	if (status == SOCKET_ERROR && errno == EWOULDBLOCK)
	{
		TraceLog(LOG_DEBUG, "Failed to receive data: %ls", gai_strerror(errno));
	}
	else if (status > 0)
	{
		TraceLog(LOG_DEBUG, "Successfully received %d bytes.", status);
	}
	else if (status == 0)
	{
		TraceLog(LOG_INFO, "Connection closed.");
	}
	return status;
}

// Reset a socket
void ResetSocket(Socket* socket)
{
	socket->handle   = -1;
	socket->blocking = false;
};

// Resolve the hostname
void ResolveHost(const char* hostname)
{
	struct addrinfo hints, *res, *p;
	int             status;
	char            ipstr[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
	{
		TraceLog(LOG_WARNING, "getaddrinfo: %ls", gai_strerror(status));
		return 2;
	}

	TraceLog(LOG_INFO, "IP addresses for %s:", hostname);

	for (p = res; p != NULL; p = p->ai_next)
	{
		void* addr;
		char* ipver;

		// get the pointer to the address itself, different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET)
		{ // IPv4
			struct sockaddr_in* ipv4 = (struct sockaddr_in*) p->ai_addr;
			addr                     = &(ipv4->sin_addr);
			ipver                    = "IPv4";
		}
		else
		{ // IPv6
			struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) p->ai_addr;
			addr                      = &(ipv6->sin6_addr);
			ipver                     = "IPv6";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		TraceLog(LOG_INFO, "%s: %s", ipver, ipstr);
	}

	freeaddrinfo(res); // free the linked list
}

// Resolve the hostname
void ResolveIP(const char* ip)
{
	struct hostent* he;
	struct in_addr  ipv4addr;
	struct in6_addr ipv6addr;
	inet_pton(AF_INET, ip, &ipv4addr);
	he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
	if (he != NULL)
	{
		printf("Host name: %s\n", he->h_name);
	}
	inet_pton(AF_INET6, ip, &ipv6addr);
	he = gethostbyaddr(&ipv6addr, sizeof ipv6addr, AF_INET6);
	if (he != NULL)
	{
		printf("Host name: %s\n", he->h_name);
	}
}

// Print socket information
void PrintSocket(struct SocketAddress* addr, const int family, const int socktype, const int protocol)
{
	struct sockaddr_in*  sockaddr_ipv4;
	struct sockaddr_in6* sockaddr_ipv6;
	struct sockaddr*     sockaddr_ip;
	char                 ip4[INET_ADDRSTRLEN]; // space to hold the IPv4 string
	char                 ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
	switch (family)
	{
		case AF_UNSPEC:
		{
			TraceLog(LOG_DEBUG, "Family: Unspecified");
		}
		break;
		case AF_INET:
		{
			TraceLog(LOG_DEBUG, "Family: AF_INET (IPv4)");
			TraceLog(LOG_INFO, "- IPv4 address %s", SocketAddressToString(addr, ip4));
		}
		break;
		case AF_INET6:
		{
			TraceLog(LOG_DEBUG, "Family: AF_INET6 (IPv6)");
			TraceLog(LOG_INFO, "- IPv6 address %s", SocketAddressToString(addr, ip6));
		}
		break;
		case AF_NETBIOS:
		{
			TraceLog(LOG_DEBUG, "Family: AF_NETBIOS (NetBIOS)");
		}
		break;
		default:
		{
			TraceLog(LOG_DEBUG, "Family: Other %ld", family);
		}
		break;
	}
	TraceLog(LOG_DEBUG, "Socket type:");
	switch (socktype)
	{
		case 0:
			TraceLog(LOG_DEBUG, "- Unspecified");
			break;
		case SOCK_STREAM:
			TraceLog(LOG_DEBUG, "- SOCK_STREAM (stream)");
			break;
		case SOCK_DGRAM:
			TraceLog(LOG_DEBUG, "- SOCK_DGRAM (datagram)");
			break;
		case SOCK_RAW:
			TraceLog(LOG_DEBUG, "- SOCK_RAW (raw)");
			break;
		case SOCK_RDM:
			TraceLog(LOG_DEBUG, "- SOCK_RDM (reliable message datagram)");
			break;
		case SOCK_SEQPACKET:
			TraceLog(LOG_DEBUG, "- SOCK_SEQPACKET (pseudo-stream packet)");
			break;
		default:
			TraceLog(LOG_DEBUG, "- Other %ld", socktype);
			break;
	}
	TraceLog(LOG_DEBUG, "Protocol:");
	switch (protocol)
	{
		case 0:
			TraceLog(LOG_DEBUG, "- Unspecified");
			break;
		case IPPROTO_TCP:
			TraceLog(LOG_DEBUG, "- IPPROTO_TCP (TCP)");
			break;
		case IPPROTO_UDP:
			TraceLog(LOG_DEBUG, "- IPPROTO_UDP (UDP)");
			break;
		default:
			TraceLog(LOG_DEBUG, "- Other %ld", protocol);
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
