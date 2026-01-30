#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <mpg123.h>

#include "sfxpa.h"

pa_sample_spec pass;
pa_simple *ps;
mpg123_handle *mh;
unsigned char *buffer;
size_t buffersz;
size_t done;
int chnls, enc;
long rate;
int error;

void
initsfx() {
  mpg123_init();
  mh = mpg123_new(NULL, &error);
  mpg123_open(mh, "assets/vine boom.mp3");
  mpg123_getformat(mh, &rate, &chnls, &enc);
  mpg123_format_none(mh);
  mpg123_format(mh, rate, chnls, MPG123_ENC_SIGNED_16);
  buffersz = mpg123_outblock(mh);
  buffer = malloc(buffersz);
  pass.format = PA_SAMPLE_S16LE; /* TODO: 32-bits?? */
  pass.rate = rate;
  pass.channels = chnls;
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
  /* XXX: There's a huge stall right here and you can't play the same sound twice... */
  while ((error = mpg123_read(mh, buffer, buffersz, &done)) == MPG123_OK) {
    if (pa_simple_write(ps, buffer, done, &error) < 0) {
      fprintf(stderr, "pa_simple_write failure!  %s\n", pa_strerror(error));
      return 0;
    }
  }
}

void
killsfx() {
  pa_simple_drain(ps, &error);
  pa_simple_free(ps);

  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();

  free(buffer);
}
