/*
*   _cocoalayer.mm - A Cocoa Layer for raylib.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2025 Annes Widow (@annes12345678910)
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
*/

#if defined(__APPLE__)

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#include "_cocoalayer.h"

#ifdef __cplusplus
extern "C" {
#endif

void CocoaSetDockIcon(unsigned char* data, int width, int height)
{
    // Create an empty NSImage
    NSImage *dockIcon = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];

    // Create a CGImage
    CGDataProviderRef provider = CGDataProviderCreateWithData(nullptr, data, width*height*4, nullptr);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef cgImage = CGImageCreate(width, height, 8, 32, width*4, colorSpace,
                                       kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big,
                                       provider, nullptr, false, kCGRenderingIntentDefault);

    // Keep the Icon Alive?
    [dockIcon addRepresentation:[[NSBitmapImageRep alloc] initWithCGImage:cgImage]];

    // set the app icon
    [NSApp setApplicationIconImage:dockIcon];

    // Free the garbage
    CGImageRelease(cgImage);
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);
}


#ifdef __cplusplus
}
#endif

#endif
