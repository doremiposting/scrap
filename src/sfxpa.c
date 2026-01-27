#include <stdio.h>
#include <stdlib.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#include "sfxpa.h"

pa_sample_spec pass;
pa_simple *ps;
int error;

void
initsfx() {
  pass.format = PA_SAMPLE_S16LE; /* TODO: 32-bits?? */
  pass.rate = 44100;
  pass.channels = 2;
  ps = pa_simple_new(
      NULL,
      "Scrap",
      PA_STREAM_PLAYBACK,
      NULL,
      "playback",
      &pass, NULL, NULL,
      &error
      );
  if (!ps) {
    fprintf(stderr, "Failed to initialize pulseaudio! %s\n", pa_strerror(error));
    return;
  }
}

int
playsfx(const char *fn) {

}

void
killsfx() {

}
