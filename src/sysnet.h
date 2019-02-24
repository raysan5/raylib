/**********************************************************************************************
*
*   sysnet - Provides cross-platform network defines, macros etc
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
*       Jak Barnes (github: @syphonx) (Feb. 2019):
*           - Initial version
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
// Platform type sizes
//----------------------------------------------------------------------------------

#include <limits.h>

//----------------------------------------------------------------------------------
// Undefine any conflicting windows.h symbols
//----------------------------------------------------------------------------------

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
#define NOSERVICE    // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND      // Sound driver routines
#define NOTEXTMETRIC // typedef TEXTMETRIC and associated routines
#define NOWH         // SetWindowsHook and WH_*
#define NOWINOFFSETS // GWL_*, GCL_*, associated routines
#define NOCOMM       // COMM driver routines
#define NOKANJI      // Kanji support stuff.
#define NOHELP       // Help engine interface.
#define NOPROFILER   // Profiler interface.
#define NODEFERWINDOWPOS // DeferWindowPos routines
#define NOMCX            // Modem Configuration Extensions

#define MMNOSOUND

//----------------------------------------------------------------------------------
// Platform defines
//----------------------------------------------------------------------------------

#define PLATFORM_WINDOWS 1
#define PLATFORM_LINUX 2

#if defined(__WIN32__) || defined(WIN32)
#	define PLATFORM PLATFORM_WINDOWS
#elif defined(_LINUX)
#	define PLATFORM PLATFORM_LINUX
#endif

//----------------------------------------------------------------------------------
// Platform type definitions
// From: https://github.com/DFHack/clsocket/blob/master/src/Host.h
//----------------------------------------------------------------------------------

#ifndef __WORDSIZE
#	define __WORDSIZE 32
#endif

#if defined(_LINUX) || defined(_DARWIN)
typedef unsigned char  uint8;
typedef char           int8;
typedef unsigned short uint16;
typedef short          int16;
typedef unsigned int   uint32;
typedef int            int32;
typedef int            SOCKET;
#endif

#ifdef WIN32
typedef unsigned char  uint8;
typedef char           int8;
typedef unsigned short uint16;
typedef short          int16;
typedef unsigned int   uint32;
typedef int            int32;
#endif

#ifdef WIN32
typedef int socklen_t;
#endif

#if defined(WIN32)
typedef unsigned long long int uint64;
typedef long long int          int64;
#elif (__WORDSIZE == 32)
__extension__ typedef long long int          int64;
__extension__ typedef unsigned long long int uint64;
#elif (__WORDSIZE == 64)
typedef unsigned long int uint64;
typedef long int          int64;
#endif

#ifdef WIN32
#	ifndef UINT8_MAX
#		define UINT8_MAX (UCHAR_MAX)
#	endif // UINT8_MAX
#	ifndef UINT16_MAX
#		define UINT16_MAX (USHRT_MAX)
#	endif // UINT16_MAX
#	ifndef UINT32_MAX
#		define UINT32_MAX (ULONG_MAX)
#	endif // UINT32_MAX
#	if __WORDSIZE == 64
#		define SIZE_MAX (18446744073709551615UL)
#	else
#		ifndef SIZE_MAX
#			define SIZE_MAX (4294967295U)
#		endif // SIZE_MAX
#	endif     // __WORDSIZE == 64
#endif         // WIN32

#if defined(WIN32)
#	define ssize_t size_t
#endif // WIN32

#ifndef TRUE
#	define TRUE 1
#endif // TRUE

#ifndef FALSE
#	define FALSE 0
#endif // FALSE 

#ifndef RESULT_SUCCESS
#	define RESULT_SUCCESS 0
#endif // RESULT_SUCCESS

#ifndef RESULT_FAILURE
#	define RESULT_FAILURE 1
#endif // RESULT_FAILURE

#ifndef INADDR_ANY
#	define INADDR_ANY 0x00000000
#endif // INADDR_ANY
#ifndef INADDR_NONE
#	define INADDR_NONE 0xFFFFFFFF
#endif // INADDR_NONE
#ifndef INADDR_LOOPBACK
#	define INADDR_LOOPBACK 0x7f000001
#endif // INADDR_LOOPBACK
#ifndef INADDR_BROADCAST
#	define INADDR_BROADCAST 0xFFFFFFFF
#endif // INADDR_BROADCAST

#ifndef htonll
#	ifdef _BIG_ENDIAN
#		define htonll(x) (x)
#		define ntohll(x) (x)
#	else
#		define htonll(x) ((((uint64) htonl(x)) << 32) + htonl(x >> 32))
#		define ntohll(x) ((((uint64) ntohl(x)) << 32) + ntohl(x >> 32))
#	endif // _BIG_ENDIAN
#endif     // htonll

//----------------------------------------------------------------------------------
// Platform specific network includes
// From: https://github.com/SDL-mirror/SDL_net/blob/master/SDLnetsys.h
//----------------------------------------------------------------------------------

// Include system network headers

#ifdef _WIN32
#	pragma comment(lib, "ws2_32.lib")
#	define __USE_W32_SOCKETS
#	include <winsock2.h>
#	include <Ws2tcpip.h>
#	include <io.h>
#	define IPTOS_LOWDELAY 0x10
#else /* UNIX */
#	include <sys/types.h>
#	include <fcntl.h>
#	include <netinet/in.h>
#	include <sys/ioctl.h>
#	include <sys/time.h>
#	include <unistd.h>
#	include <net/if.h>
#	include <netdb.h>
#	include <netinet/tcp.h>
#	include <sys/socket.h>
#endif /* WIN32 */

#ifndef INVALID_SOCKET
#	define INVALID_SOCKET ~(0)
#endif

#ifndef __USE_W32_SOCKETS
#	define closesocket close
#	define SOCKET int
#	define INVALID_SOCKET -1
#	define SOCKET_ERROR -1
#endif

#ifdef __USE_W32_SOCKETS
#	ifndef EINTR
#		define EINTR WSAEINTR
#	endif
#endif