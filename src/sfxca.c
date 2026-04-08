#include <stdint.h>
#include <string.h>
#include <AudioUnit/AudioUnit.h>

static AudioUnit outputunit;

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
sfxkill() {
  AudioOutputUnitStop(outputunit);
  AudioUnitUninitialize(outputunit);
  AudioComponentInstanceDispose(outputunit);
}
