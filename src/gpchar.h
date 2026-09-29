#ifndef GPCHAR_H
#define GPCHAR_H

#include "gfxobj.h"

typedef struct Visibleentity {
  Mesh *m;
  float x, y, z;
  float pitch, /* up-down */ yaw, /* lateral side-to-side */ roll; /* circular rotation */
} Visent;

Visent *newntt(const char *meshfp);
Visent *newnttfrommesh(Mesh *m);
void killntt(Visent *v);
void nttmove2(Visent *v, float x, float y, float z);
void nttmovealong(Visent *v, float dx, float dy, float dz); 
void nttrot2(Visent *v, float p, float y, float r);
void nttrotalong(Visent *v, float dp, float dy, float dr);

#endif /* GPCHAR_H */
