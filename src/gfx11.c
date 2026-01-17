#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

#include <X11/Xlib.h>

#include "gfx.h"
#include "gfx11.h"
#include "event.h"

#define WWIDTH 800
#define WHEIGHT 600

Display *display;
Window window;
XWindowAttributes wa = {0};
XImage *i;
int *pixels;
GC gc;
Atom wmdelwin;
Tri2d tri;
float a;
float cx, cy, mag, dx, dy;
int mmx, mmy;
double da;

#define EVTICKNS 600000000LL
#define GFXTICKNS 16666667LL
#define GETNS(ts) (clock_gettime(CLOCK_MONOTONIC, &ts))
#define DIFFNS(start, end) \
    ((int64_t)((end).tv_sec - (start).tv_sec) * 1000000000LL + \
     ((end).tv_nsec - (start).tv_nsec))

void
ginit() {
  a = 0.0f;
  da = 2*M_PI/3;
  cx = WWIDTH/2;
  cy = WHEIGHT/2;
  dx = 2.0;
  dy = 2.0;
  mmx = 1;
  mmy = 1;
  mag = WWIDTH/4;
  tri.x1 = (int)(cx + cosf((float)da*0 + a)*mag);  tri.y1 = (int)(cy + sinf((float)da*0 + a)*mag);
  tri.x2 = (int)(cx + cosf((float)da*1 + a)*mag);  tri.y2 = (int)(cy + sinf((float)da*1 + a)*mag);
  tri.x3 = (int)(cx + cosf((float)da*2 + a)*mag);  tri.y3 = (int)(cy + sinf((float)da*2 + a)*mag);
  pixels = calloc(WWIDTH*WHEIGHT, sizeof(int));
  display = XOpenDisplay(NULL);
  if (!display) { fprintf(stderr, "ERROR: Couldn't open display!\n"); exit(1); }
  window = XCreateSimpleWindow(
    display,
    XDefaultRootWindow(display),
    0, 0,
    WWIDTH, WHEIGHT,
    0, 0, 0
    );
  XGetWindowAttributes(display, window, &wa);
  i = XCreateImage(display,
    wa.visual,
    (unsigned int)wa.depth,
    ZPixmap,
    0,
    (char*) pixels,
    WWIDTH,
    WHEIGHT,
    32,
    WWIDTH * sizeof(*pixels));
  gc = XCreateGC(display, window, 0, NULL);
  wmdelwin = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmdelwin, 1);
  XSelectInput(display, window, KeyPressMask|PointerMotionMask);
  XStoreName(display, window, "Scrap");
  XMapWindow(display, window);
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
      GETNS(thenr);
    }
  }
}

void
gkill() {
  /* TODO: free() roundup from ginit(). */
  XCloseDisplay(display);
  free(pixels);
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
