#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>

#include "main.h"
#include "wcocoa.h"
#include "gfxgl.h"

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>

#define SEL_GET(name) sel_registerName(name)
#if 0
#define MSG_ID(receiver, selector, ...) \
    ((id (*)(id, SEL, ...))objc_msgSend)(receiver, selector, ##__VA_ARGS__)
#define MSG(receiver, selector, ...) \
    ((id (*)(id, SEL, ...))objc_msgSend)(receiver, selector, ##__VA_ARGS__)
#define MSG_BOOL(receiver, selector, ...) \
    ((BOOL (*)(id, SEL, ...))objc_msgSend)(receiver, selector, ##__VA_ARGS__)
#define MSG_VOID(receiver, selector, ...) \
    ((void (*)(id, SEL, ...))objc_msgSend)(receiver, selector, ##__VA_ARGS__)
#endif

Class NSApplication, NSWindow, NSAutoreleasePool,
      NSOpenGLContext, NSOpenGLPixelFormat,
      NSOpenGLView, NSDate;
id pool, app, win, ev, glctx, pxlfmt, cntview, ev, cur;
struct timespec thene, thenr, nowe, nowr, frmst, frmend;
long long elapsede, elapsedr;
CGRect rect;
int WWIDTH, WHEIGHT;

void
flipbfrs() {
  /* ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("update")); */
  ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("flushBuffer"));
}

void
winit() {
  GLenum err;
  unsigned long style;
  unsigned int attrs[] = {
    99, 0x1000, /* NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy (OpenGL 2.1) */
    5, /* NSOpenGLPFADoubleBuffer */
    73, /* NSOpenGLPFAAccelerated */
    8, 24, /* NSOpenGLPFAColorSize */
    12, 24, /* NSOpenGLPFADepthSize */
    0
  };
  NSAutoreleasePool = objc_getClass("NSAutoreleasePool");
  NSApplication = objc_getClass("NSApplication");
  NSWindow = objc_getClass("NSWindow");
  NSOpenGLContext = objc_getClass("NSOpenGLContext");
  NSOpenGLPixelFormat = objc_getClass("NSOpenGLPixelFormat");
  NSDate = objc_getClass("NSDate");
  NSOpenGLView = objc_getClass("NSOpenGLView");
  WWIDTH = 800;
  WHEIGHT = 600;
  pool = ((id (*)(id, SEL))objc_msgSend)((id)NSAutoreleasePool, SEL_GET("alloc"));
  pool = ((id (*)(id, SEL))objc_msgSend)(pool, SEL_GET("init"));
  app = ((id (*)(id, SEL))objc_msgSend)((id)NSApplication, SEL_GET("sharedApplication"));
  ((void (*)(id, SEL, long))objc_msgSend)(app, SEL_GET("setActivationPolicy:"), 0);
  ((void (*)(id, SEL))objc_msgSend)(app, SEL_GET("finishLaunching"));
  rect = CGRectMake(0, 0, WWIDTH, WHEIGHT);
  style = (1 << 0) | /* titled */
    (1 << 1) | /* closeable */
    (1 << 3); /* resizeable */
  win = ((id (*)(id, SEL))objc_msgSend)((id)NSWindow, SEL_GET("alloc"));
  win = ((id (*)(id, SEL, CGRect, unsigned long, unsigned long, BOOL))objc_msgSend)(
    win,
    sel_registerName("initWithContentRect:styleMask:backing:defer:"),
    rect, style, 2, /* NSBackingStoreBuffered */
    0
  );
  pxlfmt = ((id (*)(id, SEL))objc_msgSend)((id)NSOpenGLPixelFormat, SEL_GET("alloc"));
  pxlfmt = ((id (*)(id, SEL, const unsigned int *))objc_msgSend)(pxlfmt,
      SEL_GET("initWithAttributes:"), attrs);
  if (!pxlfmt) { fprintf(stderr, "pixel format creation failed\n"); return; }
  cntview = ((id (*)(id, SEL))objc_msgSend)((id)NSOpenGLView, SEL_GET("alloc"));
  cntview = ((id (*)(id, SEL, CGRect, id))objc_msgSend)(
      cntview, sel_registerName("initWithFrame:pixelFormat:"), rect, pxlfmt);
  /* glctx = ((id (*)(id, SEL))objc_msgSend)((id)NSOpenGLContext, SEL_GET("alloc")); */
  /* glctx = ((id (*)(id, SEL, id, id))objc_msgSend)(
      glctx, SEL_GET("initWithFormat:shareContext:"), pxlfmt, nil); */
  /* cntview = ((id (*)(id, SEL))objc_msgSend)(win, SEL_GET("contentView")); */
  ((void (*)(id, SEL))objc_msgSend)(win, SEL_GET("center"));
  /* ((void (*)(id, SEL, id))objc_msgSend)(glctx, SEL_GET("setView:"), cntview); */
  /* ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("makeCurrentContext")); */
  /* ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("update")); */
  ((void (*)(id, SEL, BOOL))objc_msgSend)(cntview, SEL_GET("setWantsBestResolutionOpenGLSurface:"), 1);
  ((void (*)(id, SEL, id))objc_msgSend)(win, sel_registerName("setContentView:"), cntview);
  ((void (*)(id, SEL, id))objc_msgSend)(win, SEL_GET("makeKeyAndOrderFront:"), nil);
  ((void (*)(id, SEL, BOOL))objc_msgSend)(app, SEL_GET("activateIgnoringOtherApps:"), 1);
  glctx = ((id (*)(id, SEL))objc_msgSend)(cntview, sel_registerName("openGLContext"));
  if (!glctx) { fprintf(stderr, "gl context creation failed\n"); return; }
  ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("makeCurrentContext"));
  ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("update"));
  cur = ((id (*)(id, SEL))objc_msgSend)((id)NSOpenGLContext, SEL_GET("currentContext"));
  fprintf(stderr, "glctx=%p current=%p\n", glctx, cur);
  const GLubyte *vendor = glGetString(GL_VENDOR);
  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  fprintf(stderr, "GL: %s | %s | %s\n",
          vendor ? (const char*)vendor : "NULL",
          renderer ? (const char*)renderer : "NULL",
          version ? (const char*)version : "NULL");
  err = glGetError();
  fprintf(stderr, "glGetError after makeCurrent: 0x%x\n", err);
  const GLubyte *ver = glGetString(GL_VERSION);
  (!ver) ? fprintf(stderr, "GL_VERSION is NULL\n") :
    fprintf(stderr, "GL_VERSION: %s\n", ver);
  ginit();
}

void
winloop() {
  int doquit;
  id distantpast, looppool;
  unsigned short type, keycode;
  GETNS(thene); GETNS(thenr);
  doquit = 0;
  distantpast = ((id (*)(id, SEL))objc_msgSend)((id)NSDate, SEL_GET("distantPast"));
  while (!doquit) {
    looppool = ((id (*)(id, SEL))objc_msgSend)((id)NSAutoreleasePool, SEL_GET("alloc"));
    looppool = ((id (*)(id, SEL))objc_msgSend)(looppool, SEL_GET("init"));
    ev = ((id (*)(id, SEL, unsigned long, id, id, BOOL))objc_msgSend)(
        app,
        SEL_GET("nextEventMatchingMask:untilDate:inMode:dequeue:"),
        ULONG_MAX,
        distantpast,
        (id)CFSTR("kCFRunLoopDefaultMode"),
        1);
    if (ev) {
      type = ((unsigned short (*)(id, SEL))objc_msgSend)
        (ev, SEL_GET("type"));
      if (type == 10) { /* 10 = keydown */
        keycode = ((unsigned short (*)(id, SEL))objc_msgSend)
           (ev, SEL_GET("keyCode"));
        if (keycode == 12) { /* 12 = Q on ANSI keybaord */
          doquit = !doquit;
        }
      }
      ((void (*)(id, SEL, id))objc_msgSend)(app, SEL_GET("sendEvent:"), ev);
    }
    GETNS(nowe);
    elapsede = DIFFNS(thene, nowe);
    if (elapsede > EVTICKNS) { GETNS(thene); }
    GETNS(nowr);
    elapsedr = DIFFNS(thenr, nowr);
    if (elapsedr > GFXTICKNS) {
      ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("makeCurrentContext"));
      /* ((void (*)(id, SEL))objc_msgSend)(glctx, SEL_GET("update")); */
      render();
    }
    ((void (*)(id, SEL))objc_msgSend)(looppool, sel_registerName("drain"));
  }
}

void
wkill () {
  gkill();
  ((void (*)(id, SEL))objc_msgSend)(pool, sel_registerName("drain"));
}
