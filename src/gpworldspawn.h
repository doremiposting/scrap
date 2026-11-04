#ifndef GPWORLDSPAWN_H
#define GPWORLDSPAWN_H

#include "gfxobj.h"
#include "gfxprim.h"
#include "gpchar.h"

#define MAXWORLDITEMS 64

typedef struct {
  Visent *ntt;
  unsigned int itemid;
  unsigned int quantity;
} Worlditem;

Visent * spawnworlditem(unsigned int itemid, unsigned int quantity,
    float x, float y, float z);
int worlditemcnt();
const Worlditem * worlditemat(int idx);

#endif /* GPWORLDSPAWN_H */
