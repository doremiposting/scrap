#ifndef GPWORLDSPAWN_H
#define GPWORLDSPAWN_H

#include "gfxobj.h"
#include "gfxprim.h"
#include "gpchar.h"

#define MAXWORLDITEMS 64
#define PICKUPREACH 1.5f
#define WORLDITEMPROMPTHEIGHT 0.5f

typedef struct {
  Visent *ntt;
  unsigned int itemid;
  unsigned int quantity;
} Worlditem;

Visent * spawnworlditem(unsigned int itemid, unsigned int quantity,
    float x, float y, float z);
int worlditemcnt();
const Worlditem * worlditemat(int idx);
void despawnworlditem(int idx);
int pickupnearest();
int worlditemprompt(Visent **outntt, const char **outname,
    unsigned int *outqty);

#endif /* GPWORLDSPAWN_H */
