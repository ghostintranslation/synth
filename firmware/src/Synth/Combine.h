#ifndef Combine1_h
#define Combine1_h

#include <Audio.h>

/**
 * Combines analog signals by offseting them first
 * so that there is no negative values in the sum
 */
template<int NN> class Combine : public AudioStream {
private:
  audio_block_t *inputQueueArray[NN];
  void (*onChangeCallback)(int16_t value) = nullptr;
  float lowPassCoeff = 0.0005;
  float accumulator;
  float prevAccumulator;

public:
  Combine(void)
    : AudioStream(NN, inputQueueArray) {
  }

  void update(void) {
    audio_block_t *in, *out = NULL;

    out = allocate();

    if (!out) {
      return;
    }

    //TODO: There must be a better way to do it
    int32_t data[AUDIO_BLOCK_SAMPLES] = { 0 };
    for (unsigned int channel = 0; channel < NN; channel++) {
      in = receiveReadOnly(channel);
      if (in) {

        for (unsigned int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
          data[i] = constrain(data[i] + in->data[i] + INT16_MAX, 0, UINT16_MAX);
        }
        release(in);
      }
    }

    for (unsigned int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      out->data[i] = constrain(data[i] - INT16_MAX, INT16_MIN, INT16_MAX);
      accumulator = (lowPassCoeff * out->data[i]) + (1.0f - lowPassCoeff) * out->data[i];
    }

    if (this->onChangeCallback && prevAccumulator != accumulator) {
      this->onChangeCallback(accumulator);
      prevAccumulator = accumulator;
    }

    transmit(out);
    release(out);
  }


  void onChange(void (*onChangeCallback)(int16_t value)) {
    this->onChangeCallback = onChangeCallback;
  }
};
#endif