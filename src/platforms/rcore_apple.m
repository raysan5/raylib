/**********************************************************************************************
*
*   rcore_apple - UIKit host for raylib's C game loop
*
*   This software is provided under the zlib/libpng license. See LICENSE for details.
*
**********************************************************************************************/

#import <GameController/GameController.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/gl.h>
#import <QuartzCore/CAEAGLLayer.h>
#import <UIKit/UIKit.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>

#include "raylib_apple.h"
#include "rcore_apple.h"

enum { APPLE_TOUCH_CAPACITY = 128 };

@interface RaylibAppleView : UIView
- (void)resetTouches;
@end

@interface RaylibAppleHost : NSObject
- (void)attachView:(RaylibAppleView *)view;
- (void)detachView:(RaylibAppleView *)view;
- (void)setActive:(BOOL)active;
- (void)enqueueTouch:(AppleTouchEvent)touch;
- (BOOL)nextTouch:(AppleTouchEvent *)touch;
- (BOOL)waitFrame:(double)minimumTimestamp frame:(AppleFrame *)frame;
- (void)present;
@end

static int (*appleGameMain)(void) = NULL;
static RaylibAppleHost *appleHost = nil;
static _Thread_local void *appleRenderHost = NULL;

@implementation RaylibAppleView
{
    NSMapTable<UITouch *, NSNumber *> *_touches;
    int _nextTouchId;
}

+ (Class)layerClass { return [CAEAGLLayer class]; }

- (void)resetTouches { [_touches removeAllObjects]; }

#if TARGET_OS_IOS
- (void)forwardTouches:(NSSet<UITouch *> *)touches action:(AppleTouchAction)action
{
    if (_touches == nil) _touches = [NSMapTable weakToStrongObjectsMapTable];
    for (UITouch *touch in touches)
    {
        NSNumber *identifier = [_touches objectForKey:touch];
        if (identifier == nil)
        {
            if (action != APPLE_TOUCH_DOWN) continue;
            identifier = @(_nextTouchId);
            _nextTouchId = (_nextTouchId == INT_MAX)? 0 : _nextTouchId + 1;
            [_touches setObject:identifier forKey:touch];
        }

        CGPoint position = [touch locationInView:self];
        [appleHost enqueueTouch:(AppleTouchEvent){ .action = action, .id = identifier.intValue, .x = position.x, .y = position.y }];
        if ((action == APPLE_TOUCH_UP) || (action == APPLE_TOUCH_CANCEL)) [_touches removeObjectForKey:touch];
    }
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { [self forwardTouches:touches action:APPLE_TOUCH_DOWN]; }
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { [self forwardTouches:touches action:APPLE_TOUCH_MOVE]; }
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { [self forwardTouches:touches action:APPLE_TOUCH_UP]; }
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { [self forwardTouches:touches action:APPLE_TOUCH_CANCEL]; }
#endif
@end

@implementation RaylibAppleHost
{
    // The main thread owns the view and input producers. The condition guards the frame handoff
    NSCondition *_condition;
    RaylibAppleView *_view;
    CADisplayLink *_link;
    GCController *_controllers[APPLE_GAMEPADS];
    AppleFrame _pendingFrame;
    CAEAGLLayer *_pendingLayer;
    AppleTouchEvent _pendingTouches[APPLE_TOUCH_CAPACITY];
    int _pendingTouchCount;
    BOOL _resetInput;
    BOOL _active;
    BOOL _hasFrame;
    BOOL _parked;
    BOOL _started;
    BOOL _finished;

    // The rendering thread owns the OpenGL context and the current frame's input
    CAEAGLLayer *_drawableLayer;
    EAGLContext *_context;
    GLuint _framebuffer;
    GLuint _colorBuffer;
    GLuint _depthBuffer;
    int _allocatedWidth;
    int _allocatedHeight;
    int _drawableWidth;
    int _drawableHeight;
    double _lastTimestamp;
    double _framePeriod;
    double _nextDeadline;
    AppleTouchEvent _frameTouches[APPLE_TOUCH_CAPACITY];
    int _touchIndex;
    int _touchCount;
}

- (instancetype)init
{
    self = [super init];
    if (self != nil)
    {
        _condition = [[NSCondition alloc] init];
        _parked = YES;
    }
    return self;
}

- (void)attachView:(RaylibAppleView *)view
{
    [_condition lock];
    _view = view;
    [_condition unlock];

    CAEAGLLayer *layer = (CAEAGLLayer *)view.layer;
    layer.opaque = YES;
    layer.drawableProperties = @{ kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8,
                                  kEAGLDrawablePropertyRetainedBacking: @NO };
    _link = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayTick:)];
    _link.paused = YES;
    [_link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
}

- (void)detachView:(RaylibAppleView *)view
{
    if (_view != view) return;
    [self setActive:NO];
    [_link invalidate];
    _link = nil;

    [_condition lock];
    _view = nil;
    [_condition unlock];
}

- (void)setActive:(BOOL)active
{
    if (!active) [_view resetTouches];

    [_condition lock];
    _active = active;
    if (!active)
    {
        _hasFrame = NO;
        _pendingLayer = nil;
        _pendingTouchCount = 0;
        _resetInput = YES;
        [_condition broadcast];
        // Wait for the render thread to finish submitting GL commands before entering the background
        while (!_parked && !_finished) [_condition wait];
    }
    BOOL animate = active && !_finished;
    [_condition unlock];

    _link.paused = !animate;
}

- (void)enqueueTouch:(AppleTouchEvent)touch
{
    [_condition lock];
    if (_active)
    {
        if (_pendingTouchCount == APPLE_TOUCH_CAPACITY)
        {
            _pendingTouchCount = 0;
            _resetInput = YES;
        }
        _pendingTouches[_pendingTouchCount++] = touch;
    }
    [_condition unlock];
}

- (BOOL)nextTouch:(AppleTouchEvent *)touch
{
    if (_touchIndex >= _touchCount) return NO;
    *touch = _frameTouches[_touchIndex++];
    return YES;
}

- (void)sampleControllers:(AppleFrame *)frame
{
    NSArray<GCController *> *controllers = GCController.controllers;
    for (int i = 0; i < APPLE_GAMEPADS; i++)
    {
        if ((_controllers[i] != nil) && ![controllers containsObject:_controllers[i]]) _controllers[i] = nil;
    }

    for (GCController *controller in controllers)
    {
        GCExtendedGamepad *gamepad = controller.extendedGamepad;
        if (gamepad == nil)
        {
#if TARGET_OS_TV
            GCMicroGamepad *remote = controller.microGamepad;
            frame->remoteKeys[0] |= remote.dpad.up.isPressed;
            frame->remoteKeys[1] |= remote.dpad.right.isPressed;
            frame->remoteKeys[2] |= remote.dpad.down.isPressed;
            frame->remoteKeys[3] |= remote.dpad.left.isPressed;
            frame->remoteKeys[4] |= remote.buttonA.isPressed;
            frame->remoteKeys[5] |= remote.buttonX.isPressed;
#endif
            continue;
        }

        int slot = 0;
        while ((slot < APPLE_GAMEPADS) && (_controllers[slot] != controller)) slot++;
        if (slot == APPLE_GAMEPADS)
        {
            slot = 0;
            while ((slot < APPLE_GAMEPADS) && (_controllers[slot] != nil)) slot++;
            if (slot == APPLE_GAMEPADS) continue;
            _controllers[slot] = controller;
        }

        AppleGamepad *input = &frame->gamepads[slot];
        *input = (AppleGamepad){
            .connected = true,
            .buttons = {
                [GAMEPAD_BUTTON_LEFT_FACE_UP] = gamepad.dpad.up.isPressed,
                [GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = gamepad.dpad.right.isPressed,
                [GAMEPAD_BUTTON_LEFT_FACE_DOWN] = gamepad.dpad.down.isPressed,
                [GAMEPAD_BUTTON_LEFT_FACE_LEFT] = gamepad.dpad.left.isPressed,
                [GAMEPAD_BUTTON_RIGHT_FACE_UP] = gamepad.buttonY.isPressed,
                [GAMEPAD_BUTTON_RIGHT_FACE_RIGHT] = gamepad.buttonB.isPressed,
                [GAMEPAD_BUTTON_RIGHT_FACE_DOWN] = gamepad.buttonA.isPressed,
                [GAMEPAD_BUTTON_RIGHT_FACE_LEFT] = gamepad.buttonX.isPressed,
                [GAMEPAD_BUTTON_LEFT_TRIGGER_1] = gamepad.leftShoulder.isPressed,
                [GAMEPAD_BUTTON_RIGHT_TRIGGER_1] = gamepad.rightShoulder.isPressed,
                [GAMEPAD_BUTTON_LEFT_TRIGGER_2] = gamepad.leftTrigger.isPressed,
                [GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = gamepad.rightTrigger.isPressed,
                [GAMEPAD_BUTTON_MIDDLE_LEFT] = gamepad.buttonOptions.isPressed,
                [GAMEPAD_BUTTON_MIDDLE] = gamepad.buttonHome.isPressed,
                [GAMEPAD_BUTTON_MIDDLE_RIGHT] = gamepad.buttonMenu.isPressed,
                [GAMEPAD_BUTTON_LEFT_THUMB] = gamepad.leftThumbstickButton.isPressed,
                [GAMEPAD_BUTTON_RIGHT_THUMB] = gamepad.rightThumbstickButton.isPressed
            },
            .axes = { gamepad.leftThumbstick.xAxis.value, -gamepad.leftThumbstick.yAxis.value,
                      gamepad.rightThumbstick.xAxis.value, -gamepad.rightThumbstick.yAxis.value,
                      gamepad.leftTrigger.value*2.0f - 1.0f, gamepad.rightTrigger.value*2.0f - 1.0f }
        };
        NSString *name = controller.vendorName ?: controller.productCategory;
        snprintf(input->name, sizeof(input->name), "%s", name.UTF8String ?: "Apple game controller");
    }
}

- (void)displayTick:(CADisplayLink *)link
{
    UIWindow *window = _view.window;
    UIScreen *screen = window.windowScene.screen;
    CGSize bounds = _view.bounds.size;
    if ((screen == nil) || (bounds.width < 1) || (bounds.height < 1)) return;

    CGFloat scale = screen.scale;
    CAEAGLLayer *layer = (CAEAGLLayer *)_view.layer;
    if (_view.contentScaleFactor != scale) _view.contentScaleFactor = scale;
    if (layer.contentsScale != scale) layer.contentsScale = scale;
    AppleFrame frame = {
        .timestamp = link.targetTimestamp,
        .interval = link.targetTimestamp - link.timestamp,
        .width = (int)lround(bounds.width*scale),
        .height = (int)lround(bounds.height*scale),
        .pointWidth = bounds.width,
        .pointHeight = bounds.height,
        .displayWidth = (int)screen.nativeBounds.size.width,
        .displayHeight = (int)screen.nativeBounds.size.height,
        .refreshRate = (int)screen.maximumFramesPerSecond,
        .window = (__bridge void *)window
    };
    [self sampleControllers:&frame];

    [_condition lock];
    if (_active && !_finished)
    {
        _pendingFrame = frame;
        _pendingLayer = layer;
        _hasFrame = YES;
        if (!_started)
        {
            _started = YES;
            NSThread *thread = [[NSThread alloc] initWithBlock:^{ [self runGame]; }];
            thread.name = @"raylib rendering";
            [thread start];
        }
        [_condition signal];
    }
    [_condition unlock];
}

- (BOOL)bindDrawable:(CAEAGLLayer *)layer frame:(AppleFrame *)frame
{
    if (_context == nil)
    {
        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if ((_context == nil) || ![EAGLContext setCurrentContext:_context])
        {
            _context = nil;
            return NO;
        }

        glGenFramebuffers(1, &_framebuffer);
        glGenRenderbuffers(1, &_colorBuffer);
        glGenRenderbuffers(1, &_depthBuffer);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    if ((_drawableLayer != layer) || (_allocatedWidth != frame->width) || (_allocatedHeight != frame->height))
    {
        glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
        if (![_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer]) return NO;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_drawableWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_drawableHeight);
        if ((_drawableWidth <= 0) || (_drawableHeight <= 0)) return NO;
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorBuffer);

        glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _drawableWidth, _drawableHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return NO;

        _drawableLayer = layer;
        _allocatedWidth = frame->width;
        _allocatedHeight = frame->height;
    }
    frame->width = _drawableWidth;
    frame->height = _drawableHeight;
    frame->framebuffer = _framebuffer;
    return YES;
}

- (BOOL)waitFrame:(double)minimumTimestamp frame:(AppleFrame *)frame
{
    @autoreleasepool
    {
        [_condition lock];
        while (true)
        {
            if (!_active && !_parked)
            {
                [_condition unlock];
                if (_context != nil) glFinish();
                _lastTimestamp = 0.0;
                _nextDeadline = 0.0;
                _allocatedWidth = 0;
                [_condition lock];
                _parked = YES;
                [_condition broadcast];
            }

            double period = (_lastTimestamp > 0.0)? fmax(0.0, minimumTimestamp - _lastTimestamp) : 0.0;
            double deadline = (fabs(period - _framePeriod) < 0.000001)? _nextDeadline : minimumTimestamp;
            if (period <= 0.0) deadline = 0.0;
            if (_active && _hasFrame && (_pendingFrame.timestamp + _pendingFrame.interval*0.05 >= deadline))
            {
                *frame = _pendingFrame;
                if (_lastTimestamp > 0.0) frame->interval = frame->timestamp - _lastTimestamp;
                _lastTimestamp = frame->timestamp;
                _framePeriod = period;
                _nextDeadline = (deadline + period > frame->timestamp)? deadline + period : frame->timestamp + period;
                _hasFrame = NO;
                _parked = NO;

                frame->resetInput = _resetInput;
                _resetInput = NO;
                _touchIndex = 0;
                _touchCount = _pendingTouchCount;
                memcpy(_frameTouches, _pendingTouches, _touchCount*sizeof(AppleTouchEvent));
                _pendingTouchCount = 0;

                CAEAGLLayer *layer = _pendingLayer;
                [_condition unlock];
                return [self bindDrawable:layer frame:frame];
            }

            [_condition wait];
        }
    }
}

- (void)present
{
    [_condition lock];
    BOOL active = _active;
    [_condition unlock];
    if (!active || (_context == nil)) return;

    glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (BOOL)prepareWorkingDirectory
{
#if TARGET_OS_TV
    NSSearchPathDirectory directory = NSCachesDirectory;
#else
    NSSearchPathDirectory directory = NSApplicationSupportDirectory;
#endif
    NSFileManager *files = NSFileManager.defaultManager;
    NSURL *work = [files URLForDirectory:directory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:NULL];
    if (work == nil) return NO;

    NSURL *resources = [NSBundle.mainBundle.resourceURL URLByAppendingPathComponent:@"raylib_resources" isDirectory:YES];
    if ([files fileExistsAtPath:resources.path])
    {
        NSString *link = [work.path stringByAppendingPathComponent:@"resources"];
        NSString *destination = [files destinationOfSymbolicLinkAtPath:link error:NULL];
        if ((destination != nil) && ![destination isEqualToString:resources.path])
        {
            if (![files removeItemAtPath:link error:NULL]) return NO;
        }
        if (![files fileExistsAtPath:link] && ![files createSymbolicLinkAtPath:link withDestinationPath:resources.path error:NULL]) return NO;
    }
    return chdir(work.fileSystemRepresentation) == 0;
}

- (void)runGame
{
    @autoreleasepool
    {
        appleRenderHost = (__bridge void *)self;
        int originalDirectory = open(".", O_RDONLY | O_CLOEXEC);
        if ((originalDirectory >= 0) && [self prepareWorkingDirectory])
        {
            int result = appleGameMain();
            if (IsWindowReady()) CloseWindow();
            if (result != 0) TraceLog(LOG_WARNING, "APPLE: Game returned %i", result);
        }
        else TraceLog(LOG_WARNING, "APPLE: Could not open the game working directory");

        if (_context != nil)
        {
            glDeleteFramebuffers(1, &_framebuffer);
            glDeleteRenderbuffers(1, &_colorBuffer);
            glDeleteRenderbuffers(1, &_depthBuffer);
            glFinish();
            [EAGLContext setCurrentContext:nil];
            _context = nil;
            _drawableLayer = nil;
        }
        if (originalDirectory >= 0)
        {
            fchdir(originalDirectory);
            close(originalDirectory);
        }
        appleRenderHost = NULL;

        [_condition lock];
        _finished = YES;
        _parked = YES;
        [_condition broadcast];
        [_condition unlock];
        dispatch_async(dispatch_get_main_queue(), ^{ self->_link.paused = YES; });
    }
}
@end

@interface RaylibAppleSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property (nonatomic, strong) UIWindow *window;
@end

@implementation RaylibAppleSceneDelegate
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options
{
    UIWindow *window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    UIViewController *controller = [[UIViewController alloc] init];
    RaylibAppleView *view = [[RaylibAppleView alloc] initWithFrame:CGRectZero];
#if TARGET_OS_IOS
    view.multipleTouchEnabled = YES;
#endif
    controller.view = view;
    window.rootViewController = controller;
    self.window = window;
    [appleHost attachView:view];
    [window makeKeyAndVisible];
}

- (void)sceneDidBecomeActive:(UIScene *)scene { [appleHost setActive:YES]; }
- (void)sceneWillResignActive:(UIScene *)scene { [appleHost setActive:NO]; }

- (void)sceneDidDisconnect:(UIScene *)scene
{
    [appleHost detachView:(RaylibAppleView *)self.window.rootViewController.view];
    self.window = nil;
}
@end

@interface RaylibAppleAppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation RaylibAppleAppDelegate
- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options
{
    UISceneConfiguration *config = [[UISceneConfiguration alloc] initWithName:nil sessionRole:session.role];
    config.sceneClass = [UIWindowScene class];
    config.delegateClass = [RaylibAppleSceneDelegate class];
    return config;
}
@end

int RaylibRunAppleApp(int argc, char **argv, int (*gameMain)(void))
{
    if ((gameMain == NULL) || !NSThread.isMainThread || (appleHost != nil)) return 1;
    @autoreleasepool
    {
        appleGameMain = gameMain;
        appleHost = [[RaylibAppleHost alloc] init];
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([RaylibAppleAppDelegate class]));
    }
}

bool AppleWaitFrame(double minimumTimestamp, AppleFrame *frame)
{
    return [(__bridge RaylibAppleHost *)appleRenderHost waitFrame:minimumTimestamp frame:frame];
}

void ApplePresentFrame(void)
{
    [(__bridge RaylibAppleHost *)appleRenderHost present];
}

bool AppleNextTouch(AppleTouchEvent *touch)
{
    return [(__bridge RaylibAppleHost *)appleRenderHost nextTouch:touch];
}

void AppleOpenURL(const char *url)
{
#if TARGET_OS_IOS
    NSString *text = [NSString stringWithUTF8String:url];
    if (text == nil) return;
    dispatch_async(dispatch_get_main_queue(), ^{
        NSURL *link = [NSURL URLWithString:text];
        if (link != nil) [UIApplication.sharedApplication openURL:link options:@{} completionHandler:nil];
    });
#else
    (void)url;
#endif
}
