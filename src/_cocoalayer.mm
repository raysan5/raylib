/*
raylib Objective-C++ Layer.
Copyright Annes Widow (annes12345678910) - zlib like license.
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
    NSImage *dockIcon = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
    CGDataProviderRef provider = CGDataProviderCreateWithData(nullptr, data, width*height*4, nullptr);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef cgImage = CGImageCreate(width, height, 8, 32, width*4, colorSpace,
                                       kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big,
                                       provider, nullptr, false, kCGRenderingIntentDefault);
    [dockIcon addRepresentation:[[NSBitmapImageRep alloc] initWithCGImage:cgImage]];
    [NSApp setApplicationIconImage:dockIcon];
    CGImageRelease(cgImage);
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);
}


#ifdef __cplusplus
}
#endif

#endif
