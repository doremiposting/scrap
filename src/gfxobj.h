#ifndef GFXOBJ_H
#define GFXOBJ_H

#include "gfxmesh.h"

Mesh *loadobj(const char *fn);
float meshfootoffset(Mesh *m);

#endif /* GFXOBJ_H */
