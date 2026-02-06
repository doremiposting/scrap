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
  SFX_BGM,
  SFX_BOOM,
  SFX_COUNT
} SfxID;

typedef struct {
  int on;
  int cut;
} Triggersfx;

typedef struct {
  Soundfx *snd;
  SfxID id;
  size_t position;
  float volume;
  int active;
  int loop;
} Voice;

int initsfx();
void triggersfx(SfxID id, int cut);
int playsfx();
void killsfx();

#endif /* SFXALSA_H */
