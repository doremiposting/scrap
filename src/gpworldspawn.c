#include <stdio.h>

#include "gpinventory.h"
#include "gpworldspawn.h"

static Worlditem worlditems[MAXWORLDITEMS];
static int worlditemcount;

Visent *
spawnworlditem(unsigned int itemid, unsigned int quantity,
    float x, float y, float z) {
  const Item *it;
  Visent *ntt;
  if (worlditemcount >= MAXWORLDITEMS) {
    fprintf(stderr, "spawnworlditem: MAXWORLDItMS exceeded\n");
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
