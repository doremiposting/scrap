#ifndef GPINTERACTABLE_H
#define GPINTERACTABLE_H

#include "gfxobj.h"
#include "gfxprim.h"
#include "gpplayer.h"

typedef enum { INTERACT_TREE, INTERACT_END } Interactflvr;

#define INTERACTREACH 2.0f
#define MAXINTERACTABLES 32
#define INTERACTCHANCE 0.35

typedef struct {
  Visent *ntt;
  Interactflvr type;
  float reach;
  float tickinterval;
  float tickaccum;
  int channeling;
  Playerstatistics stat;
  unsigned xpgain;
  const char *label, *activelabel;
} Interactable;

int ntrctisactive();
float ntrctprogress();
void regntrct(Visent *ntt, Interactflvr type, Playerstatistics stat,
    unsigned xpgain, const char *label, const char *activelabel,
    float tickinterval);
void ntrcttoggle();
void ntrctcancel();
void ntrctupdate(double dt);
int ntrctprompt(Visent **outntt, const char **outlabel);

#endif /* GPINTERACTABLE_H */
