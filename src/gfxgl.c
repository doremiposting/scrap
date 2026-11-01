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
#include "gpinteractable.h"

Mesh *tp;
Visent *box;
Visent *tree;
int wiremesh;
int animate;
int showcoll;
int scrw, scrh;
TTFAtlas hudfont;

#define GIZMOPX 96
#define GIZMOMGN 10
TEMP
#define TREEXP 45
#define PROMPTMARGIN 0.3f
#define BARW 220.0f
#define BARH 14.0f
#define BARBOTTOMMARGIN 60.0f

typedef enum { BAR_FILL, BAR_DRAIN } Bardir;

static int promptvisible;
static float promptsx, promptsy;
static const char *promptlabel;

#define updateclamp(x) \
  do { \
    if (x > (float)(2*M_PI)) { x -= (float)(2*M_PI); } \
    else if (x < (float)(-2*M_PI)) { x += (float)(2*M_PI); } \
  } while (0)
void
update(int state, int ox, int nx, int oy, int ny) {
  int dx, dy;
  dx = ox-nx; dy = ny-oy;
  switch (state) {
    case PAN:
      P->view->x -= (float)dx / 100.0f; P->view->y -= (float)dy / 100.0f;
      break;
    case ROTATE:
      P->view->pitch += ((float)dy * 180.0f) / 50000.0f;
      P->view->yaw -= ((float)dx * 180.0f) / 50000.0f;
      updateclamp(P->view->pitch); updateclamp(P->view->yaw);
      break;
    case ZOOM:
      P->view->z -= (float)(dx + dy) / 1000.0f;
      break;
  }
}

static int
worldtoscreen(float wx, float wy, float wz, float *sx, float *sy) {
  GLdouble mv[16], proj[16];
  GLint vp[4];
  GLdouble wsx, wsy, wsz;
  glGetDoublev(GL_MODELVIEW_MATRIX, mv);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);
  glGetIntegerv(GL_VIEWPORT, vp);
  if (!gluProject((GLdouble)wx, (GLdouble)wy, (GLdouble)wz, mv, proj, vp,
        &wsx, &wsy, &wsz)) {
    return 0;
  }
  /* wsz outside [0,1] means means behind the near plane or past the far plane */
  if (wsz < 0.0 || wsz > 1.0) { return 0; }
  *sx = (float)wsx;
  *sy = (float)scrh - (float)wsy;
  return 1;
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
  tree = newnttfrommesh(loadobj("assets/tree.obj"));
  nttmove2(tree, -5.0f, 0.0f, -2.0f);
  tree->shape.kind = SHAPE_AABB;
  tree->shape.aabb.hx = 0.42f;
  tree->shape.aabb.hy = 2.25f;
  tree->shape.aabb.hz = 0.42f;
  tree->shape.offx = 0.0f;
  tree->shape.offy = 2.0f;
  tree->shape.offz = 0.0f;
  regstatic(tree);
  regntrct(tree, INTERACT_TREE, PLEVEL_BOTANY, TREEXP, "Chop tree", "Chopping...", 1.0f);
  if (ttfbuildatlas(&hudfont, "assets/lib.ttf", 24.0f)) {
    fprintf(stderr, "glinit: filed to build HUD font atlas\n");
  }
}

void
glkill() {
  terrteardown();
  ttffreeatlas(&hudfont);
  unregstatic(tree);
  killntt(tree);
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
  glDrawElements(GL_TRIANGLES, (int)m->idxc, GL_UNSIGNED_INT, m->idx);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void
drawterrain() {
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(TV), tvfield);
  glDrawElements(GL_TRIANGLES, (int)triiacnt, GL_UNSIGNED_INT, triia);
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

static void
drawbar(float x, float y, float w, float h, float frac, Bardir dir) {
  float fillw;
  if (frac < 0.0f) { frac = 0.0f; }
  if (frac > 1.0f) { frac = 1.0f; }
  fillw = (dir == BAR_DRAIN) ? w * (1.0f - frac) : w * frac;
  glColor3f(0.1f, 0.1f, 0.1f);
  glRectf(x, y, x+w, y+h);
  glColor3f(0.9f, 0.8f, 0.2f);
  glRectf(x, y, x+fillw, y+h);
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINE_LOOP);
    glVertex2f(x,y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
  glEnd();
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
drawgizmo() {
  GLint vp[4];
  int gx, gy;
  glGetIntegerv(GL_VIEWPORT, vp);
  gx = scrw - GIZMOPX - GIZMOMGN;
  gy = scrh - GIZMOPX - GIZMOMGN;
  glViewport(gx, gy, GIZMOPX, GIZMOPX);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-1.2, 1.2, -1.2, 1.2, -2.0, 2.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glRotatef(P->pitch * (float)(180/M_PI), 1.0f, 0.0f, 0.0f);
  glRotatef(P->yaw * (float)(180/M_PI), 0.0f, 1.0f, 0.0f);
  glRotatef(P->roll * (float)(180/M_PI), 0.0f, 0.0f, 1.0f);
  glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,0); glVertex3f(1,0,0);
    glColor3f(0,1,0);
    glVertex3f(0,0,0); glVertex3f(0,1,0);
    glColor3f(0,0,1);
    glVertex3f(0,0,0); glVertex3f(0,0,1);
  glEnd();
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glViewport(vp[0], vp[1], vp[2], vp[3]);
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
  snprintf(buf, sizeof(buf), "botany: Lv%u, Xp: %u", P->Cl[PLEVEL_BOTANY], P->S[PLEVEL_BOTANY]);
  drawtxtwshadow(&hudfont, 10, 30+hudfont.ascent, buf);
  if (promptvisible) {
    if (!(ntrctisactive())) {
      snprintf(buf, sizeof(buf), "[E] %s", promptlabel);
    } else {
      snprintf(buf, sizeof(buf), "%s", promptlabel);
    }
    drawtxtwshadow(&hudfont, promptsx, promptsy, buf);
  }
  if (ntrctisactive()) {
    drawbar((float)scrw/2.0f - BARW/2.0f, (float)scrh - BARBOTTOMMARGIN,
        BARW, BARH, ntrctprogress(), BAR_FILL);
  }
  drawgizmo();
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void
render() {
  Visent *promptntt;
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
  glRotatef(P->view->pitch * (float)(180/M_PI), 1.0f, 0.0f, 0.0f);
  glRotatef(P->view->yaw * (float)(180/M_PI), 0.0f, 1.0f, 0.0f);
  glTranslatef(-P->view->x, -P->view->y, -P->view->z);
  promptvisible = ntrctprompt(&promptntt, &promptlabel);
  if (promptvisible) {
    promptvisible = worldtoscreen(promptntt->x,
        promptntt->y + promptntt->shape.offy + promptntt->shape.aabb.hy + PROMPTMARGIN,
        promptntt->z, &promptsx, &promptsy);
  }
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
  glRotatef(P->pitch * (float)(180/M_PI), 1.0f, 0.0f, 0.0f);
  glRotatef(P->yaw * (float)(180/M_PI), 0.0f, 1.0f, 0.0f);
  glRotatef(P->roll * (float)(180/M_PI), 0.0f, 0.0f, 1.0f);
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
  glPushMatrix();
  glColor3f(0.45f, 0.3f, 0.15f);
  glTranslatef(tree->x, tree->y, tree->z);
  drawmesh(tree->m);
  if (wiremesh) {
    glColor3f(0,0,0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawmesh(tree->m);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  glPopMatrix();
  /* XXX: Front face indicator, remove me later. */
  glPushMatrix();
  glTranslatef(P->x, P->y, P->z);
  glRotatef(P->yaw * (float)(180/M_PI), 0.0f, 1.0f, 0.0f);
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
