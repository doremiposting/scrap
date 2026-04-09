#ifndef SFXALSA_H
#define SFXALSA_H

#include <alsa/asoundlib.h>

typedef struct {
  snd_pcm_t *pcm;
  snd_pcm_uframes_t frames;
  int rate;
  int chnls;
} Aout;

int initsfx();
void triggersfx(SfxID id, int cut);
void killsfx();

#endif /* SFXALSA_H */
