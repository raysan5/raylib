#if !defined(_WIN32)
#   error "This module is only made for Windows OS"
#endif

#ifndef WIN32_CLIPBOARD_
#define WIN32_CLIPBOARD_
unsigned char *Win32GetClipboardImageData(int *width, int *height, unsigned long long int *dataSize);
#endif // WIN32_CLIPBOARD_

#ifdef WIN32_CLIPBOARD_IMPLEMENTATION
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// NOTE: These search for architecture is taken from "windows.h", and it's necessary to avoid including windows.h
// and still make it compile on msvc, because import indirectly importing "winnt.h" (e.g. <minwindef.h>) can cause problems is these are not defined.
#if !defined(_X86_) && !defined(_68K_) && !defined(_MPPC_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && !defined(_ARM64EC_) && defined(_M_IX86)
    #define _X86_
    #if !defined(_CHPE_X86_ARM64_) && defined(_M_HYBRID)
        #define _CHPE_X86_ARM64_
    #endif
#endif

#if !defined(_AMD64_) && !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && (defined(_M_AMD64) || defined(_M_ARM64EC))
    #define _AMD64_
#endif

#if !defined(_ARM_) && !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM64_) && !defined(_ARM64EC_) && defined(_M_ARM)
    #define _ARM_
#endif

#if !defined(_ARM64_) && !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64EC_) && defined(_M_ARM64)
    #define _ARM64_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_ARM_) && !defined(_ARM64_) && !defined(_ARM64EC_) && defined(_M_ARM64EC)
    #define _ARM64EC_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && !defined(_ARM64EC_) && defined(_M_M68K)
    #define _68K_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && !defined(_ARM64EC_) && defined(_M_MPPC)
    #define _MPPC_
#endif

#if !defined(_IA64_) && !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_M_IX86) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && !defined(_ARM64EC_) && defined(_M_IA64)
    #define _IA64_
#endif


#define WIN32_LEAN_AND_MEAN
// #include <sdkddkver.h>
// #include <windows.h>
// #include <winuser.h>
#include <minwindef.h>
// #include <minwinbase.h>

#ifndef WINAPI
    #if defined(_ARM_)
        #define WINAPI
    #else
        #define WINAPI __stdcall
    #endif
#endif

#ifndef WINAPI
    #if defined(_ARM_)
        #define WINAPI
    #else
        #define WINAPI __stdcall
    #endif
#endif

#ifndef WINBASEAPI
    #ifndef _KERNEL32_
        #define WINBASEAPI DECLSPEC_IMPORT
    #else
        #define WINBASEAPI
    #endif
#endif

#ifndef WINUSERAPI
    #ifndef _USER32_
        #define WINUSERAPI __declspec (dllimport)
    #else
        #define WINUSERAPI
    #endif
#endif

typedef int WINBOOL;

#if !defined(_WINUSER_) || !defined(WINUSER_ALREADY_INCLUDED)
WINUSERAPI WINBOOL WINAPI OpenClipboard(HWND hWndNewOwner);
WINUSERAPI WINBOOL WINAPI CloseClipboard(VOID);
WINUSERAPI DWORD   WINAPI GetClipboardSequenceNumber(VOID);
WINUSERAPI HWND    WINAPI GetClipboardOwner(VOID);
WINUSERAPI HWND    WINAPI SetClipboardViewer(HWND hWndNewViewer);
WINUSERAPI HWND    WINAPI GetClipboardViewer(VOID);
WINUSERAPI WINBOOL WINAPI ChangeClipboardChain(HWND hWndRemove, HWND hWndNewNext);
WINUSERAPI HANDLE  WINAPI SetClipboardData(UINT uFormat, HANDLE hMem);
WINUSERAPI HANDLE  WINAPI GetClipboardData(UINT uFormat);
WINUSERAPI UINT    WINAPI RegisterClipboardFormatA(LPCSTR  lpszFormat);
WINUSERAPI UINT    WINAPI RegisterClipboardFormatW(LPCWSTR lpszFormat);
WINUSERAPI int     WINAPI CountClipboardFormats(VOID);
WINUSERAPI UINT    WINAPI EnumClipboardFormats(UINT format);
WINUSERAPI int     WINAPI GetClipboardFormatNameA(UINT format, LPSTR  lpszFormatName, int cchMaxCount);
WINUSERAPI int     WINAPI GetClipboardFormatNameW(UINT format, LPWSTR lpszFormatName, int cchMaxCount);
WINUSERAPI WINBOOL WINAPI EmptyClipboard(VOID);
WINUSERAPI WINBOOL WINAPI IsClipboardFormatAvailable(UINT format);
WINUSERAPI int     WINAPI GetPriorityClipboardFormat(UINT *paFormatPriorityList, int cFormats);
WINUSERAPI HWND    WINAPI GetOpenClipboardWindow(VOID);
#endif

#ifndef HGLOBAL
    #define HGLOBAL void*
#endif

#if !defined(_WINBASE_) || !defined(WINBASE_ALREADY_INCLUDED)
WINBASEAPI SIZE_T  WINAPI GlobalSize (HGLOBAL hMem);
WINBASEAPI LPVOID  WINAPI GlobalLock (HGLOBAL hMem);
WINBASEAPI WINBOOL WINAPI GlobalUnlock (HGLOBAL hMem);
#endif

#if !defined(_WINGDI_) || !defined(WINGDI_ALREADY_INCLUDED)
#ifndef BITMAPINFOHEADER_ALREADY_DEFINED
#define BITMAPINFOHEADER_ALREADY_DEFINED
// Does this header need to be packed ? by the windowps header it doesnt seem to be
#pragma pack(push, 1)
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
#pragma pack(pop)
#endif

#ifndef BITMAPFILEHEADER_ALREADY_DEFINED
#define BITMAPFILEHEADER_ALREADY_DEFINED
#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER,*LPBITMAPFILEHEADER,*PBITMAPFILEHEADER;
#pragma pack(pop)
#endif

#ifndef RGBQUAD_ALREADY_DEFINED
#define RGBQUAD_ALREADY_DEFINED
typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD, *LPRGBQUAD;
#endif

// REF: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/4e588f70-bd92-4a6f-b77f-35d0feaf7a57
#define BI_RGB       0x0000
#define BI_RLE8      0x0001
#define BI_RLE4      0x0002
#define BI_BITFIELDS 0x0003
#define BI_JPEG      0x0004
#define BI_PNG       0x0005
#define BI_CMYK      0x000B
#define BI_CMYKRLE8  0x000C
#define BI_CMYKRLE4  0x000D
#endif

#ifndef BI_ALPHABITFIELDS
// Bitmap not compressed and that the color table consists of four DWORD color masks, 
// that specify the red, green, blue, and alpha components of each pixel
#define BI_ALPHABITFIELDS 0x0006
#endif

// REF: https://learn.microsoft.com/en-us/windows/win32/dataxchg/standard-clipboard-formats
#define CF_DIB 8

// REF: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setsystemcursor
// #define OCR_NORMAL      32512 // Normal     select
// #define OCR_IBEAM       32513 // Text       select
// #define OCR_WAIT        32514 // Busy
// #define OCR_CROSS       32515 // Precision  select
// #define OCR_UP          32516 // Alternate  select
// #define OCR_SIZENWSE    32642 // Diagonal   resize 1
// #define OCR_SIZENESW    32643 // Diagonal   resize 2
// #define OCR_SIZEWE      32644 // Horizontal resize
// #define OCR_SIZENS      32645 // Vertical   resize
// #define OCR_SIZEALL     32646 // Move
// #define OCR_NO          32648 // Unavailable
// #define OCR_HAND        32649 // Link       select
// #define OCR_APPSTARTING 32650 //

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
static BOOL OpenClipboardRetrying(HWND handle); // Open clipboard with a number of retries
static int GetPixelDataOffset(BITMAPINFOHEADER bih); // Get pixel data offset from DIB image

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
unsigned char *Win32GetClipboardImageData(int *width, int *height, unsigned long long int *dataSize)
{
    unsigned char *bmpData = NULL;
    
    if (OpenClipboardRetrying(NULL))
    {
        HGLOBAL clipHandle = (HGLOBAL)GetClipboardData(CF_DIB);
        if (clipHandle != NULL)
        {
            BITMAPINFOHEADER *bmpInfoHeader = (BITMAPINFOHEADER *)GlobalLock(clipHandle);
            if (bmpInfoHeader)
            {
                *width = bmpInfoHeader->biWidth;
                *height = bmpInfoHeader->biHeight;
                SIZE_T clipDataSize = GlobalSize(clipHandle);
                if (clipDataSize >= sizeof(BITMAPINFOHEADER))
                {
                    int pixelOffset = GetPixelDataOffset(*bmpInfoHeader);
                    
                    // Create the bytes for a correct BMP file and copy the data to a pointer
                    //------------------------------------------------------------------------
                    BITMAPFILEHEADER bmpFileHeader = { 0 };
                    SIZE_T bmpFileSize = sizeof(bmpFileHeader) + clipDataSize;
                    *dataSize = bmpFileSize;

                    bmpFileHeader.bfType = 0x4D42; // BMP fil type constant
                    bmpFileHeader.bfSize = (DWORD)bmpFileSize; // Up to 4GB works fine
                    bmpFileHeader.bfOffBits = sizeof(bmpFileHeader) + pixelOffset;

                    bmpData = (unsigned char *)RL_MALLOC(sizeof(bmpFileHeader) + clipDataSize);
                    memcpy(bmpData, &bmpFileHeader, sizeof(bmpFileHeader)); // Add BMP file header data
                    memcpy(bmpData + sizeof(bmpFileHeader), bmpInfoHeader, clipDataSize); // Add BMP info header data
                    
                    GlobalUnlock(clipHandle);
                    CloseClipboard();
                    
                    TRACELOG(LOG_INFO, "Clipboad image acquired successfully");
                    //------------------------------------------------------------------------
                }
                else
                {
                    TRACELOG(LOG_WARNING, "Clipboard data is malformed");
                    GlobalUnlock(clipHandle);
                    CloseClipboard();
                }
            }
            else 
            {
                TRACELOG(LOG_WARNING, "Clipboard data failed to be locked");
                GlobalUnlock(clipHandle);
                CloseClipboard();
            }
        }
        else 
        {
            TRACELOG(LOG_WARNING, "Clipboard data is not an image");
            CloseClipboard();
        }
    }
    else TRACELOG(LOG_WARNING, "Clipboard can not be opened");

    return bmpData;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
// Open clipboard with several tries
// NOTE: If parameter is NULL, the open clipboard is associated with the current task
static BOOL OpenClipboardRetrying(HWND hWnd)
{
    static const int maxTries = 20;
    static const int sleepTimeMS = 60;
    
    for (int i = 0; i < maxTries; i++)
    {
        // Might be being hold by another process
        // Or yourself forgot to CloseClipboard
        if (OpenClipboard(hWnd)) return true;

        Sleep(sleepTimeMS);
    }
    
    return false;
}

// Get the byte offset where does the pixels data start (from a packed DIB)
// REF: https://stackoverflow.com/questions/30552255/how-to-read-a-bitmap-from-the-windows-clipboard#30552856
// REF: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
static int GetPixelDataOffset(BITMAPINFOHEADER bih)
{
    int offset = 0;
    const unsigned int rgbaSize = sizeof(RGBQUAD);

    // NOTE: biSize specifies the number of bytes required by the structure
    // It's expected to be always 40 because it should be packed
    if ((bih.biSize == 40) && (sizeof(BITMAPINFOHEADER) == 40))
    {
        // NOTE: biBitCount specifies the number of bits per pixel
        // Might exist some bit masks *after* the header and *before* the pixel offset
        // we're looking, but only if more than 8 bits per pixel, so it needs to be ajusted for that
        if (bih.biBitCount > 8)
        {
            // If (bih.biCompression == BI_RGB) no need to be offset more

            if (bih.biCompression == BI_BITFIELDS) offset += 3*rgbaSize;
            else if (bih.biCompression == BI_ALPHABITFIELDS) offset += 4 * rgbaSize; // Not widely supported, but valid
        }
    }

    // NOTE: biClrUsed specifies the number of color indices in the color table that are actually used by the bitmap
    // If this value is zero, the bitmap uses the maximum number of colors
    // corresponding to the value of the biBitCount member for the compression mode specified by biCompression
    // If biClrUsed is nonzero and the biBitCount member is less than 16
    // the biClrUsed member specifies the actual number of colors
    if (bih.biClrUsed > 0) offset += bih.biClrUsed*rgbaSize;
    else
    {
        if (bih.biBitCount < 16) offset = offset + (rgbaSize << bih.biBitCount);
    }

    return bih.biSize + offset;
}
#endif // WIN32_CLIPBOARD_IMPLEMENTATION
