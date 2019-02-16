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
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

#define MMNOSOUND


//----------------------------------------------------------------------------------
// Platform defines
//----------------------------------------------------------------------------------
 
#define PLATFORM_WINDOWS 1 
#define PLATFORM_UNIX 2

#if defined(__WIN32__) || defined(WIN32)
#define PLATFORM PLATFORM_WINDOWS 
#else
#define PLATFORM PLATFORM_UNIX
#endif

//----------------------------------------------------------------------------------
// Platform specific network includes
//----------------------------------------------------------------------------------
 
#if PLATFORM_WINDOWS
#define __USE_W32_SOCKETS 
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <iphlpapi.h> 
#else /* UNIX */ 
#include <sys/types.h> 
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h> 
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#endif /* WIN32 */

/* System-dependent definitions */
#ifndef __USE_W32_SOCKETS 
#define closesocket close 
#define SOCKET  int
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1
#endif /* __USE_W32_SOCKETS */

#ifdef __USE_W32_SOCKETS
#define RNet_GetLastError WSAGetLastError
#define RNet_SetLastError WSASetLastError
#ifndef EINTR
#define EINTR WSAEINTR
#endif
#else
int  RNet_GetLastError(void);
void RNet_SetLastError(int err);
#endif 