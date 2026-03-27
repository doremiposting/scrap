#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "gfx.h"
#include "gfxsw.h"
#include "gfxobj.h"
#include "event.h"
/* TODO: This will need guarding on a per-platform basis. */
#include "wx11.h"

int pausesim, wiremesh, doprofile;
uint32_t *framebuffer;
int fbwidth, fbheight;

static const struct { float ratio; int w, h; } reztbl[] = {
  { 4.0f / 3.0f, 640, 480 },
  { 5.0f / 4.0f, 600, 480 },
  { 16.0f / 10.0f, 768, 480 },
  { 16.0f / 9.0f, 854, 480 },
  { 21.0f / 9.0f, 1120, 280 },
};
#define NRES ((int)(sizeof(reztbl)/sizeof(reztbl[0])))

void
resizegfx(int ww, int wh) {
  int i, best;
  float ratio, diff, bestdiff;
  ratio = (wh > 0) ? (float)ww / (float)wh : 4.0f/3.0f;
  best = 0;
  bestdiff = fabsf(ratio - reztbl[0].ratio);
  for (i = 1; i < NRES; i++) {
    diff = fabsf(ratio - reztbl[i].ratio);
    if (diff < bestdiff) { bestdiff = diff; best = i; }
  }
  if (reztbl[best].w == fbwidth && reztbl[best].h == fbheight) { return; }
  fbwidth = reztbl[best].w; fbheight = reztbl[best].h;
  if (framebuffer) { free(framebuffer); }
  framebuffer = calloc(fbwidth * fbheight, sizeof(uint32_t));
  if (!framebuffer) { fprintf(stderr, "failed to alloc framebuffer data!\n"); exit(1); }
  fprintf(stderr, "SW render res: %dx%d (ratio %.3f)\n", fbwidth, fbheight, ratio);
}

void
ginit() {
  pausesim = 0; wiremesh = 0; doprofile = 1;
  framebuffer = NULL;
  fbheight = 0; fbwidth = 0;
  resizegfx(800, 600);
}

void
render() {
  struct timespec frmst, frmend;
  int x, y;
  uint32_t color;
  if (doprofile) { GETNS(frmst); }
  color = 0xFF69495eD;
  for (y = 0; y < fbheight; y++) {
    for (x = 0; x < fbwidth; x++) {
      framebuffer[y * fbwidth + x] = color;
    }
  }
  if (doprofile) {
    GETNS(frmend);
    fprintf(stderr, "\rFPS: %.2f  FT: %lld us",
        1000000000.0 / (double)elapsedr,
        DIFFNS(frmst, frmend) / 1000);
    fflush(stderr);
  }
}

void
gkill() {
  fprintf(stderr, "\rDone.                    \n");
  free(framebuffer);
  framebuffer = NULL;
}
