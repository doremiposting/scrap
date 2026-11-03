#ifndef GPINTERACTABLE_H
#define GPINTERACTABLE_H

#include "gfxobj.h"
#include "gfxprim.h"
#include "gpplayer.h"
#include "gpinventory.h"

/* TODO: XXX: If we ever have expansion packs, we need to handle this separately.
 * "new category support probably needs those categories to stop being
 * compile-time enums at all (string- or integer-ID-resolved purely from data,
 * the alternative I mentioned but didn’t pick in part (a))" */
/* XXX: New interactables get defined here and exactly only here */
#define INTERACT_TYPELIST \
  X(INTERACT_TREE, "tree")

typedef enum {
#define X(id, nm) id,
  INTERACT_TYPELIST
#undef X
  INTERACT_END
} Interactflvr;

#define MAXINTERACTABLES 32
#define INTERACTCHANCE 0.35

#define DEFNAMELEN 16
#define DEFLABELLEN 32

typedef struct {
  char name[DEFNAMELEN];
  char statname[DEFNAMELEN];
  unsigned xpgain;
  float tickinterval;
  float reach;
  char label[DEFLABELLEN];
  char activelabel[DEFLABELLEN];
  Playerstatistics stat;
  char dropitemname[DEFNAMELEN];
  unsigned dropitemid;
} Interactdef;
typedef struct {
  Visent *ntt;
  Interactflvr type;
  float tickaccum;
  int channeling;
  const Interactdef *def;
} Interactable;

int ntrctisactive();
float ntrctprogress();
int loadinteractdefs(const char *fn);
void regntrct(Visent *ntt, Interactflvr type);
void ntrcttoggle();
void ntrctcancel();
void ntrctupdate(double dt);
int ntrctprompt(Visent **outntt, const char **outlabel);

#endif /* GPINTERACTABLE_H */
