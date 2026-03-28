#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

#include <X11/Xlib.h>

#include "main.h"
#include "wquartz.h"

int screen;
XWindowAttributes wa = {0};
XSetWindowAttributes swa = {0};
XVisualInfo *vi;
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
  if (screenbuffer) { free(screenbuffer); exit(1); }
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
blitnstretch() {}

static void
blitnfit() {}

void
flipbfrs() {
  blitnstretch();
  /* blitnfit(); */
}

void
winit() {
}

void
winloop() {
}

void
wkill() {
  gkill();
  /* TODO: free() roundup from ginit(). */
}
