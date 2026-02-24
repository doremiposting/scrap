#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <mpg123.h>
//#include <pthread.h>
#include <time.h>

#include "main.h"
#include "sfxcore.h"

int playsfx(long long elapsed);

static void *
sfxloop(void *arg) {
  UNUSED(arg);
  return NULL;
}

Soundfx *
newsnd() {
  return NULL;
}

int
initsfx() {
  return 1;
}

void
triggersfx(SfxID id, int cut) {
  UNUSED(id); UNUSED(cut);
}

int
playsfx(long long elapsed) {
  UNUSED(elapsed);
  return 1;
}

void
killsfx() {
}
