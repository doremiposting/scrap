#include <math.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "gfxgl.h"
#include "gfxobj.h"
#include "gfxprim.h"
#include "gfxterrain.h"
#include "gpchar.h"
#include "gpplayer.h"

Mesh *tp;
Visent *box;
int wiremesh;
int animate;
int showcoll;
int scrw, scrh;
TTFAtlas hudfont;

#define updateclamp(x) do {x = x > (2*M_PI) ? x - (2*M_PI) : x < (-2*M_PI) ? x += (-2*M_PI) : x;} while (0)
void
update(int state, int ox, int nx, int oy, int ny) {
  int dx, dy;
  dx = ox-nx; dy = ny-oy;
  switch (state) {
    case PAN:
      P->view->x -= dx / 100.0f; P->view->y -= dy / 100.0f;
      break;
    case ROTATE:
      P->view->pitch += (dy * 180.0f) / 50000.0f;
      P->view->yaw -= (dx * 180.0f) / 50000.0f;
      updateclamp(P->view->pitch); updateclamp(P->view->yaw);
      break;
    case ZOOM:
      P->view->z -= (dx + dy) / 1000.0f;
      break;
  }
}

void
glinit() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  tp = loadobj("assets/teapot.obj");
  playerbuildup("assets/pill.obj");
  playermove2(3, 3, 3);
  terrbuildup();
  box = newnttfrommesh(buildabox(1.0f, 1.0f, 1.0f));
  nttmove2(box, 2.0, 1.3f, 0.0f);
  box->shape.kind = SHAPE_AABB;
  box->shape.aabb.hx = 1.0f;
  box->shape.aabb.hy = 1.0f;
  box->shape.aabb.hz = 1.0f;
  regstatic(box);
  if (ttfbuildatlas(&hudfont, "assets/lib.ttf", 24.0f)) {
    fprintf(stderr, "glinit: filed to build HUD font atlas\n");
  }
}

void
glkill() {
  terrteardown();
  ttffreeatlas(&hudfont);
  unregstatic(box);
  killntt(box);
  playerteardown(P->id);
  killmesh(tp);
}

void
glreshape(int width, int height) {
  scrw = width; scrh = height;
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (float)width/(float)height, 0.001, 100.0);
  glMatrixMode(GL_MODELVIEW);
}

void
drawmesh(const Mesh *m) {
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), m->v);
  glDrawElements(GL_TRIANGLES, m->idxc, GL_UNSIGNED_INT, m->idx);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void
drawterrain() {
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(TV), tvfield);
  glDrawElements(GL_TRIANGLES, triiacnt, GL_UNSIGNED_INT, triia);
  glDisableClientState(GL_VERTEX_ARRAY);
}

static void
drawaabb(float cx, float cy, float cz, float hx, float hy, float hz) {
  float x0, x1, y0, y1, z0, z1;
  x0 = cx-hx; x1 = cx+hx;
  y0 = cy-hy; y1 = cy+hy;
  z0 = cz-hz; z1 = cz+hz;
  glBegin(GL_LINE_LOOP);
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
  glEnd();
  glBegin(GL_LINE_LOOP);
    glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
  glEnd();
  glBegin(GL_LINES);
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1);
  glEnd();
}

static void
drawcollview() {
  int i;
  float fx, fy, fz;
  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_DEPTH_TEST);
  if (P->shape.kind == SHAPE_AABB) {
    glColor3f(0.0f, 1.0f, 0.0f);
    drawaabb(P->x + P->shape.offx, P->y + P->shape.offy,
        P->z + P->shape.offz,
        P->shape.aabb.hx, P->shape.aabb.hy, P->shape.aabb.hz);
  }
  glColor3f(1.0f, 1.0f, 0.0f);
  for (i = 0; i < staticnttcnt; i++) {
    if (staticentities[i]->shape.kind != SHAPE_AABB) { continue; }
    drawaabb(staticentities[i]->x + staticentities[i]->shape.offx,
      staticentities[i]->y + staticentities[i]->shape.offy,
      staticentities[i]->z + staticentities[i]->shape.offz,
      staticentities[i]->shape.aabb.hx,
      staticentities[i]->shape.aabb.hy,
      staticentities[i]->shape.aabb.hz);
  }
  fx = P->x; fy = P->y - P->foot; fz = P->z;
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINES);
    glVertex3f(fx-0.3f, fy, fz); glVertex3f(fx+0.3f, fy, fz);
    glVertex3f(fx, fy-0.3f, fz); glVertex3f(fx, fy+0.3f, fz);
    glVertex3f(fx, fy, fz-0.3f); glVertex3f(fx, fy, fz+0.3f);
  glEnd();
  glPopAttrib();
}

static inline void
drawtxtwshadow(const TTFAtlas *a, float x, float y, const char *s) {
  glColor3f(0.0, 0.0f, 0.0f);
  ttfdrawtxt(a, x, y, s);
  glColor3f(1.0f, 1.0f, 1.0f);
  ttfdrawtxt(a, x+1, y+1, s);
}

static inline void
colortxtwshadow(const TTFAtlas *a, float x, float y, const char *s,
    float r, float g, float b) {
  glColor3f(0.0, 0.0f, 0.0f);
  ttfdrawtxt(a, x, y, s);
  glColor3f(r, g, b);
  ttfdrawtxt(a, x+1, y+1, s);
}

static void
drawhud() {
  char buf[64];
  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_CULL_FACE);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, scrw, scrh, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  snprintf(buf, sizeof(buf), "collision view: %s", showcoll ? "ON" : "OFF");
  drawtxtwshadow(&hudfont, 10, 10+hudfont.ascent, buf);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void
render() {
  static float spin = 0.0f;
  static float tpdx = 0.05f;
  static float tpdy = 0.05f;
  static float tpcx = 0.0f;
  static float tpcy = 0.0f;
  static float dspin = 3.0f;
  static int tpmx = 1;
  static int tpmy = 1;
  static float tprad = 0.75f;
  glClearColor(0.39f, 0.58f, 0.92f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  /* XXX: This one's for managing scale a little */
  /* glTranslatef(0.0f, 0.0f, -12.0f); */
  glPushMatrix();
  glRotatef(P->view->pitch * (180/M_PI), 1.0f, 0.0f, 0.0f);
  glRotatef(P->view->yaw * (180/M_PI), 0.0f, 1.0f, 0.0f);
  glTranslatef(-P->view->x, -P->view->y, -P->view->z);
  glColor3f(0, 1, 1);
  drawterrain();
  if (wiremesh) {
    glColor3f(0,0,0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawterrain();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  glPushMatrix();
  glColor3f(0.85f,1,0.85f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glTranslatef(P->x, P->y, P->z);
  glRotatef(P->pitch * (180/M_PI), 1.0f, 0.0f, 0.0f);
  glRotatef(P->yaw * (180/M_PI), 0.0f, 1.0f, 0.0f);
  glRotatef(P->roll * (180/M_PI), 0.0f, 0.0f, 1.0f);
  drawmesh(P->m);
  if (wiremesh) {
    glColor3f(0,0,0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawmesh(P->m);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  glPopMatrix();
  glPushMatrix();
  glColor3f(0.8f, 0.4f, 0.2f);
  glTranslatef(box->x, box->y, box->z);
  drawmesh(box->m);
  if (wiremesh) {
    glColor3f(0,0,0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawmesh(box->m);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  glPopMatrix();
  /* XXX: Front face indicator, remove me later. */
  glPushMatrix();
  glTranslatef(P->x, P->y, P->z);
  glRotatef(P->yaw * (180/M_PI), 0.0f, 1.0f, 0.0f);
  glDisable(GL_CULL_FACE);
  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_LINES);
    /* shaft */
    glVertex3f(0, 0, 0); glVertex3f(0, 0, 2.0f);
    /* arrowhead */
    glVertex3f(0, 0, 2.0f); glVertex3f(0.3f, 0, 1.6f);
    glVertex3f(0, 0, 2.0f); glVertex3f(-0.3f, 0., 1.6f);
  glEnd();
  glEnable(GL_CULL_FACE);
  glPopMatrix();
  glPushMatrix();
  glColor3f(0.95f, 1, 0.95f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glTranslatef(tpcx, tpcy, -5.0f);
  glRotatef(spin, 0.0f, 1.0f, 0.0f);
  drawmesh(tp);
  if (wiremesh) {
    glColor3f(0,0,0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawmesh(tp);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  glPopMatrix();
  glPushMatrix();
  glDisable(GL_CULL_FACE);
  glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,-0.001f); glVertex3f(10,0,-0.002f);
    glColor3f(0,1,0);
    glVertex3f(0,0,-0.001f); glVertex3f(0,10,-0.002f);
    glColor3f(0,0,1);
    glVertex3f(0,0,-0.001f); glVertex3f(0,0,10);
  glEnd();
  glEnable(GL_CULL_FACE);
  glPopMatrix();
  if (showcoll) { drawcollview(); }
  glPopMatrix();
  drawhud();
  if (animate) {
    spin += dspin;
    tpcx += (tpdx * (float)tpmx);
    if (tpcx > 3.8f-tprad) { tpcx = 3.8f-tprad; tpmx *= -1; } if (tpcx < -3.8f-tprad) { tpcx = -3.8f-tprad; tpmx *= -1; }
    tpcy += (tpdy * (float)tpmy);
    if (tpcy > 2.8f-tprad) { tpcy = 2.8f-tprad; tpmy *= -1; } if (tpcy < -2.8f-tprad) { tpcy = -2.8f-tprad; tpmy *= -1; }
  }
  glFlush();
}
