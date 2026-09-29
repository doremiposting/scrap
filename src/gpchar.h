#ifndef GPCHAR_H
#define GPCHAR_H

#include "gfxobj.h"
#include "gpcoll.h"

typedef struct Visibleentity {
  Mesh *m;
  float x, y, z;
  float pitch, /* up-down */ yaw, /* lateral side-to-side */ roll; /* circular rotation */
  Collshape shape;
} Visent;

#define MAX_STATICS 64
extern Visent *staticentities[MAX_STATICS];
extern int staticnttcnt;

void staticsinit();
Visent *newntt(const char *meshfp);
Visent *newnttfrommesh(Mesh *m);
void killntt(Visent *v);
void nttmove2(Visent *v, float x, float y, float z);
void nttmovealong(Visent *v, float dx, float dy, float dz); 
void nttrot2(Visent *v, float p, float y, float r);
void nttrotalong(Visent *v, float dp, float dy, float dr);

int regstatic(Visent *v);
void unregstatic(Visent *v);

#endif /* GPCHAR_H */
