#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <X11/Xlib.h>

#include "gfx.h"
#include "gfx11.h"

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
float cx, cy;
float da, mag;

void
ginit() {
  a = 0.0f;
  da = 2*M_PI/3;
  cx = WWIDTH/2;
  cy = WHEIGHT/2;
  mag = WWIDTH/4;
  tri.x1 = cx + cosf(da*0 + a)*mag;  tri.y1 = cy + sinf(da*0 + a)*mag;
  tri.x2 = cx + cosf(da*1 + a)*mag;  tri.y2 = cy + sinf(da*1 + a)*mag;
  tri.x3 = cx + cosf(da*2 + a)*mag;  tri.y3 = cy + sinf(da*2 + a)*mag;
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
  int quit, dorender, xi, yj;
  struct timespec thene, thenr, nowe, nowr;
  double elapsede, elapsedr;
  quit = 0;
  dorender = 10;
  clock_gettime(CLOCK_MONOTONIC, &thene);
  clock_gettime(CLOCK_MONOTONIC, &thenr);
  while (!quit) {
    clock_gettime(CLOCK_MONOTONIC, &nowe);
    elapsede = (nowe.tv_sec - thene.tv_sec) +
               (nowe.tv_nsec - thene.tv_nsec) / 1000000000.0;
    if (elapsede > 0.6f) {
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
      clock_gettime(CLOCK_MONOTONIC, &thene);
    }
    clock_gettime(CLOCK_MONOTONIC, &nowr);
    elapsedr = (nowr.tv_sec - thenr.tv_sec) +
               (nowr.tv_nsec - thenr.tv_nsec) / 1000000000.0;
    if (elapsedr > (1/60.0f)) {
      tri.x1 = cx + cosf(da*0 + a)*mag;  tri.y1 = cy + sinf(da*0 + a)*mag;
      tri.x2 = cx + cosf(da*1 + a)*mag;  tri.y2 = cy + sinf(da*1 + a)*mag;
      tri.x3 = cx + cosf(da*2 + a)*mag;  tri.y3 = cy + sinf(da*2 + a)*mag;
      for (xi = 0; xi < WWIDTH; xi++) {
        for (yj = 0; yj < WHEIGHT; yj++) {
          pixels[yj*WWIDTH+xi] = intri2d(tri, xi, yj) ? tri2drbary(tri, xi, yj) : yj*WWIDTH+xi;
        }
      }
      /* printf("(%d, %d), (%d, %d), (%d, %d)\n", tri.x1, tri.y1, tri.x2, tri.y2, tri.x3, tri.y3); */
      XPutImage(display, window, gc, i, 0, 0, 0, 0, WWIDTH, WHEIGHT);
      a += 0.001f;
      clock_gettime(CLOCK_MONOTONIC, &thene);
    }
  }
}

void
gkill() {
  /* TODO: free() roundup from ginit(). */
  XCloseDisplay(display);
  free(pixels);
}
