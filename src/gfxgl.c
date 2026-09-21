#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "gfxgl.h"
#include "gfxobj.h"
#include "gfxterrain.h"
#include "gpchar.h"
#include "gpplayer.h"

Mesh *tp;
int wiremesh;
int animate;

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
}

void
glkill() {
  terrteardown();
  playerteardown(P->id);
  killmesh(tp);
}

void
glreshape(int width, int height) {
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
  glPopMatrix();
  if (animate) {
    spin += dspin;
    tpcx += (tpdx * (float)tpmx);
    if (tpcx > 3.8f-tprad) { tpcx = 3.8f-tprad; tpmx *= -1; } if (tpcx < -3.8f-tprad) { tpcx = -3.8f-tprad; tpmx *= -1; }
    tpcy += (tpdy * (float)tpmy);
    if (tpcy > 2.8f-tprad) { tpcy = 2.8f-tprad; tpmy *= -1; } if (tpcy < -2.8f-tprad) { tpcy = -2.8f-tprad; tpmy *= -1; }
  }
  glFlush();
}
