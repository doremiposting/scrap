#ifndef GPCHAR_H
#define GPCHAR_H

#include "gfxobj.h"

typedef struct Visibleentity {
  Mesh *m;
  float posx, posy, posz;
  float rotp, /* up-down */ roty, /* lateral side-to-side */ rotr; /* circular rotation */
} Visent;

Visent *newntt(const char *meshfp);
void killntt(Visent *v);
void nttmove2(Visent *v, float x, float y, float z);
void nttmovealong(Visent *v, float dx, float dy, float dz); 
void nttrot2(Visent *v, float p, float y, float r);
void nttrotalong(Visent *v, float dp, float dy, float dr);

#endif /* GPCHAR_H */
