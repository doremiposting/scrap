#ifndef GPINTERACTABLE_H
#define GPINTERACTABLE_H

#include "gfxobj.h"
#include "gfxprim.h"
#include "gpplayer.h"

typedef enum { INTERACT_TREE, INTERACT_END } Interactflvr;

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
