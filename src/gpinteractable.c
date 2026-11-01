#include <stdio.h>
#include <stdlib.h>

#include "gpplayer.h"
#include "gpinteractable.h"

static Interactable ntrcts[MAXINTERACTABLES];
static int ntrctcnt;
static int activeidx = -1;

int
ntrctisactive() {
  return activeidx >= 0;
}

float
ntrctprogress() {
  if (activeidx < 0) { return 0.0f; }
  return ntrcts[activeidx].tickaccum / ntrcts[activeidx].tickinterval;
}

void
regntrct(Visent *ntt, Interactflvr type, Playerstatistics stat, 
    unsigned xpgain, const char *label, const char *activelabel,
    float tickinterval) {
  Interactable *in;
  if (ntrctcnt >= MAXINTERACTABLES) {
    fprintf(stderr, "Couldn't register new interactable, maximum reached!\n");
    return;
  }
  in = &ntrcts[ntrctcnt++];
  in->ntt = ntt;
  in->type = type;
  in->reach = INTERACTREACH;
  in->stat = stat;
  in->xpgain = xpgain;
  in->label = label;
  in->activelabel = activelabel;
  in->tickinterval = tickinterval;
  in->tickaccum = 0.0f;
  in->channeling = 0;
}

static int
inrange(const Interactable *in) {
  float dx, dz, distsq, rx, rz, reach;
  rx = in->ntt->x + in->ntt->shape.offx;
  rz = in->ntt->z + in->ntt->shape.offz;
  dx = P->x - rx; dz = P->z - rz;
  distsq = dx*dx + dz*dz;
  reach = in->ntt->shape.aabb.hx + in->reach;
  return distsq <= reach*reach;
}

static int
findinrange() {
  int i;
  for (i = 0; i < ntrctcnt; i++) {
    if (inrange(&ntrcts[i])) { return i; }
  }
  return -1;
}

void 
ntrcttoggle() {
  int i;
  if (activeidx >= 0) {
    ntrctcancel(); return;
  }
  i = findinrange();
  if (i >= 0) {
    ntrcts[i].channeling = 1;
    ntrcts[i].tickaccum = 0.0f;
    activeidx = i;
  }
}

void
ntrctcancel() {
  if (activeidx < 0) { return; }
  ntrcts[activeidx].channeling = 0;
  ntrcts[activeidx].tickaccum = 0.0f;
  activeidx = -1;
}

int
ntrctprompt(Visent **outntt, const char **outlabel) {
  int i;
  if (activeidx >= 0) {
    *outntt = ntrcts[activeidx].ntt;
    *outlabel = ntrcts[activeidx].activelabel;
    return 1;
  }
  i = findinrange();
  if (i < 0) { return 0; }
  *outntt = ntrcts[i].ntt;
  *outlabel = ntrcts[i].label;
  return 1;
}

void
ntrctupdate(double dt) {
  Interactable *in;
  if (activeidx < 0) {
    return;
  }
  in = &ntrcts[activeidx];
  if (!inrange(in)) {
    ntrctcancel();
    return;
  }
  in->tickaccum += (float)dt;
  if (in->tickaccum >= in->tickinterval) {
    in->tickaccum -= in->tickinterval;
    if ((float)rand() / (float)RAND_MAX < INTERACTCHANCE) {
      playergivexp(in->stat, in->xpgain);
    }
  }
}
