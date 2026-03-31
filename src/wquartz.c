#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "gfxsw.h"

#include "main.h"
#include "wquartz.h"

int screen;
XWindowAttributes wa = {0};
XSetWindowAttributes swa = {0};
XImage *i;
GC gc;
Atom wmdelwin;
Colormap cmap;
Display *display;
Window window;
struct timespec thene, thenr, nowe, nowr, frmst, frmend;
int WWIDTH, WHEIGHT;
long long elapsede, elapsedr;

static uint32_t *screenbuffer;
static XImage *ximg;

static void
initsb() {
  if (ximg) {
    ximg->data = NULL;
    XDestroyImage(ximg);
  }
  if (screenbuffer) { free(screenbuffer); }
  screenbuffer = calloc((size_t)WWIDTH * WHEIGHT, sizeof(uint32_t));
  if (!screenbuffer) { fprintf(stderr, "screenbuffer alloc failed\n"); exit(1); }
  ximg = XCreateImage(
      display, DefaultVisual(display, screen),
      (unsigned int)DefaultDepth(display, screen),
      ZPixmap, 0, (char *)screenbuffer,
      (unsigned int) WWIDTH, (unsigned int)WHEIGHT,
      32, 0
  );
  if (!ximg) { fprintf(stderr, "XCreateImage failed\n"); exit(1); }
}

static void
blitnstretch() {
  int x, y;
  for (y = 0; y < WHEIGHT; y++) {
    for (x = 0; x < WWIDTH; x++) {
      screenbuffer[y * WWIDTH + x] =
        framebuffer[(y * fbheight / WHEIGHT) * fbwidth
                    + (x * fbwidth / WWIDTH)];
    }
  }
  XPutImage(display, window, gc, ximg,
      0, 0, 0, 0, (unsigned int)WWIDTH, (unsigned int) WHEIGHT);
}

static void
blitnfit() {
  int bx, by, bw, bh, x, y;
  float scale;
  uint32_t *row;
  memset(screenbuffer, 0, (size_t)WWIDTH * WHEIGHT * sizeof(uint32_t));
  scale = (float)WWIDTH / (float)fbwidth;
  if (scale * (float)fbheight > (float)WHEIGHT) {
    scale = (float)WHEIGHT / (float)fbheight;
  }
  bw = (int)((float)fbwidth * scale);
  bh = (int)((float)fbheight * scale);
  bx = (WWIDTH - bw) / 2;
  by = (WHEIGHT - bh) / 2;
  for (y = 0; y < WHEIGHT; y++) {
    for (x = 0; x < WWIDTH; x++) {
      row[x] = framebuffer[(y * fbheight / bh) * fbwidth
                    + (x * fbwidth / bw)];
    }
  }
  XPutImage(display, window, gc, ximg,
      0, 0, 0, 0, (unsigned int)WWIDTH, (unsigned int) WHEIGHT);
}

void
flipbfrs() {
  blitnstretch();
  /* blitnfit(); */
}

void
winit() {
  XSetWindowAttributes swa;
  WWIDTH = 800;
  WHEIGHT = 600;
  display = XOpenDisplay(NULL);
  if (!display) { fprintf(stderr, "Can't open display!\n"); exit(1); }
  screen = DefaultScreen(display);
  swa.colormap = DefaultColormap(display, screen);
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | PointerMotionMask;
  window = XCreateWindow(
      display, RootWindow(display, screen),
      0, 0, (unsigned int)WWIDTH, (unsigned int)WHEIGHT,
      0, DefaultDepth(display, screen), InputOutput,
      DefaultVisual(display, screen),
      CWColormap | CWBorderPixel | CWEventMask, &swa
  );
  wmdelwin = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmdelwin, 1);
  XStoreName(display, window, "scrap on quartz");
  XMapWindow(display, window);
  XSync(display, 0);
  gc = XCreateGC(display, window, 0, NULL);
  initsb();
  ginit();
}

void
winloop() {
  XEvent ev;
  int quit;
  quit = 0;
  GETNS(thene);
  GETNS(thenr);
  while (!quit) {
    while(XPending(display) > 0) {
      XNextEvent(display, &ev);
      switch (ev.type) {
        case ConfigureNotify:
          WWIDTH = ev.xconfigure.width;
          WHEIGHT = ev.xconfigure.height;
          initsb();
          resizegfx(WWIDTH, WHEIGHT);
          break;
        case KeyPress:
          switch (XLookupKeysym(&ev.xkey, 0)) {
            case 'f':
              doprofile = !doprofile;
              break;
            case 'w':
              wiremesh = !wiremesh;
              break;
            case 'p':
              pausesim = !pausesim;
              break;
            case 'q':
            case XK_Escape:
              quit = 1;
              break;
            default: break;
          } break;
        case ClientMessage:
          if ((Atom)ev.xclient.data.l[0] == wmdelwin) { quit = 1; }
          break;
        default:
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
  if (ximg) { ximg->data = NULL; XDestroyImage(ximg); }
  free(screenbuffer);
  XFreeGC(display, gc);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}
