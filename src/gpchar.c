#include <stdio.h>
#include <stdlib.h>
#include "gfxobj.h"

#include "gpchar.h"

Visent *
newntt(const char *meshfp) {
  Visent *v;
  v = calloc(1, sizeof(Visent));
  v->m = loadobj(meshfp);
  v->x = v->y = v->z = 0;
  v->pitch = v->yaw = v->roll = 0;
  return v;
}

Visent *
newnttfrommesh(Mesh *m) {
  Visent *v;
  v = calloc(1, sizeof(Visent));
  v->m = m;
  v->x = v->y = v->z = 0;
  v->pitch = v->yaw = v->roll = 0;
  return v;
}

void
killntt(Visent *v) {
  if (v->m) { killmesh(v->m); }
  if (v) { free(v); }
}

void
nttmove2(Visent *v, float x, float y, float z) {
  v->x = x; v->y = y; v->z = z;
}

void
nttmovealong(Visent *v, float dx, float dy, float dz) {
  v->x += dx; v->y += dy; v->z += dz;
}

void
nttrot2(Visent *v, float p, float y, float r) {
  v->pitch = p; v->yaw = y; v->roll = r;
}

void
nttrotalong(Visent *v, float dp, float dy, float dr) {
  v->pitch += dp; v->yaw += dy; v->roll += dr;
}
