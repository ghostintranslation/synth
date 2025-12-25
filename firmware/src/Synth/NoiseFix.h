#ifndef NoiseFix_h
#define NoiseFix_h

#include <Audio.h>

// This class makes sure the output returns 1s when there is not input
// so that the output is never 0 when no input signal.
// This is to elimiate the PCM5100 jumps to complete shutdown when the signal is only 0s
// which produces hearable clicksw
class NoiseFix : public AudioStream {
protected:
  audio_block_t *inputQueueArray[1];

public:
  NoiseFix();
  void update();
};

inline NoiseFix::NoiseFix()
  : AudioStream(1, inputQueueArray) {
}

inline void NoiseFix::update() {
  audio_block_t *in = receiveReadOnly(0);

  if (in) {
    transmit(in);
    release(in);
  } else {
    audio_block_t *out = allocate();
    if (out) {
      for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        out->data[i] = 1;
      }
      transmit(out);
      release(out);
    }
  }
}

#endif