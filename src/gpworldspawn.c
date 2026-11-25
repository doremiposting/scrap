#include <stdio.h>
#include <string.h>

#include "gpplayer.h"
#include "gpinventory.h"
#include "gpworldspawn.h"

static Worlditem worlditems[MAXWORLDITEMS];
static int worlditemcount;

static int
worlditemreach(const Worlditem *w, float *distsq) {
  float dx, dz;
  dx = P->x - w->ntt->x;
  dz = P->z - w->ntt->z;
  *distsq = dx*dx + dz*dz;
  return *distsq <= (PICKUPREACH * PICKUPREACH);
}

static int
findnearest() {
  int i, best;
  float bestdsq, dsq;
  best  = -1; bestdsq = 0.0f;
  for (i = 0 ; i < worlditemcount; i++) {
    if (!worlditemreach(&worlditems[i], &dsq)) { continue; }
    if (best < 0 || dsq < bestdsq) { best = i; bestdsq = dsq; }
  }
  return best;
}

Visent *
spawnworlditem(unsigned int itemid, unsigned int quantity,
    float x, float y, float z) {
  const Item *it;
  Visent *ntt;
  if (worlditemcount >= MAXWORLDITEMS) {
    fprintf(stderr, "spawnworlditem: MAXWORLDITEMS exceeded\n");
    return NULL;
  }
  it = finditem(itemid);
  if (!it) {
    fprintf(stderr, "spawnworlditem: unknown item id %u\n", itemid);
    return NULL;
  }
  /* XXX: We'll want to start caching models later on as we scale this up. */
  ntt = newnttfrommesh(loadobj(it->modelfp));
  nttmove2(ntt, x, y, z);
  ntt->shape.kind = SHAPE_NONE;
  worlditems[worlditemcount].ntt = ntt;
  worlditems[worlditemcount].itemid = itemid;
  worlditems[worlditemcount].quantity = quantity;
  worlditemcount++;
  return ntt;
}

int
worlditemcnt() {
  return worlditemcount;
}

const 
Worlditem *
worlditemat(int idx) {
  if (idx < 0 || idx >= worlditemcount) { return NULL; }
  return &worlditems[idx];
}

void
despawnworlditem(int idx) {
  if (idx < 0 || idx >= worlditemcount) { return; }
  killntt(worlditems[idx].ntt);
  memmove(&worlditems[idx], &worlditems[idx+1],
      (size_t)(worlditemcount - idx - 1) * sizeof(Worlditem));
  worlditemcount--;
}

int
pickupnearest() {
  int idx;
  idx = findnearest();
  if (idx < 0) { return 0; }
  if (!invadditem(P->inv, worlditems[idx].itemid,
        worlditems[idx].quantity)) {
    /* Inventory full, abort to leacve on ground and not destroy item. */
    return 0;
  }
  despawnworlditem(idx);
  return 1;
}

int 
worlditemprompt(Visent **outntt, const char **outname,
    unsigned int *outqty) {
  int idx;
  const Item *it;
  idx = findnearest();
  if (idx < 0) { return 0; }
  it = finditem(worlditems[idx].itemid);
  if (!it) { return 0; }
  *outntt = worlditems[idx].ntt;
  *outname = it->name;
  *outqty = worlditems[idx].quantity;
  return 1;
}
