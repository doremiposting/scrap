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

static float a, cx, cy, dx, rad;
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
  m[8] = s.z; m[9] = u.z; m[10] = -f.z;
  m[12] = -(s.x*eye.x + s.y*eye.y + s.z*eye.z);
  m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
  m[14] = (f.x*eye.x + f.y*eye.y + f.z*eye.z);
  m[15] = 1.0f;
}

static void
mat4ffrustum(float *m, float l, float r, float b, float t, float n, float f) {
  memset(m, 0, 16 * sizeof(float));
  m[0] = 2.0f*n/(r-l);
  m[5] = 2.0f*n/(t-b);
  m[8] = (r+l)/(r-l);
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
packrgb(float r, float g, float b) {
  unsigned int ri, gi, bi;
  ri = (unsigned int)(r < 0.0f ? 0.0f : r > 1.0f ? 255.0f : r*255.0f + 0.5f);
  gi = (unsigned int)(g < 0.0f ? 0.0f : g > 1.0f ? 255.0f : g*255.0f + 0.5f);
  bi = (unsigned int)(b < 0.0f ? 0.0f : b > 1.0f ? 255.0f : b*255.0f + 0.5f);
  return 0xFF000000u | (ri << 16) | (gi << 8) | bi;
}

static void
drawtri(
    float sx0, float sy0, float sz0, float li0,
    float sx1, float sy1, float sz1, float li1,
    float sx2, float sy2, float sz2, float li2,
    uint32_t col
) {
  int minx, miny, maxx, maxy;
  float area, cr, cg, cb;
  int px, py;
  float pcx, pcy, w0, w1, w2, t0, t1, t2, depth, intense;
  int idx;
  minx = (int)fmaxf(0.0f, fminf(sx0, fminf(sx1, sx2)));
  miny = (int)fmaxf(0.0f, fminf(sy0, fminf(sy1, sy2)));
  maxx = (int)fminf((float)(fbwidth-1), ceilf(fmaxf(sx0, fmaxf(sx1, sx2))));
  maxy = (int)fminf((float)(fbheight-1), ceilf(fmaxf(sy0, fmaxf(sy1, sy2))));
  area = edgefn(sx0, sy0, sx1, sy1, sx2, sy2);
  cr = (float)((col>>16)&0xFF) / 255.0f;
  cg = (float)((col>>8)&0xFF) / 255.0f;
  cb = (float)(col&0xFF) / 255.0f;
  if (area <= 0.0f) { return; }
  for (py = miny ; py <= maxy ; py++) {
    for (px = minx ; px <= maxx ; px++) {
      pcx = (float)px + 0.5f;
      pcy = (float)py + 0.5;
      w0 = edgefn(sx1, sy1, sx2, sy2, pcx, pcy);
      w1 = edgefn(sx2, sy2, sx0, sy0, pcx, pcy);
      w2 = edgefn(sx0, sy0, sx1, sy1, pcx, pcy);
      if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
        t0 = w0/area; t1 = w1/area; t2 = w2/area;
        depth = t0 * sz0 + t1 * sz1 + t2 * sz2;
        intense = t0 * li0 + t1 * li1 + t2 * li2;
        idx = py*fbwidth + px;
        if (depth < zbuf[idx]) {
          zbuf[idx] = depth;
          framebuffer[idx] = packrgb(cr*intense, cg*intense, cb*intense);
        }
      }
    }
  }
}

static void
drawm(const Mesh *mesh, float *mv) {
  unsigned int i;
  float mvp[16];
  float hw, hh, lx, ly, lz;
  uint32_t col;
  vec4f cp0, cp1, cp2;
  float nx0, ny0, nz0, nx1, ny1, nz1, nx2, ny2, nz2;
  float sx0, sy0, sx1, sy1, sx2, sy2;
  float dz0, dz1, dz2;
  vec3f vn0, vn1, vn2;
  float li0, li1, li2;
  hw = (float)fbwidth * 0.5f;
  hh = (float)fbheight * 0.5f;
  lx = 0.577f; ly = 0.577f; lz = 0.577f;
  col = 0xFFE07010u;
  mat4fmul(mvp, proj, mv);
  for (i = 0; i+2 < mesh->cnt; i+= 3) {
    cp0 = mat4fmulv(mvp, (vec4f){mesh->v[i].x, mesh->v[i].y, mesh->v[i].z, 1.0f});
    cp1 = mat4fmulv(mvp, (vec4f){mesh->v[i+1].x, mesh->v[i+1].y, mesh->v[i+1].z, 1.0f});
    cp2 = mat4fmulv(mvp, (vec4f){mesh->v[i+2].x, mesh->v[i+2].y, mesh->v[i+2].z, 1.0f});
    if (cp0.t <= 0.0f || cp1.t <= 0.0f || cp2.t <= 0.0f) { continue; }
    nx0 = cp0.x / cp0.t; ny0 = cp0.y / cp0.t; nz0 = cp0.z / cp0.t;
    nx1 = cp1.x / cp1.t; ny1 = cp1.y / cp1.t; nz1 = cp1.z / cp1.t;
    nx2 = cp2.x / cp2.t; ny2 = cp2.y / cp2.t; nz2 = cp2.z / cp2.t;
    sx0 = hw*(nx0+1.0f); sy0 = hh*(1.0f-ny0);
    sx1 = hw*(nx1+1.0f); sy1 = hh*(1.0f-ny1);
    sx2 = hw*(nx2+1.0f); sy2 = hh*(1.0f-ny2);
    dz0 = (nz0 + 1.0f)*0.5f; dz1 = (nz1 + 1.0f)*0.5f; dz2 = (nz2 + 1.0f)*0.5f;
    vn0 = mat4fmuln(mv, (vec3f){mesh->v[i].nx, mesh->v[i].ny, mesh->v[i].nz});
    vn1 = mat4fmuln(mv, (vec3f){mesh->v[i+1].nx, mesh->v[i+1].ny, mesh->v[i+1].nz});
    vn2 = mat4fmuln(mv, (vec3f){mesh->v[i+2].nx, mesh->v[i+2].ny, mesh->v[i+2].nz});
    li0 = fmaxf(0.0f, vn0.x * lx + vn0.y * ly + vn0.z * lz) * 0.8f + 0.2f;
    li1 = fmaxf(0.0f, vn1.x * lx + vn1.y * ly + vn1.z * lz) * 0.8f + 0.2f;
    li2 = fmaxf(0.0f, vn2.x * lx + vn2.y * ly + vn2.z * lz) * 0.8f + 0.2f;
    drawtri(sx0, sy0, dz0, li0, sx2, sy2, dz2, li2, sx1, sy1, dz1, li1, col);
  }
}

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
  float ratio, diff, bestdiff, fovyrad, top, right;
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
  if (zbuf) { free(zbuf); }
  framebuffer = calloc((size_t)fbwidth * (size_t)fbheight, sizeof(uint32_t));
  zbuf = calloc((size_t)fbwidth * (size_t)fbheight, sizeof(float));
  if (!framebuffer) { fprintf(stderr, "failed to alloc framebuffer data!\n"); exit(1); }
  if (!zbuf) { fprintf(stderr, "failed to alloc zbuffer data!\n"); exit(1); }
  fovyrad = (float)(60.0 * (M_PI / 180.0));
  top = tanf((float)fovyrad * 0.5f) * 0.1f;
  right = top * ((float)fbwidth / (float)fbheight);
  mat4ffrustum(proj, -right, right, -top, top, 0.1f, 1000.0f);
  fprintf(stderr, "SW render res: %dx%d (ratio %.3f)\n", fbwidth, fbheight, ratio);
}

void
ginit() {
  pausesim = 0; wiremesh = 0; doprofile = 1;
  framebuffer = NULL;
  fbheight = 0; fbwidth = 0;
  a = 0.0f; cx = 0.0f; cy = 0.0f;
  dx = 0.05f; rad = 0.75f;
  mmx = 1; mmy = 1;
  tp = loadobj("assets/teapot.obj");
  resizegfx(800, 600);
}

void
render() {
  struct timespec frmst, frmend;
  float mv[16], t[16], tmp[16], r[16], view[16];
  int k, n;
  if (doprofile) { GETNS(frmst); }
  n = fbwidth * fbheight;
  for (k = 0; k < n ; k++) {
    framebuffer[k] = 0xFF6495EDu;
    zbuf[k] = 1.0f;
  }
  mat4flookat(view, (vec3f){3.0f, 3.0f, 3.0f}, (vec3f){0.0f, 0.0f, -4.5f},
      (vec3f){0.0f, 1.0f, 0.0f});
  mat4ftranslate(t, cx, cy, -5.0f);
  mat4froty(r, a);
  mat4fmul(tmp, t, r);
  mat4fmul(mv, view, tmp);
  drawm(tp, mv);
  if (!pausesim) {
    a += 3.0f;
    cx += dx * (float)mmx;
    if (cx > 3.8-rad) { cx = 3.8f - rad; mmx *= -1; }
    if (cx < -3.8-rad) {cx = -3.8f - rad; mmx *= -1; }
    cy += dx * (float)mmy;
    if (cy > 2.8f-rad) { cy = 2.8f - rad; mmy *= -1; }
    if (cy < -2.8f-rad) { cy = -2.8f - rad; mmy *= -1; }
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
  killmesh(tp);
  free(framebuffer); framebuffer = NULL;
  free(zbuf); zbuf = NULL;
}
