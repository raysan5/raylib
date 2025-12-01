#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#include "_cocoalayer.h"
// Example: make a global function you can call
extern "C" void CocoaSetDockIcon(unsigned char* data, int width, int height)
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
#endif
