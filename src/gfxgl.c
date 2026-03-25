#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#elifdef __APPLE__
#include <OpenGL/gl.h>
#endif

#include "gfx.h"
#include "gfxgl.h"
#ifdef __linux__
#include "wx11.h"
#elifdef __APPLE__
#include "wcocoa.h"
#endif
#include "gfxobj.h"
#ifdef __linux__
#include "sfxalsa.h"
#elifdef __APPLE__
#include "sfxcore.h"
#endif
#include "event.h"

float a;
float cx, cy, mag, rad, dx, dy;
int mmx, mmy;
double da;
Mesh *tp;
static float fovy;
int pausesim, wiremesh, doprofile;

void
lookat(float ex, float ey, float ez,
       float cx, float cy, float cz,
       float ux, float uy, float uz) {
  float fx = cx - ex;
  float fy = cy - ey;
  float fz = cz - ez;
  float fl = sqrtf(fx*fx + fy*fy + fz*fz);
  fx /= fl; fy /= fl; fz /= fl;

  float ul = sqrtf(ux*ux + uy*uy + uz*uz);
  ux /= ul; uy /= ul; uz /= ul;

  float sx = fy*uz - fz*uy;
  float sy = fz*ux - fx*uz;
  float sz = fx*uy - fy*ux;

  float ux2 = sy*fz - sz*fy;
  float uy2 = sz*fx - sx*fz;
  float uz2 = sx*fy - sy*fx;

  float m[16] = {
    sx,  ux2, -fx, 0,
    sy,  uy2, -fy, 0,
    sz,  uz2, -fz, 0,
    0,   0,    0,  1
  };

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixf(m);
  glTranslatef(-ex, -ey, -ez);
}

void
drawm(const Mesh *m) {
  unsigned int i;
  wiremesh ? glPolygonMode(GL_FRONT, GL_LINE)
    : glPolygonMode(GL_FRONT, GL_FILL);
  glBegin(GL_TRIANGLES);
  for (i = 0; i < m->cnt; i++) {
    glNormal3f(m->v[i].nx, m->v[i].ny, m->v[i].nz);
    glTexCoord2f(m->v[i].u, m->v[i].v);
    glVertex3f(m->v[i].x, m->v[i].y, m->v[i].z);
  }
  glEnd();
}

void
resizegl() {
  float fh, fw;
  if (WHEIGHT == 0) { WHEIGHT = 1; }
  glViewport(0, 0, WWIDTH, WHEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  fh = tanf(fovy * 0.5f * ((float)(M_PI) / 180.f)) * 0.1f;
  fw = fh * (float)WWIDTH/(float)WHEIGHT;
  glFrustum(-fw, fw, -fh, fh, 0.1f, 100.0f);

  glMatrixMode(GL_MODELVIEW);
}

void
ginit() {
  tp = loadobj("assets/teapot.obj");
  a = 0.0f;
  da = 60.0f; /* The sw render logic uses radians, opengl uses degrees. */
  cx = 0.0f;
  cy = 0.0f;
  dx = 0.05f;
  dy = 0.05f;
  mmx = 1;
  mmy = 1;
  rad = 0.75;
  float top, bottom, right, left;
  double fovyrad;
  pausesim = 0; wiremesh = 0; doprofile = 1;
  glViewport(0, 0, (int)WWIDTH, (int)WHEIGHT);
  fovy = 60.0f;
  fovyrad = fovy * (M_PI / 180.0f);
  top = tanf((float)fovyrad * 0.5f) * 0.1f;
  bottom = -top;
  right = top * ((float)WWIDTH / (float)WHEIGHT);
  left = -right;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(left, right, bottom, top, 0.1f, 1000.0f); // TODO: Update frustum on window resize
  //glOrtho(0, WWIDTH, 0, WHEIGHT, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  initsfx();
}

void
render() {
  struct timespec frmst, frmend;
  GETNS(frmst); GETNS(frmend);
  if (doprofile) { GETNS(frmst); }
	glClearColor(0.39f, 0.58f, 0.92f, 1.0f);
	/* glClearColor(0.0f, 0.0f, 0.0f, 1.0f); */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
	lookat(3.0, 3.0, 3.0, 0.0, 0.0, -4.5, 0, 1, 0);
	glPushMatrix();
  glTranslatef(0.0f, 0.0f, -4.5f); 
  glBegin(GL_LINES);
		glColor3f(1,0,0);
		glVertex3f(0,0,-0.001f);
		glVertex3f(10,0,-0.002f);
		glColor3f(0,1,0);
		glVertex3f(0,0,-0.001f);
		glVertex3f(0,10,-0.002f);
		glColor3f(0,0,1);
		glVertex3f(0,0,-0.001f);
		glVertex3f(0,0,10);
  glEnd();
	glPopMatrix();
  glPushMatrix();
  glTranslatef(cx, cy, -5.0f); 
  glRotatef(a, 0.0f, 1.0f, 0.0f);
  drawm(tp);
  glPopMatrix();
  if (!pausesim) {
    a += 3.0f;
    cx += (dx*(float)mmx);
    if (cx > 3.8f-rad) { cx = 3.8f-rad; mmx *= -1; triggersfx(SFX_BOOM, 1); } if (cx < -3.8f-rad) { cx = -3.8f-rad; mmx *= -1; triggersfx(SFX_BOOM, 1); }
    cy += (dx*(float)mmy);
    if (cy > 2.8f-rad) { cy = 2.8f-rad; mmy *= -1; triggersfx(SFX_BOOM, 1); } if (cy < -2.8f-rad) { cy = -2.8f - rad; mmy *= -1; triggersfx(SFX_BOOM, 1); }
  }
  if (doprofile) {
    GETNS(frmend);
    fprintf(stderr, "\rFPS: %.2f, FT: %lld us", (1000000000.0 /((double)(elapsedr))), DIFFNS(frmst, frmend)/1000);
    fflush(stderr);
  }
}

void
gkill() {
  fprintf(stderr, "\rDone.                    \n");
  killsfx();
  killmesh(tp);
}
