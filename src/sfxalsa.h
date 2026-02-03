#ifndef SFXALSA_H
#define SFXALSA_H

#include <alsa/asoundlib.h>

typedef struct {
  snd_pcm_t *pcm;
  snd_pcm_uframes_t frames;
  int rate;
  int chnls;
} Aout;

typedef struct {
  short *pcm;
  size_t frames;
  int rate;
  int chnls;
} Soundfx;

typedef enum {
  SFX_BOOM,
  SFX_COUNT
} SfxID;

typedef struct {
  Soundfx *snd;
  size_t framepos;
  float volume;
  int active;
  int loop;
} Voice;

int initsfx();
int playsfx();
void killsfx();

#endif /* SFXALSA_H */
