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

int
initsfx() {
  mpg123_init();
  mh = mpg123_new(NULL, &error);
  mpg123_open(mh, "assets/vine boom.mp3");
  mpg123_getformat(mh, &rate, &chnls, &enc);
  mpg123_format_none(mh);
  mpg123_format(mh, rate, chnls, MPG123_ENC_SIGNED_16);
  buffersz = mpg123_outblock(mh);
  buffer = malloc(buffersz);

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
  snd_pcm_hw_params_set_channels(ao->pcm, hw, chnls);
  snd_pcm_hw_params_set_rate(ao->pcm, hw, rate, 0);
  ao->frames = 1024;
  snd_pcm_hw_params_set_period_size(ao->pcm, hw, ao->frames, 0);
  rc = snd_pcm_hw_params(ao->pcm, hw);
  snd_pcm_hw_params_free(hw);
  snd_pcm_prepare(ao->pcm);
  ao->rate = rate;
  ao->chnls = chnls;

  return rc;
}

int
playsfx(const char *fn) {
  snd_pcm_sframes_t frames, rc;
  while (mpg123_read(mh, buffer, buffersz, &done) == MPG123_OK) {
    frames = done / (ao->chnls * 2);
    rc = snd_pcm_writei(ao->pcm, buffer, frames);
    if (rc == -EPIPE) { snd_pcm_prepare(ao->pcm); }
  }
}

void
killsfx() {
  snd_pcm_drain(ao->pcm);
  snd_pcm_close(ao->pcm);

  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();

  free(buffer);
  free(ao);
}
