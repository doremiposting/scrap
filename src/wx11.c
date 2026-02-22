#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include "main.h"
#include "wx11.h"
#include "gfxgl.h"
#include "sfxalsa.h"

int screen;
XWindowAttributes wa = {0};
XSetWindowAttributes swa = {0};
XVisualInfo *vi;
XImage *i;
GC gc;
Atom wmdelwin;
GLXContext glc;
Colormap cmap;
Display *display;
Window window;
struct timespec thene, thenr, nowe, nowr, frmst, frmend;
int WWIDTH, WHEIGHT;
long long elapsede, elapsedr;

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display*, GLXFBConfig, GLXContext, Bool, const int*);

static int glxctxerr;
static int
glxctxerrhandler(Display *d, XErrorEvent *e) {
  (void)d; (void)e;
  glxctxerr = 1;
  return 0;
}

void
winit() {
  int fbcnt, i, j, k, smplbfrs, smpls, bestsmpls, dtype;
	int tdt, trt, tdb, tdep, tr, tg, tb, ta, tsb, tsamp;
  int tsel, seldt, selrt, seldb, seldep, selsb;
  int maj, min, usert, actualrt;
  GLXFBConfig *fbcs, bestfbc;
  int visattribs[] = {
    GLX_X_RENDERABLE, 1,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_BLUE_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_RED_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_DOUBLEBUFFER, True,
    None
  };
  int ctxattribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
    GLX_CONTEXT_MINOR_VERSION_ARB, 1,
    None
  };
  WWIDTH = 800;
  WHEIGHT = 600;
  glxctxerr = 0;
  display = XOpenDisplay(NULL);
  if (!display) { fprintf(stderr, "ERROR: Couldn't open display!\n"); exit(1); }
  screen = DefaultScreen(display);
  printf("GLX vendor: %s\n", glXGetClientString(display, GLX_VENDOR));
  glXQueryVersion(display, &maj, &min);
  glXQueryExtensionsString(display, screen);
  fbcs = glXChooseFBConfig(display, screen, visattribs, &fbcnt);
  if (!fbcs || fbcnt == 0) { fprintf(stderr, "No valid fbconfig found!\n"); exit(1); }
	fprintf(stderr, "fbcs available: %d\n", fbcnt);
  bestfbc = 0;
  bestsmpls = -1;
  tsel = -1;
  for (j = 0; j < fbcnt; j++) {
    i = 0;
    glXGetFBConfigAttrib(display, fbcs[j], GLX_RENDER_TYPE,    &trt);
    glXGetFBConfigAttrib(display, fbcs[j], GLX_DRAWABLE_TYPE,  &tdt);
    glXGetFBConfigAttrib(display, fbcs[j], GLX_DOUBLEBUFFER,   &tdb);
    glXGetFBConfigAttrib(display, fbcs[j], GLX_DEPTH_SIZE,     &tdep);
    glXGetFBConfigAttrib(display, fbcs[j], GLX_SAMPLE_BUFFERS, &tsb);
    glXGetFBConfigAttrib(display, fbcs[j], GLX_SAMPLES,        &tsamp);
    glXGetFBConfigAttrib(display, fbcs[j], GLX_RED_SIZE,       &tr);
    if (!(trt  & GLX_RGBA_BIT))   { continue; }
    if (!(tdt  & GLX_WINDOW_BIT)) { continue; }
    if (!tdb)                     { continue; }
    if (tdep < 24)                { continue; }
    if (tr   < 8)                 { continue; }
    if (tsb == 0 && tsamp >= bestsmpls) { continue; }  /* prefer no multisampling */
    tsel = j;
    seldt = tdt; selrt = trt; seldb = tdb; seldep = tdep; selsb = tsb;
    bestsmpls = tsamp;
    break;
  }
  tsel = 0;
  bestfbc = fbcs[tsel];
  fprintf(stderr, "Selected fbconfig: %d: dt: %d, rt: %d, db: %d, dp: %d, sb: %d\n", tsel, seldt, selrt, seldb, seldep, selsb);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_DRAWABLE_TYPE,  &tdt);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_RENDER_TYPE,    &trt);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_DOUBLEBUFFER,   &tdb);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_DEPTH_SIZE,     &tdep);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_RED_SIZE,       &tr);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_GREEN_SIZE,     &tg);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_BLUE_SIZE,      &tb);
	glXGetFBConfigAttrib(display, fbcs[tsel], GLX_ALPHA_SIZE,     &ta);
	fprintf(stderr, "[%d] drawable=%d render_type=%d db=%d depth=%d rgba=%d%d%d%d\n",
	                tsel, tdt, trt, tdb, tdep, tr, tg, tb, ta);
  if (!(bestfbc)) {
    fprintf(stderr, "No suitable fbconfig found!\n");
    exit(1);
  } else {
    glXGetFBConfigAttrib(display, bestfbc, GLX_DRAWABLE_TYPE, &dtype);
    fprintf(stderr, "drawable type: %x\n", dtype);
  }
  /* GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None}; */
  /* vi = glXChooseVisual(display, DefaultScreen(display), att); */
  vi = glXGetVisualFromFBConfig(display, bestfbc);
  if (!vi) { printf("No valid visual found\n"); return; }
  else { printf("visual: %ld\n", vi->visualid); }
  cmap = XCreateColormap(display, XRootWindow(display, vi->screen), vi->visual, AllocNone);
  wmdelwin = XInternAtom(display, "WM_DELETE_WINDOW", False);
  swa.colormap = cmap; swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
  swa.border_pixel = 0;
  window = XCreateWindow(
    display,
    XRootWindow(display, vi->screen),
    0, 0,
    (unsigned int)WWIDTH, (unsigned int)WHEIGHT, 0,
    vi->depth, InputOutput, vi->visual, CWColormap | CWBorderPixel | CWEventMask, &swa
  );
  XSync(display, 0);
  XGetWindowAttributes(display, window, &wa);
  XSetWMProtocols(display, window, &wmdelwin, 1);
  XSelectInput(display, window, swa.event_mask|PointerMotionMask);
  XStoreName(display, window, "Scrap");
  XMapWindow(display, window);
  XSync(display, 0);
  int (*oldxerr)(Display*, XErrorEvent*) = XSetErrorHandler(glxctxerrhandler);
  actualrt = 0;
  glXGetFBConfigAttrib(display, bestfbc, GLX_RENDER_TYPE, &actualrt);
  usert = (actualrt & GLX_RGBA_BIT) ? GLX_RGBA_TYPE : GLX_COLOR_INDEX_TYPE;
  glc = NULL;
  glxctxerr = 0;
  XSync(display, 0);
  #ifndef GLX_CONTEXT_MAJOR_VERSION_ARB
  #define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
  #endif
  #ifndef GLX_CONTEXT_MINOR_VERSION_ARB
  #define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
  #endif
	glXCreateContextAttribsARBProc createctxarb = 
    (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	if (createctxarb) {
    fprintf(stderr, "Using \"new\" arb method for context creation...\n");
    glc = createctxarb(display, bestfbc, NULL, 1, ctxattribs);
    XSync(display, 0);
	}
  if (glxctxerr || !glc) {
    fprintf(stderr, "\"new\" method failed, falling back on glXCreateNewContext...\n");
    glc = glXCreateNewContext(display, bestfbc, usert, NULL, 1);
  }
  if (glxctxerr || !glc) {
    fprintf(stderr, "glXCreateNewContext failed (render type=0x%x), trying glXCreateContext\n", usert);
    glxctxerr = 0;
    glc = glXCreateContext(display, vi, NULL, 1);
    XSync(display, 0);
    if (!glc) {
      fprintf(stderr, "GLX context could not be created!\n");
      exit(1);
    }
  }
  XSetErrorHandler(oldxerr);
  printf("Context created: %p  Is direct? %s\n",
       (void*)glc, glXIsDirect(display, glc) ? "yes" : "no (indirect)");
  //glXWaitX();
  glXMakeCurrent(display, window, glc) ? printf("bound gl context to window\n") : printf("Could not make gl context current\n");
  ginit();
  XFree(fbcs);
  XFree(vi);
}

void
winloop() {
  XEvent ev;
  int quit;
  quit = 0;
  GETNS(thene); GETNS(thenr);
  while (!quit) {
    /* TODO: Somehow we need to translate engine inputs, handled immediately, */
    /* into game inputs, handled on a per-tick basis. */
    while (XPending(display) > 0) {
      XNextEvent(display, &ev);
      switch (ev.type) {
        case ConfigureNotify:
          WWIDTH = ev.xconfigure.width;
          WHEIGHT = ev.xconfigure.height;
          resizegl();
          break;
        case KeyPress:
        switch (XLookupKeysym(&ev.xkey, 0)) {
          case 'f':
            doprofile = !doprofile;
            break;
          case 'w':
            wiremesh = !wiremesh;
            break;
          case 'q':
          case XK_Escape:
            quit = 1;
            break;
          case 'p':
          case XK_Pause:
            pausesim = !pausesim;
            break;
          default:
            break;
        }
        break;
        case MotionNotify: { /* event.xmotion.x, event.xmotion.y */ }
        break;
        case ClientMessage: {
          if ((Atom) ev.xclient.data.l[0] == wmdelwin) { quit = 1; }
        }
        break;
        default:
          /* if (ev.type == CompletionType) { } */
        break;

      }
    }
    GETNS(nowe);
    elapsede = DIFFNS(thene, nowe);
    if (elapsede > EVTICKNS) { GETNS(thene); }

    GETNS(nowr);
    elapsedr = DIFFNS(thenr, nowr);
    if (elapsedr > GFXTICKNS) {
      render();
    }
  }
}

void
wkill() {
  /* TODO: free() roundup from ginit(). */
	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}
