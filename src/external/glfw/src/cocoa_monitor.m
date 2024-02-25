//========================================================================
// GLFW 3.4 macOS - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

#if defined(_GLFW_COCOA)

#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h>


// Get the name of the specified display, or NULL
//
static char* getMonitorName(CGDirectDisplayID displayID, NSScreen* screen)
{
    // IOKit doesn't work on Apple Silicon anymore
    // Luckily, 10.15 introduced -[NSScreen localizedName].
    // Use it if available, and fall back to IOKit otherwise.
    if (screen)
    {
        if ([screen respondsToSelector:@selector(localizedName)])
        {
            NSString* name = [screen valueForKey:@"localizedName"];
            if (name)
                return _glfw_strdup([name UTF8String]);
        }
    }

    io_iterator_t it;
    io_service_t service;
    CFDictionaryRef info;

    if (IOServiceGetMatchingServices(MACH_PORT_NULL,
                                     IOServiceMatching("IODisplayConnect"),
                                     &it) != 0)
    {
        // This may happen if a desktop Mac is running headless
        return _glfw_strdup("Display");
    }

    while ((service = IOIteratorNext(it)) != 0)
    {
        info = IODisplayCreateInfoDictionary(service,
                                             kIODisplayOnlyPreferredName);

        CFNumberRef vendorIDRef =
            CFDictionaryGetValue(info, CFSTR(kDisplayVendorID));
        CFNumberRef productIDRef =
            CFDictionaryGetValue(info, CFSTR(kDisplayProductID));
        if (!vendorIDRef || !productIDRef)
        {
            CFRelease(info);
            continue;
        }

        unsigned int vendorID, productID;
        CFNumberGetValue(vendorIDRef, kCFNumberIntType, &vendorID);
        CFNumberGetValue(productIDRef, kCFNumberIntType, &productID);

        if (CGDisplayVendorNumber(displayID) == vendorID &&
            CGDisplayModelNumber(displayID) == productID)
        {
            // Info dictionary is used and freed below
            break;
        }

        CFRelease(info);
    }

    IOObjectRelease(it);

    if (!service)
        return _glfw_strdup("Display");

    CFDictionaryRef names =
        CFDictionaryGetValue(info, CFSTR(kDisplayProductName));

    CFStringRef nameRef;

    if (!names || !CFDictionaryGetValueIfPresent(names, CFSTR("en_US"),
                                                 (const void**) &nameRef))
    {
        // This may happen if a desktop Mac is running headless
        CFRelease(info);
        return _glfw_strdup("Display");
    }

    const CFIndex size =
        CFStringGetMaximumSizeForEncoding(CFStringGetLength(nameRef),
                                          kCFStringEncodingUTF8);
    char* name = _glfw_calloc(size + 1, 1);
    CFStringGetCString(nameRef, name, size, kCFStringEncodingUTF8);

    CFRelease(info);
    return name;
}

// Check whether the display mode should be included in enumeration
//
static GLFWbool modeIsGood(CGDisplayModeRef mode)
{
    uint32_t flags = CGDisplayModeGetIOFlags(mode);

    if (!(flags & kDisplayModeValidFlag) || !(flags & kDisplayModeSafeFlag))
        return GLFW_FALSE;
    if (flags & kDisplayModeInterlacedFlag)
        return GLFW_FALSE;
    if (flags & kDisplayModeStretchedFlag)
        return GLFW_FALSE;

#if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
    CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) &&
        CFStringCompare(format, CFSTR(IO32BitDirectPixels), 0))
    {
        CFRelease(format);
        return GLFW_FALSE;
    }

    CFRelease(format);
#endif /* MAC_OS_X_VERSION_MAX_ALLOWED */
    return GLFW_TRUE;
}

// Convert Core Graphics display mode to GLFW video mode
//
static GLFWvidmode vidmodeFromCGDisplayMode(CGDisplayModeRef mode,
                                            double fallbackRefreshRate)
{
    GLFWvidmode result;
    result.width = (int) CGDisplayModeGetWidth(mode);
    result.height = (int) CGDisplayModeGetHeight(mode);
    result.refreshRate = (int) round(CGDisplayModeGetRefreshRate(mode));

    if (result.refreshRate == 0)
        result.refreshRate = (int) round(fallbackRefreshRate);

#if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
    CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) == 0)
    {
        result.redBits = 5;
        result.greenBits = 5;
        result.blueBits = 5;
    }
    else
#endif /* MAC_OS_X_VERSION_MAX_ALLOWED */
    {
        result.redBits = 8;
        result.greenBits = 8;
        result.blueBits = 8;
    }

#if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
    CFRelease(format);
#endif /* MAC_OS_X_VERSION_MAX_ALLOWED */
    return result;
}

// Starts reservation for display fading
//
static CGDisplayFadeReservationToken beginFadeReservation(void)
{
    CGDisplayFadeReservationToken token = kCGDisplayFadeReservationInvalidToken;

    if (CGAcquireDisplayFadeReservation(5, &token) == kCGErrorSuccess)
    {
        CGDisplayFade(token, 0.3,
                      kCGDisplayBlendNormal,
                      kCGDisplayBlendSolidColor,
                      0.0, 0.0, 0.0,
                      TRUE);
    }

    return token;
}

// Ends reservation for display fading
//
static void endFadeReservation(CGDisplayFadeReservationToken token)
{
    if (token != kCGDisplayFadeReservationInvalidToken)
    {
        CGDisplayFade(token, 0.5,
                      kCGDisplayBlendSolidColor,
                      kCGDisplayBlendNormal,
                      0.0, 0.0, 0.0,
                      FALSE);
        CGReleaseDisplayFadeReservation(token);
    }
}

// Returns the display refresh rate queried from the I/O registry
//
static double getFallbackRefreshRate(CGDirectDisplayID displayID)
{
    double refreshRate = 60.0;

    io_iterator_t it;
    io_service_t service;

    if (IOServiceGetMatchingServices(MACH_PORT_NULL,
                                     IOServiceMatching("IOFramebuffer"),
                                     &it) != 0)
    {
        return refreshRate;
    }

    while ((service = IOIteratorNext(it)) != 0)
    {
        const CFNumberRef indexRef =
            IORegistryEntryCreateCFProperty(service,
                                            CFSTR("IOFramebufferOpenGLIndex"),
                                            kCFAllocatorDefault,
                                            kNilOptions);
        if (!indexRef)
            continue;

        uint32_t index = 0;
        CFNumberGetValue(indexRef, kCFNumberIntType, &index);
        CFRelease(indexRef);

        if (CGOpenGLDisplayMaskToDisplayID(1 << index) != displayID)
            continue;

        const CFNumberRef clockRef =
            IORegistryEntryCreateCFProperty(service,
                                            CFSTR("IOFBCurrentPixelClock"),
                                            kCFAllocatorDefault,
                                            kNilOptions);
        const CFNumberRef countRef =
            IORegistryEntryCreateCFProperty(service,
                                            CFSTR("IOFBCurrentPixelCount"),
                                            kCFAllocatorDefault,
                                            kNilOptions);

        uint32_t clock = 0, count = 0;

        if (clockRef)
        {
            CFNumberGetValue(clockRef, kCFNumberIntType, &clock);
            CFRelease(clockRef);
        }

        if (countRef)
        {
            CFNumberGetValue(countRef, kCFNumberIntType, &count);
            CFRelease(countRef);
        }

        if (clock > 0 && count > 0)
            refreshRate = clock / (double) count;

        break;
    }

    IOObjectRelease(it);
    return refreshRate;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _glfwPollMonitorsCocoa(void)
{
    uint32_t displayCount;
    CGGetOnlineDisplayList(0, NULL, &displayCount);
    CGDirectDisplayID* displays = _glfw_calloc(displayCount, sizeof(CGDirectDisplayID));
    CGGetOnlineDisplayList(displayCount, displays, &displayCount);

    for (int i = 0;  i < _glfw.monitorCount;  i++)
        _glfw.monitors[i]->ns.screen = nil;

    _GLFWmonitor** disconnected = NULL;
    uint32_t disconnectedCount = _glfw.monitorCount;
    if (disconnectedCount)
    {
        disconnected = _glfw_calloc(_glfw.monitorCount, sizeof(_GLFWmonitor*));
        memcpy(disconnected,
               _glfw.monitors,
               _glfw.monitorCount * sizeof(_GLFWmonitor*));
    }

    for (uint32_t i = 0;  i < displayCount;  i++)
    {
        if (CGDisplayIsAsleep(displays[i]))
            continue;

        const uint32_t unitNumber = CGDisplayUnitNumber(displays[i]);
        NSScreen* screen = nil;

        for (screen in [NSScreen screens])
        {
            NSNumber* screenNumber = [screen deviceDescription][@"NSScreenNumber"];

            // HACK: Compare unit numbers instead of display IDs to work around
            //       display replacement on machines with automatic graphics
            //       switching
            if (CGDisplayUnitNumber([screenNumber unsignedIntValue]) == unitNumber)
                break;
        }

        // HACK: Compare unit numbers instead of display IDs to work around
        //       display replacement on machines with automatic graphics
        //       switching
        uint32_t j;
        for (j = 0;  j < disconnectedCount;  j++)
        {
            if (disconnected[j] && disconnected[j]->ns.unitNumber == unitNumber)
            {
                disconnected[j]->ns.screen = screen;
                disconnected[j] = NULL;
                break;
            }
        }

        if (j < disconnectedCount)
            continue;

        const CGSize size = CGDisplayScreenSize(displays[i]);
        char* name = getMonitorName(displays[i], screen);
        if (!name)
            continue;

        _GLFWmonitor* monitor = _glfwAllocMonitor(name, size.width, size.height);
        monitor->ns.displayID  = displays[i];
        monitor->ns.unitNumber = unitNumber;
        monitor->ns.screen     = screen;

        _glfw_free(name);

        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displays[i]);
        if (CGDisplayModeGetRefreshRate(mode) == 0.0)
            monitor->ns.fallbackRefreshRate = getFallbackRefreshRate(displays[i]);
        CGDisplayModeRelease(mode);

        _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_LAST);
    }

    for (uint32_t i = 0;  i < disconnectedCount;  i++)
    {
        if (disconnected[i])
            _glfwInputMonitor(disconnected[i], GLFW_DISCONNECTED, 0);
    }

    _glfw_free(disconnected);
    _glfw_free(displays);
}

// Change the current video mode
//
void _glfwSetVideoModeCocoa(_GLFWmonitor* monitor, const GLFWvidmode* desired)
{
    GLFWvidmode current;
    _glfwGetVideoModeCocoa(monitor, &current);

    const GLFWvidmode* best = _glfwChooseVideoMode(monitor, desired);
    if (_glfwCompareVideoModes(&current, best) == 0)
        return;

    CFArrayRef modes = CGDisplayCopyAllDisplayModes(monitor->ns.displayID, NULL);
    const CFIndex count = CFArrayGetCount(modes);
    CGDisplayModeRef native = NULL;

    for (CFIndex i = 0;  i < count;  i++)
    {
        CGDisplayModeRef dm = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
        if (!modeIsGood(dm))
            continue;

        const GLFWvidmode mode =
            vidmodeFromCGDisplayMode(dm, monitor->ns.fallbackRefreshRate);
        if (_glfwCompareVideoModes(best, &mode) == 0)
        {
            native = dm;
            break;
        }
    }

    if (native)
    {
        if (monitor->ns.previousMode == NULL)
            monitor->ns.previousMode = CGDisplayCopyDisplayMode(monitor->ns.displayID);

        CGDisplayFadeReservationToken token = beginFadeReservation();
        CGDisplaySetDisplayMode(monitor->ns.displayID, native, NULL);
        endFadeReservation(token);
    }

    CFRelease(modes);
}

// Restore the previously saved (original) video mode
//
void _glfwRestoreVideoModeCocoa(_GLFWmonitor* monitor)
{
    if (monitor->ns.previousMode)
    {
        CGDisplayFadeReservationToken token = beginFadeReservation();
        CGDisplaySetDisplayMode(monitor->ns.displayID,
                                monitor->ns.previousMode, NULL);
        endFadeReservation(token);

        CGDisplayModeRelease(monitor->ns.previousMode);
        monitor->ns.previousMode = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwFreeMonitorCocoa(_GLFWmonitor* monitor)
{
}

void _glfwGetMonitorPosCocoa(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    @autoreleasepool {

    const CGRect bounds = CGDisplayBounds(monitor->ns.displayID);

    if (xpos)
        *xpos = (int) bounds.origin.x;
    if (ypos)
        *ypos = (int) bounds.origin.y;

    } // autoreleasepool
}

void _glfwGetMonitorContentScaleCocoa(_GLFWmonitor* monitor,
                                      float* xscale, float* yscale)
{
    @autoreleasepool {

    if (!monitor->ns.screen)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Cocoa: Cannot query content scale without screen");
    }

    const NSRect points = [monitor->ns.screen frame];
    const NSRect pixels = [monitor->ns.screen convertRectToBacking:points];

    if (xscale)
        *xscale = (float) (pixels.size.width / points.size.width);
    if (yscale)
        *yscale = (float) (pixels.size.height / points.size.height);

    } // autoreleasepool
}

void _glfwGetMonitorWorkareaCocoa(_GLFWmonitor* monitor,
                                  int* xpos, int* ypos,
                                  int* width, int* height)
{
    @autoreleasepool {

    if (!monitor->ns.screen)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Cocoa: Cannot query workarea without screen");
    }

    const NSRect frameRect = [monitor->ns.screen visibleFrame];

    if (xpos)
        *xpos = frameRect.origin.x;
    if (ypos)
        *ypos = _glfwTransformYCocoa(frameRect.origin.y + frameRect.size.height - 1);
    if (width)
        *width = frameRect.size.width;
    if (height)
        *height = frameRect.size.height;

    } // autoreleasepool
}

GLFWvidmode* _glfwGetVideoModesCocoa(_GLFWmonitor* monitor, int* count)
{
    @autoreleasepool {

    *count = 0;

    CFArrayRef modes = CGDisplayCopyAllDisplayModes(monitor->ns.displayID, NULL);
    const CFIndex found = CFArrayGetCount(modes);
    GLFWvidmode* result = _glfw_calloc(found, sizeof(GLFWvidmode));

    for (CFIndex i = 0;  i < found;  i++)
    {
        CGDisplayModeRef dm = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
        if (!modeIsGood(dm))
            continue;

        const GLFWvidmode mode =
            vidmodeFromCGDisplayMode(dm, monitor->ns.fallbackRefreshRate);
        CFIndex j;

        for (j = 0;  j < *count;  j++)
        {
            if (_glfwCompareVideoModes(result + j, &mode) == 0)
                break;
        }

        // Skip duplicate modes
        if (j < *count)
            continue;

        (*count)++;
        result[*count - 1] = mode;
    }

    CFRelease(modes);
    return result;

    } // autoreleasepool
}

GLFWbool _glfwGetVideoModeCocoa(_GLFWmonitor* monitor, GLFWvidmode *mode)
{
    @autoreleasepool {

    CGDisplayModeRef native = CGDisplayCopyDisplayMode(monitor->ns.displayID);
    if (!native)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Cocoa: Failed to query display mode");
        return GLFW_FALSE;
    }

    *mode = vidmodeFromCGDisplayMode(native, monitor->ns.fallbackRefreshRate);
    CGDisplayModeRelease(native);
    return GLFW_TRUE;

    } // autoreleasepool
}

GLFWbool _glfwGetGammaRampCocoa(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    @autoreleasepool {

    uint32_t size = CGDisplayGammaTableCapacity(monitor->ns.displayID);
    CGGammaValue* values = _glfw_calloc(size * 3, sizeof(CGGammaValue));

    CGGetDisplayTransferByTable(monitor->ns.displayID,
                                size,
                                values,
                                values + size,
                                values + size * 2,
                                &size);

    _glfwAllocGammaArrays(ramp, size);

    for (uint32_t i = 0; i < size; i++)
    {
        ramp->red[i]   = (unsigned short) (values[i] * 65535);
        ramp->green[i] = (unsigned short) (values[i + size] * 65535);
        ramp->blue[i]  = (unsigned short) (values[i + size * 2] * 65535);
    }

    _glfw_free(values);
    return GLFW_TRUE;

    } // autoreleasepool
}

void _glfwSetGammaRampCocoa(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    @autoreleasepool {

    CGGammaValue* values = _glfw_calloc(ramp->size * 3, sizeof(CGGammaValue));

    for (unsigned int i = 0;  i < ramp->size;  i++)
    {
        values[i]                  = ramp->red[i] / 65535.f;
        values[i + ramp->size]     = ramp->green[i] / 65535.f;
        values[i + ramp->size * 2] = ramp->blue[i] / 65535.f;
    }

    CGSetDisplayTransferByTable(monitor->ns.displayID,
                                ramp->size,
                                values,
                                values + ramp->size,
                                values + ramp->size * 2);

    _glfw_free(values);

    } // autoreleasepool
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI CGDirectDisplayID glfwGetCocoaMonitor(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(kCGNullDirectDisplay);

    if (_glfw.platform.platformID != GLFW_PLATFORM_COCOA)
    {
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "Cocoa: Platform not initialized");
        return kCGNullDirectDisplay;
    }

    return monitor->ns.displayID;
}

#endif // _GLFW_COCOA

