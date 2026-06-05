#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/gl.h>
#import <QuartzCore/CADisplayLink.h>
#import <QuartzCore/CAFrameRateRange.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <dlfcn.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

extern int raylib_main(int argc, char *argv[]);

extern void ios_handle_touch_began(intptr_t touchId, float x, float y);
extern void ios_handle_touch_moved(intptr_t touchId, float x, float y);
extern void ios_handle_touch_ended(intptr_t touchId, float x, float y);
extern void ios_handle_touch_cancelled(intptr_t touchId, float x, float y);
extern void ios_request_close(void);
extern void ios_set_window_focused(bool focused);

typedef struct IOSBridgeState {
  int argc;
  char **argv;
  UIWindow *window;
  UIViewController *viewController;
  UIView *glView;
  EAGLContext *context;
  GLuint framebuffer;
  GLuint colorBuffer;
  GLuint depthBuffer;
  int screenWidth;
  int screenHeight;
  int renderWidth;
  int renderHeight;
  float scaleX;
  float scaleY;
  bool raylibStarted;
  bool shuttingDown;
  bool initialized;
} IOSBridgeState;

static IOSBridgeState ios = {0};

static dispatch_queue_t raylibQueue = nil;
static CADisplayLink *displayLink = nil;
static dispatch_semaphore_t frameSem = nil;
static int64_t vsyncCount = 0;

@interface RaylibGLView : UIView
@end

@implementation RaylibGLView

+ (Class)layerClass {
  return [CAEAGLLayer class];
}

@end

@interface RaylibViewController : UIViewController
@end

@implementation RaylibViewController

- (BOOL)prefersStatusBarHidden {
  return YES;
}

- (void)loadView {
  self.view = [[RaylibGLView alloc] initWithFrame:[UIScreen mainScreen].bounds];
  self.view.multipleTouchEnabled = YES;
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  ios_set_window_focused(true);
}

- (void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];
  ios_set_window_focused(false);
}

- (void)forwardTouches:(NSSet<UITouch *> *)touches action:(int)action {
  for (UITouch *touch in touches) {
    CGPoint point = [touch locationInView:self.view];
    intptr_t touchId = (intptr_t)(__bridge void *)touch;

    switch (action) {
    case 0:
      ios_handle_touch_began(touchId, point.x, point.y);
      break;
    case 1:
      ios_handle_touch_moved(touchId, point.x, point.y);
      break;
    case 2:
      ios_handle_touch_ended(touchId, point.x, point.y);
      break;
    case 3:
      ios_handle_touch_cancelled(touchId, point.x, point.y);
      break;
    default:
      break;
    }
  }
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  (void)event;
  [self forwardTouches:touches action:0];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  (void)event;
  [self forwardTouches:touches action:1];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  (void)event;
  [self forwardTouches:touches action:2];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches
               withEvent:(UIEvent *)event {
  (void)event;
  [self forwardTouches:touches action:3];
}

@end

@interface RaylibDisplayLinkTarget : NSObject
+ (void)startDisplayLink;
+ (void)onDisplayFrame:(CADisplayLink *)link;
+ (void)stopDisplayLink;
@end

@implementation RaylibDisplayLinkTarget

+ (void)startDisplayLink {
  if (displayLink != nil)
    return;

  frameSem = dispatch_semaphore_create(0);
  vsyncCount = 0;

  displayLink =
      [CADisplayLink displayLinkWithTarget:[self class]
                                  selector:@selector(onDisplayFrame:)];

  NSInteger maxFPS = [UIScreen mainScreen].maximumFramesPerSecond;

  // Use the display native refresh rate
  displayLink.preferredFrameRateRange =
      CAFrameRateRangeMake(60.0f, maxFPS, maxFPS);

  [displayLink addToRunLoop:[NSRunLoop mainRunLoop]
                    forMode:NSRunLoopCommonModes];
}

+ (void)onDisplayFrame:(CADisplayLink *)link {
  (void)link;
  __atomic_fetch_add(&vsyncCount, 1, __ATOMIC_RELEASE);
  dispatch_semaphore_signal(frameSem);
}

+ (void)stopDisplayLink {
  if (displayLink == nil)
    return;

  [displayLink invalidate];
  displayLink = nil;

  dispatch_semaphore_signal(frameSem);
}

@end

static void shutdown_internal(void) {
  [RaylibDisplayLinkTarget stopDisplayLink];

  if (ios.context != nil) {
    [EAGLContext setCurrentContext:ios.context];

    if (ios.framebuffer != 0)
      glDeleteFramebuffers(1, &ios.framebuffer);
    if (ios.colorBuffer != 0)
      glDeleteRenderbuffers(1, &ios.colorBuffer);
    if (ios.depthBuffer != 0)
      glDeleteRenderbuffers(1, &ios.depthBuffer);

    ios.framebuffer = 0;
    ios.colorBuffer = 0;
    ios.depthBuffer = 0;
    ios.context = nil;
  }

  ios.glView = nil;
  ios.viewController = nil;
  ios.window = nil;
  ios.initialized = false;
}

void ios_close_platform(void) {
  if (!ios.initialized || ios.shuttingDown)
    return;

  ios.shuttingDown = true;

  if ([NSThread isMainThread])
    shutdown_internal();
  else
    dispatch_async(dispatch_get_main_queue(), ^{
      shutdown_internal();
    });
}

@interface RaylibAppDelegate : UIResponder <UIApplicationDelegate>
@end

@interface RaylibSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(strong, nonatomic) UIWindow *window;
@end

static void ios_start_raylib_main(void) {
  if (ios.raylibStarted)
    return;

  ios.raylibStarted = true;

  raylibQueue =
      dispatch_queue_create("com.raylib.engine", DISPATCH_QUEUE_SERIAL);

  dispatch_async(raylibQueue, ^{
    [[NSThread currentThread] setName:@"com.raylib.engine"];

    // NSLog(@"[raylib][IOS] raylib thread: %@", [NSThread currentThread].name);

    raylib_main(ios.argc, ios.argv);
  });
}

@implementation RaylibAppDelegate

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  (void)application;
  (void)launchOptions;

  return YES;
}

- (UISceneConfiguration *)application:(UIApplication *)application
    configurationForConnectingSceneSession:
        (UISceneSession *)connectingSceneSession
                                   options:(UISceneConnectionOptions *)options {
  (void)application;
  (void)connectingSceneSession;
  (void)options;

  UISceneConfiguration *configuration = [UISceneConfiguration
      configurationWithName:@"Default Configuration"
                sessionRole:UIWindowSceneSessionRoleApplication];
  configuration.delegateClass = [RaylibSceneDelegate class];
  return configuration;
}

- (void)applicationWillResignActive:(UIApplication *)application {
  (void)application;
  ios_set_window_focused(false);
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
  (void)application;
  ios_set_window_focused(true);
}

- (void)applicationWillTerminate:(UIApplication *)application {
  (void)application;
  ios_request_close();
}

@end

@implementation RaylibSceneDelegate

- (void)scene:(UIScene *)scene
    willConnectToSession:(UISceneSession *)session
                 options:(UISceneConnectionOptions *)connectionOptions {
  (void)session;
  (void)connectionOptions;

  if (![scene isKindOfClass:[UIWindowScene class]])
    return;

  UIWindowScene *windowScene = (UIWindowScene *)scene;
  self.window = [[UIWindow alloc] initWithWindowScene:windowScene];
  ios.window = self.window;

  if (ios.viewController == nil) {
    ios.viewController = [[RaylibViewController alloc] init];
  }
  self.window.rootViewController = ios.viewController;
  [self.window makeKeyAndVisible];

  [RaylibDisplayLinkTarget startDisplayLink];

  ios_start_raylib_main();
}

- (void)sceneDidBecomeActive:(UIScene *)scene {
  (void)scene;
  ios_set_window_focused(true);
}

- (void)sceneWillResignActive:(UIScene *)scene {
  (void)scene;
  ios_set_window_focused(false);
}

- (void)sceneDidDisconnect:(UIScene *)scene {
  (void)scene;
  ios_request_close();
  ios_close_platform();
}

@end

static BOOL ios_setup_gl(void) {
  if (!ios.window || !ios.viewController)
    return NO;

  ios.glView = ios.viewController.view;
  CAEAGLLayer *eaglLayer = (CAEAGLLayer *)ios.glView.layer;
  eaglLayer.opaque = YES;
  eaglLayer.drawableProperties = @{
    kEAGLDrawablePropertyRetainedBacking : @NO,
    kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8
  };

  // Ensure layer uses native scale for proper retina resolution
  CGFloat nativeScale = [UIScreen mainScreen].nativeScale;
  eaglLayer.contentsScale = nativeScale;
  ios.glView.contentScaleFactor = nativeScale;

  ios.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
  if (!ios.context) {
    ios.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  }

  if (!ios.context)
    return NO;

  [EAGLContext setCurrentContext:ios.context];

  glGenFramebuffers(1, &ios.framebuffer);
  glGenRenderbuffers(1, &ios.colorBuffer);
  glGenRenderbuffers(1, &ios.depthBuffer);

  glBindFramebuffer(GL_FRAMEBUFFER, ios.framebuffer);

  glBindRenderbuffer(GL_RENDERBUFFER, ios.colorBuffer);
  if (![ios.context renderbufferStorage:GL_RENDERBUFFER
                           fromDrawable:eaglLayer]) {
    return NO;
  }

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, ios.colorBuffer);

  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,
                               &ios.renderWidth);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT,
                               &ios.renderHeight);

  glBindRenderbuffer(GL_RENDERBUFFER, ios.depthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ios.renderWidth,
                        ios.renderHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, ios.depthBuffer);

  GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
    NSLog(@"[raylib][IOS] Framebuffer incomplete: 0x%04X", fbStatus);
  } else {
    NSLog(@"[raylib][IOS] Framebuffer complete (%d x %d)", ios.renderWidth,
          ios.renderHeight);
  }

  [EAGLContext setCurrentContext:nil];

  return fbStatus == GL_FRAMEBUFFER_COMPLETE;
}

bool ios_initialize_window(int requestedWidth, int requestedHeight,
                           int *screenWidth, int *screenHeight,
                           int *renderWidth, int *renderHeight, float *scaleX,
                           float *scaleY) {
  (void)requestedWidth;
  (void)requestedHeight;

  __block bool ok = false;
  dispatch_sync(dispatch_get_main_queue(), ^{
    if (ios.window == nil) {
      ios.window =
          [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    }

    if (ios.viewController == nil) {
      ios.viewController = [[RaylibViewController alloc] init];
    }

    ios.window.rootViewController = ios.viewController;
    [ios.window makeKeyAndVisible];

    ok = ios_setup_gl();

    UIScreen *screen = [UIScreen mainScreen];
    CGSize boundsSize = screen.bounds.size;
    ios.screenWidth = (int)boundsSize.width;
    ios.screenHeight = (int)boundsSize.height;
    ios.scaleX = (float)screen.nativeScale;
    ios.scaleY = (float)screen.nativeScale;

    if (screenWidth)
      *screenWidth = ios.screenWidth;

    if (screenHeight)
      *screenHeight = ios.screenHeight;

    if (renderWidth)
      *renderWidth = ios.renderWidth;

    if (renderHeight)
      *renderHeight = ios.renderHeight;

    if (scaleX)
      *scaleX = ios.scaleX;

    if (scaleY)
      *scaleY = ios.scaleY;
  });

  ios.initialized = ok;
  return ok;
}

void ios_make_current_context(void) {
  if (ios.context != nil)
    [EAGLContext setCurrentContext:ios.context];

  if (ios.framebuffer != 0)
    glBindFramebuffer(GL_FRAMEBUFFER, ios.framebuffer);
  if (ios.colorBuffer != 0)
    glBindRenderbuffer(GL_RENDERBUFFER, ios.colorBuffer);

  glViewport(0, 0, ios.renderWidth, ios.renderHeight);
  //   NSLog(@"[raylib][IOS] IOSMakeCurrentContext: thread=%@, fb=%u, rb=%u, "
  //         @"viewport=%dx%d",
  //         [NSThread currentThread], ios.framebuffer, ios.colorBuffer,
  //         ios.renderWidth, ios.renderHeight);
}

void ios_present_frame(void) {
  if (ios.context == nil || ios.shuttingDown)
    return;

  if ([EAGLContext currentContext] != ios.context) {
    [EAGLContext setCurrentContext:ios.context];
  }

  //   NSLog(@"[raylib][IOS] IOSPresentFrame: thread=%@, currentContext=%@",
  //   [NSThread currentThread], [EAGLContext currentContext]);

  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
    NSLog(@"[raylib][IOS] glGetError before present: 0x%04X", err);

  [ios.context presentRenderbuffer:GL_RENDERBUFFER];

  err = glGetError();
  if (err != GL_NO_ERROR)
    NSLog(@"[raylib][IOS] glGetError after present: 0x%04X", err);
  if (frameSem != nil && !ios.shuttingDown) {
    static int64_t lastVsync = 0;
    int64_t current = __atomic_load_n(&vsyncCount, __ATOMIC_ACQUIRE);

    while ((current - lastVsync) < 1 && !ios.shuttingDown) {
      dispatch_semaphore_wait(frameSem, DISPATCH_TIME_FOREVER);
      current = __atomic_load_n(&vsyncCount, __ATOMIC_ACQUIRE);
    }

    lastVsync = current;
  }
}

void *ios_get_window_handle(void) { return (__bridge void *)ios.window; }

void ios_get_window_metrics(int *screenWidth, int *screenHeight,
                            int *renderWidth, int *renderHeight, float *scaleX,
                            float *scaleY) {
  if (screenWidth)
    *screenWidth = ios.screenWidth;

  if (screenHeight)
    *screenHeight = ios.screenHeight;

  if (renderWidth)
    *renderWidth = ios.renderWidth;

  if (renderHeight)
    *renderHeight = ios.renderHeight;

  if (scaleX)
    *scaleX = ios.scaleX;

  if (scaleY)
    *scaleY = ios.scaleY;
}

void *ios_get_proc_address(const char *name) {
  return dlsym(RTLD_DEFAULT, name);
}

void ios_open_url(const char *url) {
  if (!url) {
    NSLog(@"[raylib][IOS] invalid URL: null");
    return;
  }

  NSURLComponents *components = [NSURLComponents
      componentsWithString:[NSString stringWithUTF8String:url]];

  if (components == nil || components.scheme == nil) {

    NSLog(@"[raylib][IOS] invalid URL: %s, Scheme: %s", url,
          components ? components.scheme.UTF8String : "nil");
    return;
  }

  if (components.URL != nil) {

    dispatch_async(dispatch_get_main_queue(), ^{
      [[UIApplication sharedApplication] openURL:components.URL
                                         options:@{}
                               completionHandler:nil];
    });
  }
}

int main(int argc, char *argv[]) {
  ios.argc = argc;
  ios.argv = argv;

  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil,
                             NSStringFromClass([RaylibAppDelegate class]));
  }
}