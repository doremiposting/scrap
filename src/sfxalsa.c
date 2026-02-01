#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <mpg123.h>

#include "sfxalsa.h"

Aout *ao;
mpg123_handle *mh;
unsigned char *buffer;
size_t buffersz, done;
snd_pcm_hw_params_t *hw;
int rc, chnls, enc, error;
long rate;
Soundfx *s;

#if 0
Soundfx
newsnd() {
  /* TODO: expand to initialize anything as pcm, require reuse of mh et. al. */
  return NULL;
}
#endif

int
initsfx() {
  unsigned char b[8196];
  unsigned char *a;
  int r;
  size_t cap;
  mpg123_init();
  mh = mpg123_new(NULL, &error);
  mpg123_open(mh, "assets/vine boom.mp3");
  mpg123_getformat(mh, &rate, &chnls, &enc);
  mpg123_format_none(mh);
  mpg123_format(mh, rate, chnls, MPG123_ENC_SIGNED_16);
  buffersz = mpg123_outblock(mh);
  buffer = malloc(buffersz);
  
  /* TODO: Extract this out to newsnd(), reuse mh as set up above. */
  s = calloc(1, sizeof(Soundfx));
  a = NULL; cap = 0;
  s->rate = (int)rate; /* TODO: Should the struct member be changed to long? */
  s->chnls = chnls;
  while(mpg123_read(mh, (unsigned char *)&a, 0, &done) != MPG123_DONE) {
    r = mpg123_read(mh, b, sizeof(b), &done);
    if (r == MPG123_OK && done > 0) {
      a = realloc(a, cap+done);
      memcpy(a + cap, b, done);
      cap += done;
    }
  }
  s->pcm = (short*)a;
  s->frames = (size_t)(cap / ((size_t)chnls * sizeof(short)));


  ao = calloc(1, sizeof(Aout));
  if (!ao) { return -1; }
  //rc = snd_pcm_open(&ao->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
  rc = snd_pcm_open(&ao->pcm, "pulse", SND_PCM_STREAM_PLAYBACK, 0);
  //rc = snd_pcm_open(&ao->pcm, "dmix", SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) { return rc; }
  snd_pcm_hw_params_malloc(&hw);
  snd_pcm_hw_params_any(ao->pcm, hw);
  snd_pcm_hw_params_set_access(ao->pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(ao->pcm, hw, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(ao->pcm, hw, (unsigned int)chnls);
  snd_pcm_hw_params_set_rate(ao->pcm, hw, (unsigned int)rate, 0);
  ao->frames = 1024;
  snd_pcm_hw_params_set_period_size(ao->pcm, hw, ao->frames, 0);
  rc = snd_pcm_hw_params(ao->pcm, hw);
  snd_pcm_hw_params_free(hw);
  snd_pcm_prepare(ao->pcm);
  ao->rate = (int)rate;
  ao->chnls = chnls;

  return rc;
}

int
playsfx(const char *fn) {
  snd_pcm_sframes_t frames;
  snd_pcm_prepare(ao->pcm);
  frames = snd_pcm_writei(ao->pcm, s->pcm, s->frames);
  if (frames < 0) { snd_pcm_recover(ao->pcm, (int)frames, 0); }
}

void
killsfx() {
  snd_pcm_drain(ao->pcm);
  snd_pcm_close(ao->pcm);

  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();

  free(s->pcm); free(s);
  free(buffer);
  free(ao);
}
