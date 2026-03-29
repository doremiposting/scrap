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
}

void
winloop() {
}

void
wkill() {
  gkill();
  /* TODO: free() roundup from ginit(). */
}
