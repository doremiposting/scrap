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
Soundfx *s[SFX_COUNT];
Triggersfx triggers[SFX_COUNT];
#define MAXVCS 32
Voice *voices;
static unsigned int voicecnt;
#define MIXFRAMES 512
static short *mixbuffer;

Soundfx *
newsnd() {
  /* TODO: expand to initialize anything as pcm, require reuse of mh et. al. */
  Soundfx *ss;
  unsigned char *a;
  size_t cap;
  unsigned char b[8196];
  int r, i;
  /* s = calloc(SFX_COUNT, sizeof(Soundfx)); */
  ss = calloc(1, sizeof(Soundfx));
  voices = calloc(MAXVCS, sizeof(Voice));
  voicecnt = 0;
  for (i = 0 ; i < SFX_COUNT ; i++) { triggers[i].on = 0; triggers[i].cut = 0; }
  a = NULL; cap = 0;
  ss->rate = (int)rate; /* TODO: Should the struct member be changed to long? */
  ss->chnls = chnls;
  while(mpg123_read(mh, (unsigned char *)&a, 0, &done) != MPG123_DONE) {
    r = mpg123_read(mh, b, sizeof(b), &done);
    if (r == MPG123_OK && done > 0) {
      a = realloc(a, cap+done);
      memcpy(a + cap, b, done);
      cap += done;
    }
  }
  ss->pcm = (short*)a;
  ss->frames = (size_t)(cap / ((size_t)chnls * sizeof(short)));
  return ss;
}

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
  mixbuffer = calloc(MIXFRAMES * chnls, sizeof(short));
  
  /* TODO: newsnd() exists, but it needs to take a const char* for passing file paths.. */
  s[SFX_BOOM] = newsnd();

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

void
triggersfx(SfxID id, int cut) {
  if (!triggers[id].on) { triggers[id].on = 1; triggers[id].cut = cut; }
}

int
playsfx() {
  snd_pcm_sframes_t frames, mixableframes, remainingframes;
  size_t f, mixi, srci;
  int i, v, ch, activevc, mixed;
  Voice *voice;
  Soundfx *sfx;
  activevc = 0;
  
  for (i = 0; i < voicecnt; i++) {
    if (!triggers[i].on) { continue; }
    sfx = s[i];
    if (!sfx) { continue; }

    voice = NULL;
    for (v = 0; v < voicecnt; v++) {
      if (voices[v].id == i && voices[v].active) { voice = &voices[v]; break; }
    }
    if (voice && triggers[i].cut) { voice->position = 0; }
    if (!voice) {
      for (v = 0; v < voicecnt; v++) {
        if (!voices[v].active) {
          voice = &voices[i];
          voice->id = i;
          voice->active = 1;
          voice->position = 0;
          if (v >= voicecnt) { voicecnt = v+1; }
          break;
        }
      }
      if (!voice && MAXVCS > 0) { 
        voice = &voices[0];
        voice->id = i;
        voice->active = 1;
        voice->position = 0;
      }
    }

    triggers[i].on = 0;
  }

  memset(mixbuffer, 0, MIXFRAMES * chnls * sizeof(mixbuffer));

  for (v = 0; v < voicecnt; v++) {
    voice = &voices[v];
    if (!voice->active) { continue; }
    
    sfx = s[voice->id];
    if (!sfx || !sfx->pcm) { voice->active = 0; continue; }

    mixableframes = MIXFRAMES;
    remainingframes = sfx->frames - voice->position;
    if (mixableframes > remainingframes) { mixableframes = remainingframes; }

    for (f = 0; f < mixableframes; f++) {
      for (ch = 0; ch < chnls; ch++) {
        mixi = f * chnls + ch;
        srci = (voice->position + f) * chnls + ch;

        mixed = mixbuffer[mixi] + sfx->pcm[srci];
        /* TODO: Don't use magic numbers here. */
        if (mixed > 32767) { mixed = 32767; }
        if (mixed < -32768) { mixed = -32768; }
        mixbuffer[mixi] = (short) mixed;
      }
    }

    voice->position = mixableframes;
    activevc++;

    if (voice->position >= sfx->frames) { voice->active = 0; }
  }

  if (activevc > 0) {
    snd_pcm_prepare(ao->pcm);
    frames = snd_pcm_writei(ao->pcm, mixbuffer, MIXFRAMES);
    if (frames < 0) { snd_pcm_recover(ao->pcm, (int)frames, 0); }
  }
  return activevc;
}

void
killsfx() {
  snd_pcm_drain(ao->pcm);
  snd_pcm_close(ao->pcm);

  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();

  free(s[SFX_BOOM]->pcm);
  free (voices);
  free(buffer);
  free(ao);
}
