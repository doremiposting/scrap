#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "gptable.h"
#include "gfxtex.h"
#include "gpinventory.h"

typedef struct {
  unsigned int localid;
  char name[ITEMNAMELEN];
  char iconfp[ITEMPATHLEN];
  char modelfp[ITEMPATHLEN];
} Itemrow;

static const Fielddef itemfields[] = {
  { "id", FIELD_UINT, offsetof(Itemrow, localid), 0 },
  { "name", FIELD_STR, offsetof(Itemrow, name), ITEMNAMELEN },
  { "iconfp", FIELD_STR, offsetof(Itemrow, iconfp), ITEMPATHLEN },
  { "modelfp", FIELD_STR, offsetof(Itemrow, modelfp), ITEMPATHLEN },
};

static Itemrow rows[MAXITEMS];
static Item items[MAXITEMS];
static int itemcnt;
static int invopen;

int
itemloadtable(const char *fp, unsigned int packid) {
  int n, i, j;
  if (packid > MAXPACKID) {
    fprintf(stderr, "%s: pack id %u exceeds max %u\n", fp, packid, MAXPACKID);
    return 0;
  }
  n = loadtable(fp, itemfields, FIELDCOUNT(itemfields), rows, sizeof(Itemrow), MAXITEMS);
  if (n < 0) { return 0; }
  for (i = 0; i < n; i++) {
    if (rows[i].localid == 0 || rows[i].localid > ITEMLOCALMASK) {
      fprintf(stderr, "%s: item '%s' has invalid id %u (must be 1 thru %u)\n",
          fp, rows[i].name, rows[i].localid, ITEMLOCALMASK);
      return 0;
    }
    for (j = 0; j < i; j++) {
      if (rows[j].localid == rows[i].localid) {
        fprintf(stderr, "%s: duplicate item id %u ( '%s' and '%s')\n",
            fp, rows[i].localid, rows[j].name, rows[i].name);
        return 0;
      }
    }
  }
  if (itemcnt + n > MAXITEMS) {
    fprintf(stderr, "%s: too many items loaded (max %d)\n", fp, MAXITEMS);
    return 0;
  }
  for (i = 0; i < n; i++) {
    items[itemcnt].id = MAKEITEMID(packid, rows[i].localid);
    strcpy(items[itemcnt].name, rows[i].name);
    strcpy(items[itemcnt].iconfp, rows[i].iconfp);
    strcpy(items[itemcnt].modelfp, rows[i].modelfp);
    if (!loadtexture(items[itemcnt].iconfp, &items[itemcnt].icontex)) {
      fprintf(stderr, "%s: failed to load icon for item '%s'\n",
          fp, items[itemcnt].name);
      return 0;
    }
    itemcnt++;
  }
  return 1;
}

int
itemidfromname(const char *name, unsigned int *outid) {
  int i;
  for (i = 0; i < itemcnt; i++) {
    if (!strcmp(items[i].name, name)) {
      *outid = items[i].id;
      return 1;
    }
  }
  return 0;
}

const
Item *
finditem(unsigned int id) {
  int i;
  for (i = 0; i < itemcnt; i++) {
    if (items[i].id == id) { return &items[i]; }
  }
  return NULL;
}

Inventoryslot *
buildupinventory() {
  Inventoryslot *inv;
  int i;
  inv = calloc(MAXINVSLOTS, sizeof(Inventoryslot));
  for (i = 0; i < MAXINVSLOTS; i++) { inv[i].itemid = ITEMID_NONE; }
  return inv;
}

int
invadditem(Inventoryslot *inv, unsigned int itemid, unsigned int qty) {
  int i, empty;
  if (!finditem(itemid)) { return 0; }
  empty = -1;
  for (i = 0; i < MAXINVSLOTS; i++) {
    if (inv[i].itemid == itemid) {
      inv[i].quantity += qty;
      return 1;
    }
    if (empty < 0 && inv[i].itemid == ITEMID_NONE) { empty = i; }
  }
  if (empty < 0) { return 0; } /* full */
  inv[empty].itemid = itemid;
  inv[empty].quantity = qty;
  return 1;
}

void
invtoggle() {
  invopen = !invopen;
}

int
invisopen() {
  return invopen;
}
