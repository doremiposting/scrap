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

static int visattribs[] = {
  GLX_RGBA,
  GLX_DOUBLEBUFFER,
  GLX_DEPTH_SIZE, 24,
  GLX_RED_SIZE, 8,
  GLX_GREEN_SIZE, 8,
  GLX_BLUE_SIZE, 8,
  None
};

void
flipbfrs() {
  glXSwapBuffers(display, window);
  //XSync(display, 0);
  //glFlush();
}

void
winit() {
  int fbcnt, i, j, k, smplbfrs, smpls, bestsmpls, dtype;
	int tdt, trt, tdb, tdep, tr, tg, tb, ta, tsb, tsamp;
  int tsel, seldt, selrt, seldb, seldep, selsb;
  int maj, min, usert, actualrt;
  WWIDTH = 800;
  WHEIGHT = 600;
  display = XOpenDisplay(NULL);
  if (!display) { fprintf(stderr, "ERROR: Couldn't open display!\n"); exit(1); }
  screen = DefaultScreen(display);
  vi = glXChooseVisual(display, screen, visattribs);
  if (!vi) { fprintf(stderr, "No suitable GLX visual!\n"); exit(1); }
  cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | PointerMotionMask;
  window = XCreateWindow(
      display, RootWindow(display, vi->screen),
      0, 0, (unsigned int)WWIDTH, (unsigned int)WHEIGHT,
      0, vi->depth, InputOutput, vi->visual,
      CWColormap | CWBorderPixel | CWEventMask, &swa
  );
  wmdelwin = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmdelwin, 1);
  XStoreName(display, window, "scrap");
  XMapWindow(display, window);
  XSync(display, 0);
  glc = glXCreateContext(display, vi, NULL, 1);
  if (!glc) { fprintf(stderr, "Cannot create oGL context!\n"); exit(1); }
  glXMakeCurrent(display, window, glc);
  fprintf(stderr, "GL Renderer: %s\n" "GL version: %s\n",
      glGetString(GL_RENDERER), glGetString(GL_VERSION));
  XFree(vi);
  ginit();
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
      GETNS(thenr);
      render();
      flipbfrs();
    }
  }
}

void
wkill() {
  gkill();
  /* TODO: free() roundup from ginit(). */
	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}
