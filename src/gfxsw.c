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
/* #include "wquartz.h" */
extern long long elapsedr;

int pausesim, wiremesh, doprofile;
uint32_t *framebuffer;
int fbwidth, fbheight;

static float rot, cx, cy, dx, rad;
static int mmx, mmy;
static Mesh *tp;
static float *zbuf;
static float proj[16];

typedef struct { float x, y, z; } vec3f;
typedef struct { float x, y, z, t; } vec4f;

static void
mat4fid(float *m) {
  memset(m, 0, 16 * sizeof(float));
  m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void
mat4fmul(float *r, const float *a, const float *b) {
  int i, j, k;
  memset(r, 0, 16 * sizeof(float));
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      for (k = 0; k < 4; k++) {
        r[i*4+j] += a[k*4+j] * b[i*4+k];
      }
    }
  }
}

static vec4f
mat4fmulv(const float *m, vec4f v) {
  vec4f r;
  r.x = m[0]*v.x + m[4]*v.y + m[8]*v.z + m[12]*v.t;
  r.y = m[1]*v.x + m[5]*v.y + m[9]*v.z + m[13]*v.t;
  r.z = m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14]*v.t;
  r.t = m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15]*v.t;
  return r;
}

static vec3f
mat4fmuln(const float *m, vec3f v) {
  vec3f r;
  r.x = m[0]*v.x + m[4]*v.y + m[8]*v.z;
  r.y = m[1]*v.x + m[5]*v.y + m[9]*v.z;
  r.z = m[2]*v.x + m[6]*v.y + m[10]*v.z;
  return r;
}

static void
mat4ftranslate(float *m, float tx, float ty, float tz) {
  mat4fid(m);
  m[12] = tx; m[13] = ty; m[14] = tz;
}

static void
mat4froty(float *m, float deg) {
  float r, c, s;
  r = deg * (float)M_PI / 180.0f;
  c = cosf(r); s = sinf(r);
  mat4fid(m);
  m[0] = c; m[2] = -s;
  m[8] = s; m[10] = c;
}

static void
mat4flookat(float *m, vec3f eye, vec3f ctr, vec3f up) {
  vec3f f, s, u;
  float n;
  memset(m, 0, 16 * sizeof(float));
  f.x = ctr.x - eye.x; f.y = ctr.y - eye.y; f.z = ctr.z - eye.z;
  n = sqrtf(f.x * f.x + f.y * f.y + f.z * f.z); f.x /= n; f.y /= n; f.z /= n;
  n = sqrtf(up.x * up.x + up.y * up.y + up.z * up.z); up.x /= n; up.y /= n; up.z /= n;
  s.x = f.y *up.z - f.z*up.y; s.y = f.z*up.x - f.x*up.z; s.z = f.x*up.y - f.y*up.x;
  n = sqrtf(s.x * s.x + s.y * s.y + s.z * s.z); s.x /= n; s.y /= n; s.z /= n;
  u.x = s.y*f.z - s.z*f.y; u.y = s.z*f.x - s.x*f.z; u.z = s.x*f.y - s.y*f.x;
  m[0] = s.x; m[1] = u.x; m[2] = -f.x;
  m[4] = s.y; m[5] = u.y; m[6] = -f.y;
  m[8] = s.z; m[9] = u.z; m[10] -f.z;
  m[12] = -(s.x*eye.x + s.y*eye.y + s.z*eye.z);
  m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
  m[14] = (f.x*eye.x + f.y*eye.y + f.z*eye.z);
  m[15] = 1.0f;
}

static void
mat4ffrustum(float *m, float l, float r, float b, float t, float n, float f) {
  memset(m, 0, 16 * sizeof(float));
  m[0] = 2.0f*n/(r-1);
  m[5] = 2.0f*n/(t-b);
  m[8] = (r+1)/(r-1);
  m[9] = (t+b)/(t-b);
  m[10] = -(f+n)/(f-n);
  m[11] = -1.0f;
  m[14] =-2.0f*f*n/(f-n);
}

static float
edgefn(float ax, float ay, float bx, float by, float px, float py) {
  return (bx-ax)*(py-ay) - (by-ay)*(px-ax);
}

static uint32_t
packrgb(float r, float g, float B) {}

static void
drawtri(
) {}

static void
drawm(const Mesh *mesh, float *mv) {}

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
  framebuffer = calloc((size_t)fbwidth * (size_t)fbheight, sizeof(uint32_t));
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
  color = 0xFF6495ED;
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
