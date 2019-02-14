/**********************************************************************************************
*
*   rnet - A simple and easy-to-use network module for raylib
*
*   FEATURES:
*       - Manage network stuff
*
*   DEPENDENCIES:
*       core.h    - core stuff
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
#include "config.h"         // Defines module configuration flags
#endif

#include "sysnet.h" 
#include "raylib.h" 
 
bool InitializeSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup
	(
		MAKEWORD(2, 2),
		&WsaData
	) == NO_ERROR;
#else
	return true;
#endif
}

void ShutdownSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

int CreateUDPSocket()
{
	int handle = socket(AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP);

	if (handle <= 0)
	{
		printf("failed to create socket\n");
		return false;
	}

	return handle;
}

bool OpenSocket(int handle, unsigned short port)
{
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)port);

	if (bind(handle, (const struct  sockaddr*)&address, sizeof( struct sockaddr_in)) < 0)
	{
		printf("failed to bind socket\n");
		return false;
	}

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

	int nonBlocking = 1;
	if (fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
	{
		printf("failed to set non-blocking\n");
		return false;
	}

#elif PLATFORM == PLATFORM_WINDOWS

	DWORD nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0)
	{
		printf("failed to set non-blocking\n");
		return false;
	}

#endif

	return true;
}

bool SendData(int handle, const Address* destination, const void* data, int size)
{
	int sent_bytes = sendto(handle, (const char*) data, size, 0, ( struct sockaddr*) &destination, sizeof(struct sockaddr_in));
	if (sent_bytes != size) {
		printf("failed to send packet\n");
		return false;
	}
}

int ReceiveData(int handle, Address* sender, void* data, int size)
{
#if PLATFORM == PLATFORM_WINDOWS
	typedef int socklen_t;
#endif	

	while (true) {
		socklen_t fromLength = sizeof(AddressToInt(*sender));
		int       bytes = recvfrom(handle, (char*) data, size, 0, (struct sockaddr*) &sender, &fromLength);
		if (bytes <= 0) break;
		return bytes;
	}
}

int AddressToInt(Address address)
{
	return (address.a << 24) | (address.b << 16) | (address.c << 8) | address.d;
}