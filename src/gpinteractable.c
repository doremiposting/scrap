#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "gpplayer.h"
#include "gptable.h"
#include "gpinventory.h"
#include "gpinteractable.h"

static Interactable ntrcts[MAXINTERACTABLES];
static int ntrctcnt;
static int activeidx = -1;

static const char *interactnames[INTERACT_END] = {
#define X(id, nm) nm,
  INTERACT_TYPELIST
#undef X
};
static const Fielddef interactdeffields[] = {
  { "name", FIELD_STR, offsetof(Interactdef, name), DEFNAMELEN },
  { "statname", FIELD_STR, offsetof(Interactdef, statname), DEFNAMELEN },
  { "xpgain", FIELD_UINT, offsetof(Interactdef, xpgain), 0 },
  { "tickinterval", FIELD_FLOAT, offsetof(Interactdef, tickinterval), 0 },
  { "reach", FIELD_FLOAT, offsetof(Interactdef, reach), 0 },
  { "label", FIELD_STR, offsetof(Interactdef, label), DEFLABELLEN },
  { "activelabel", FIELD_STR, offsetof(Interactdef, activelabel), DEFLABELLEN },
  /* TODO: We can get multiple dropped items by iterating over a comma-separated
   * list with strtok, right? */
  { "dropitem", FIELD_STR, offsetof(Interactdef, dropitemname), DEFNAMELEN },
};
static Interactdef interactdefs[INTERACT_END];
static int ndefs;

static const
Interactdef *
finddef(Interactflvr type) {
  int i;
  for (i = 0; i < ndefs; i++) {
    if (!strcmp(interactdefs[i].name, interactnames[type])) {
      return &interactdefs[i];
    }
  }
  return NULL;
}

int
loadinteractdefs(const char *fn) {
  int i, statid;
  ndefs = loadtable(fn, interactdeffields, FIELDCOUNT(interactdeffields),
      interactdefs, sizeof(Interactdef), INTERACT_END);
  if (ndefs < 0) { ndefs = 0; return 0; }
  for (i = 0; i < ndefs; i++) {
    statid = statfromname(interactdefs[i].statname);
    if (statid < 0) {
      fprintf(stderr, "%s: unknown stat '%s' for '%s'\n",
          fn, interactdefs[i].statname, interactdefs[i].name);
      ndefs = 0;
      return 0;
    }
    interactdefs[i].stat = (Playerstatistics)statid;
    if (!strcmp(interactdefs[i].dropitemname, "none")) {
      interactdefs[i].dropitemid = ITEMID_NONE;
    } else if (!itemidfromname(interactdefs[i].dropitemname, &interactdefs[i].dropitemid)) {
      fprintf(stderr, "%s: unknown drop item '%s' for '%s'\n",
          fn, interactdefs[i].dropitemname, interactdefs[i].name);
      ndefs = 0;
      return 0;
    }
  }
  for (i = 0; i < INTERACT_END; i++) {
    if (!finddef((Interactflvr)i)) {
      fprintf(stderr, "%s: missing definition for interactable '%s'\n",
          fn, interactnames[i]);
      ndefs = 0;
      return 0;
    }
  }
  return 1;
}

int
ntrctisactive() {
  return activeidx >= 0;
}

float
ntrctprogress() {
  if (activeidx < 0) { return 0.0f; }
  return ntrcts[activeidx].tickaccum / ntrcts[activeidx].def->tickinterval;
}

void
regntrct(Visent *ntt, Interactflvr type) {
  Interactable *in;
  const Interactdef *def;
  if (ntrctcnt >= MAXINTERACTABLES) {
    fprintf(stderr, "Couldn't register new interactable, maximum reached!\n");
    return;
  }
  def = finddef(type);
  if (!def) {
    fprintf(stderr, "regntrct: no definition loaded for interactable type %d\n",
        type);
  }
  in = &ntrcts[ntrctcnt++];
  in->ntt = ntt;
  in->type = type;
  in->def = def;
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
  reach = in->ntt->shape.aabb.hx + in->def->reach;
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
    *outlabel = ntrcts[activeidx].def->activelabel;
    return 1;
  }
  i = findinrange();
  if (i < 0) { return 0; }
  *outntt = ntrcts[i].ntt;
  *outlabel = ntrcts[i].def->label;
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
  if (in->tickaccum >= in->def->tickinterval) {
    in->tickaccum -= in->def->tickinterval;
    if ((float)rand() / (float)RAND_MAX < INTERACTCHANCE) {
      playergivexp(in->def->stat, in->def->xpgain);
      if (in->def->dropitemid != ITEMID_NONE) {
        invadditem(P->inv, in->def->dropitemid, 1);
      }
    }
  }
}
