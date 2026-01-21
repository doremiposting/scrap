#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "gfx.h"
#include "gfxgl.h"
#include "event.h"

#define WWIDTH 800
#define WHEIGHT 600

Display *display;
Window window;
XWindowAttributes wa = {0};
XSetWindowAttributes swa;
XVisualInfo *vi;
XImage *i;
GC gc;
Atom wmdelwin;
Tri2df tri;
float a;
float cx, cy, mag, rad, dx, dy;
int mmx, mmy;
double da;
GLXContext glc;
Colormap cmap;

#define EVTICKNS 600000000LL
#define GFXTICKNS 16666667LL
#define GETNS(ts) (clock_gettime(CLOCK_MONOTONIC, &ts))
#define DIFFNS(start, end) \
    ((int64_t)((end).tv_sec - (start).tv_sec) * 1000000000LL + \
     ((end).tv_nsec - (start).tv_nsec))


void
ginit() {
  XEvent z;
  a = 0.0f;
  da = 60.0f; /* The sw render logic uses radians, opengl uses degrees. */
  cx = WWIDTH/2;
  cy = WHEIGHT/2;
  dx = 2.0;
  dy = 2.0;
  mmx = 1;
  mmy = 1;
  tri.x1 = 0.0f; tri.y1 = 100.0f;
  tri.x2 = -75.0f; tri.y2 = -50.0f;
  tri.x3 = 75.0f; tri.y3 = -50.0f;

  display = XOpenDisplay(NULL);
  if (!display) { fprintf(stderr, "ERROR: Couldn't open display!\n"); exit(1); }
  GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
  vi = glXChooseVisual(display, 0, att);
  if (!vi) { printf("No valid visual found\n"); return; }
  else { printf("visual: %p\n", vi->visualid); }
  cmap = XCreateColormap(display, XRootWindow(display, vi->screen), vi->visual, AllocNone);
  wmdelwin = XInternAtom(display, "WM_DELETE_WINDOW", False);
  swa.colormap = cmap; swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
  window = XCreateWindow(
    display,
    XRootWindow(display, vi->screen),
    0, 0,
    WWIDTH, WHEIGHT, 0,
    vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa
    );
  XGetWindowAttributes(display, window, &wa);
  XSetWMProtocols(display, window, &wmdelwin, 1);
  XSelectInput(display, window, StructureNotifyMask|KeyPressMask|PointerMotionMask);
  XStoreName(display, window, "Scrap");
  XMapWindow(display, window);
  for (;;) {
    XNextEvent(display, &z);
    if (z.type == MapNotify) { break; }
    else { printf("event type %d\n", z.type); }
  }
  glc = glXCreateContext(display, vi, NULL, 1);
  glXMakeCurrent(display, window, glc);
  glViewport(0, 0, WWIDTH, WHEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, WWIDTH, 0, WHEIGHT, -1, 1);
  glMatrixMode(GL_MODELVIEW);
}

void
render() {
  XEvent ev;
  int quit, xi, yj;
  struct timespec thene, thenr, nowe, nowr;
  long long elapsede, elapsedr;
  quit = 0;
  GETNS(thene);
  GETNS(thenr);
  while (!quit) {
    /* TODO: Pending events should be queued in realtime but executed in ticktime */
    /* Next loop should addevent() a queue of events which then get popped off with */
    /* handlenext() dispatching back to x11. EXCEPT FOR QUIT, WHICH SHOULD ALWAYS */
    /* TAKE IMMEDIATE PRIORITY. */
    while (XPending(display) > 0) {
      XNextEvent(display, &ev);
      switch (ev.type) {
        case KeyPress:
        switch (XLookupKeysym(&ev.xkey, 0)) {
          case 'q':
            quit = 1;
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
			glClearColor(0.39f, 0.58f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		#if 0
      tri.x1 = (int)(cx + cosf((float)da*0 + a)*mag);  tri.y1 = (int)(cy + sinf((float)da*0 + a)*mag);
      tri.x2 = (int)(cx + cosf((float)da*1 + a)*mag);  tri.y2 = (int)(cy + sinf((float)da*1 + a)*mag);
      tri.x3 = (int)(cx + cosf((float)da*2 + a)*mag);  tri.y3 = (int)(cy + sinf((float)da*2 + a)*mag);
      for (xi = 0; xi < WWIDTH; xi++) {
        for (yj = 0; yj < WHEIGHT; yj++) {
          pixels[yj*WWIDTH+xi] = intri2d(tri, xi, yj) ? tri2drbary(tri, xi, yj) : yj*WWIDTH+xi;
        }
      }
      /* printf("(%d, %d), (%d, %d), (%d, %d)\n", tri.x1, tri.y1, tri.x2, tri.y2, tri.x3, tri.y3); */
      XPutImage(display, window, gc, i, 0, 0, 0, 0, WWIDTH, WHEIGHT);
      a += 0.05f;
      cx += (dx*(float)mmx); if (cx > WWIDTH || cx < 0) { mmx *= -1; }
      cy += (dy*(float)mmy); if (cy > WHEIGHT || cy < 0) { mmy *= -1; }
		#endif
      glLoadIdentity();
      glTranslatef(cx, cy, 0.0f);
      glRotatef(a, 0.0f, 0.0f, 1.0f);
      glBegin(GL_TRIANGLES);
        glColor3f(1, 0, 0);
        glVertex2f(tri.x1, tri.y1);
        glColor3f(0, 1, 0);
        glVertex2f(tri.x2, tri.y2);
        glColor3f(0, 0, 1);
        glVertex2f(tri.x3, tri.y3);
      glEnd();
			glXSwapBuffers(display, window);
      XSync(display, 0);
      glFlush();
      a += 3.0f;
      cx += (dx*mmx);
      if (cx - rad < 0) { cx = rad; mmx *= -1; } if (cx + rad > WWIDTH) { cx = WWIDTH - rad; mmx *= -1; }
      cy += (dx*mmy);
      if (cy - rad < 0) { cy = rad; mmy *= -1; } if (cy + rad > WHEIGHT) { cy = WHEIGHT - rad; mmy *= -1; }
      GETNS(thenr);
    }
  }
}

void
gkill() {
  /* TODO: free() roundup from ginit(). */
	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

void foo() { printf("hi!\n"); }

static const evhandler evdispatch[NUMEVS] = {
  [SOMEEV] = foo,
  [NOTANEV] = NULL
};

/* TODO: Make an extra file with an extra dispatcher using */
/* __attribute__(weak) flags as a fallback if one doesn't exist/fails */
void
execev(Event e) {
  if (e < NUMEVS && evdispatch[e]) {
    evdispatch[e]();
  }
}
