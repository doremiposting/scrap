#include <stdio.h>
#include <stdlib.h>
#include "gfxobj.h"

#include "gpchar.h"

Visent *
newntt(const char *meshfp) {
  Visent *v;
  v = calloc(1, sizeof(Visent));
  v->m = loadobj(meshfp);
  v->posx = v->posy = v->posz = 0;
  v->rotp = v->roty = v->rotr = 0;
  return v;
}

void
killntt(Visent *v) {
  if (v->m) { killmesh(v->m); }
  if (v) { free(v); }
}

void
nttmove2(Visent *v, float x, float y, float z) {
  v->posx = x; v->posy = y; v->posz = z;
}

void
nttmovealong(Visent *v, float dx, float dy, float dz) {
  v->posx += dx; v->posy += dy; v->posz += dz;
}

void
nttrot2(Visent *v, float p, float y, float r) {
  v->rotp = p; v->roty = y; v->rotr = r;
}

void
nttrotalong(Visent *v, float dp, float dy, float dr) {
  v->rotp += dp; v->roty += dy; v->rotr += dr;
}
