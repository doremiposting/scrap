#include <stdio.h>
#include <stdlib.h>

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

void
ginit() {
  int y, z;
  int cx, cy;
  cx = WWIDTH/2;
  cy = WHEIGHT/2;
  Tri2d tri = {
    .x1 = cx - cx/2,
    .y1 = cy + cy/2,
    .x2 = cx + cx/2,
    .y2 = cy + cy/2,
    .x3 = cx,
    .y3 = cy - cy/2,
  };
  pixels = calloc(WWIDTH*WHEIGHT, sizeof(int));
  for (y = 0; y < WWIDTH; y++) {
    for (z = 0; z < WHEIGHT; z++) {
      pixels[y*WWIDTH+z] = intri2d(tri, y, z) ? tri2duv(tri, y, z) : y*WWIDTH+z;
    }
  }
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
  int quit;
  quit = 0;
  while (!quit) {
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
    XPutImage(display, window, gc, i, 0, 0, 0, 0, WWIDTH, WHEIGHT);
  }
}

void
gkill() {
  XCloseDisplay(display);
  free(pixels);
}
