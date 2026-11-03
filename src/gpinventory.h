#ifndef GPINVENTORY_H
#define GPINVENTORY_H

#define ITEMNAMELEN 48
#define ITEMPATHLEN 64

#define ITEMPACKBITS 8
#define ITEMLOCALBITS 24
#define ITEMLOCALMASK ((1u << ITEMLOCALBITS) - 1)
#define MAXPACKID ((1u << ITEMPACKBITS) - 1)
#define MAKEITEMID(packid, localid) \
  (((unsigned)(packid) << ITEMLOCALBITS) | ((unsigned)(localid) & ITEMLOCALMASK))
#define ITEMID_PACK(id) ((id) >> ITEMLOCALBITS)
#define ITEMID_LOCAL(id) ((id) & ITEMLOCALMASK)

/* XXX: Reserve (0,0) for "no item" */
#define ITEMID_NONE 0u
#define PACKID_BASE 0u

#define MAXITEMS 256
#define MAXINVSLOTS 28

typedef struct {
  unsigned int id;
  char name[ITEMNAMELEN];
  char iconfp[ITEMPATHLEN];
  char modelfp[ITEMPATHLEN];
} Item;

typedef struct {
  unsigned int itemid;
  unsigned int quantity;
} Inventoryslot;

int itemloadtable(const char *fp, unsigned int packid);
int itemidfromname(const char *name, unsigned int *outid);
const Item *finditem(unsigned int id);
Inventoryslot * buildupinventory();
int invadditem(Inventoryslot *inv, unsigned int itemid, unsigned int qty);

#endif /* GPINVENTORY_H */
