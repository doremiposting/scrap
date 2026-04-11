#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <mpg123.h>
#include <AudioUnit/AudioUnit.h>

#include "sfx.h"
#include "sfxca.h"
#include "main.h"

#define MAXVCS 32
#define CHANNELS 2

static AudioUnit outputunit;
static Soundfx *s[SFX_COUNT];
static Triggersfx triggers[SFX_COUNT];
static Voice voices[MAXVCS];
static unsigned int voicecnt;
static pthread_mutex_t sfxmutex;

static Soundfx *
loadsnd(const char *path) {
  mpg123_handle *mh;
  Soundfx *ss;
  unsigned char *a, b[8192];
  size_t cap, done;
  long rate;
  int err, chnls, enc, r;
  mh = mpg123_new(NULL, &err);
  a = NULL;
  cap = 0;
  ss = calloc(1, sizeof(Soundfx));
  mpg123_open(mh, path);
  mpg123_getformat(mh, &rate, &chnls, &enc);
  mpg123_format_none(mh);
  mpg123_format(mh, rate, chnls, MPG123_ENC_SIGNED_16);
  ss->rate = (int)rate;
  ss->chnls = chnls;
  while ((r = mpg123_read(mh, b, sizeof(b), &done)) != MPG123_DONE) {
    if (r == MPG123_OK && done > 0) {
      a = realloc(a, cap+done);
      memcpy(a+cap, b, done);
      cap += done;
    }
  }
  ss->pcm = (short *)a;
  ss->frames = cap / ((size_t)chnls * sizeof(short));
  mpg123_close(mh);
  mpg123_delete(mh);
  return ss;
}

static void
mixaudio(void *buf, unsigned int nframes) {
  memset(buf, 0, nframes *sizeof(int16_t) * 2);
}

static OSStatus
rendercb(void *ref, AudioUnitRenderActionFlags *flags,
    const AudioTimeStamp *ts, UInt32 bus, UInt32 nframes,
    AudioBufferList *bufs) {
  mixaudio(bufs->mBuffers[0].mData, nframes);
  return noErr;
}

void
sfxinit() {
  AudioComponent comp;
  AudioComponentDescription desc = {
    .componentType = kAudioUnitType_Output,
    .componentSubType = kAudioUnitSubType_DefaultOutput,
    .componentManufacturer = kAudioUnitManufacturer_Apple,
  };
  comp = AudioComponentFindNext(NULL, &desc);
  AudioComponentInstanceNew(comp, &outputunit);
  AudioStreamBasicDescription fmt = {
    .mSampleRate = 44100,
    .mFormatID = kAudioFormatLinearPCM,
    .mBitsPerChannel = 16,
    .mChannelsPerFrame = 2,
    .mFramesPerPacket = 1,
    .mBytesPerFrame = sizeof(int16_t) * 2,
    .mBytesPerPacket = sizeof(int16_t) * 2,
  };
  AudioUnitSetProperty(outputunit,
      kAudioUnitProperty_StreamFormat,
      kAudioUnitScope_Input, 0,
      &fmt, sizeof(fmt));
  AURenderCallbackStruct cb = { rendercb, NULL };
  AudioUnitSetProperty(outputunit,
      kAudioUnitProperty_SetRenderCallback,
      kAudioUnitScope_Input, 0,
      &cb, sizeof(cb));
  AudioUnitInitialize(outputunit);
  AudioOutputUnitStart(outputunit);
}

void
triggersfx(SfxID id, int cut) {
}

void
sfxkill() {
  AudioOutputUnitStop(outputunit);
  AudioUnitUninitialize(outputunit);
  AudioComponentInstanceDispose(outputunit);
}
