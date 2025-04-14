#ifndef mixer2_h_
#define mixer2_h_

#include "Arduino.h"
#include "AudioStream.h"

#define AudioMixer4 AudioMixer<4>

#if defined(__ARM_ARCH_7EM__)

#define MULTI_UNITYGAIN 65536
#define MULTI_UNITYGAIN_F 65536.0f
#define MAX_GAIN 32767.0f
#define MIN_GAIN -32767.0f
#define MULT_DATA_TYPE int32_t

#elif defined(KINETISL)

#define MULTI_UNITYGAIN 256
#define MULTI_UNITYGAIN_F 256.0f
#define MAX_GAIN 127.0f
#define MIN_GAIN -127.0f
#define MULT_DATA_TYPE int16_t

#endif

template <int NN> class AudioMixer : public AudioStream
{
public:
	AudioMixer(void) : AudioStream(NN, inputQueueArray) {
		for (int i=0; i<NN; i++) multiplier[i] = MULTI_UNITYGAIN;
	}	
	void update();
	/**
	 * this sets the individual gains
	 * @param channel
	 * @param gain
	 */
	void gain(unsigned int channel, float gain);
	/**
	 * set all channels to specified gain
	 * @param gain
	 */
	void gain(float gain);

private:
	MULT_DATA_TYPE multiplierTarget[NN];
	MULT_DATA_TYPE multiplier[NN];
	audio_block_t *inputQueueArray[NN];
};


// the following Forward declarations 
// must be defined when we use template 
// the compiler throws some warnings that should be errors otherwise
static inline int32_t signed_multiply_32x16b(int32_t a, uint32_t b); 
static inline int32_t signed_multiply_32x16t(int32_t a, uint32_t b);
static inline int32_t signed_saturate_rshift(int32_t val, int bits, int rshift);
static inline uint32_t pack_16b_16b(int32_t a, int32_t b);
static inline uint32_t signed_add_16_and_16(uint32_t a, uint32_t b);

// because of the template use applyGain and applyGainThenAdd functions
// must be in this file and NOT in cpp file
#if defined(__ARM_ARCH_7EM__)

    static void applyGain(int16_t *data, int32_t mult)
    {
        uint32_t *p = (uint32_t *)data;
        const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

        do {
            uint32_t tmp32 = *p; // read 2 samples from *data
            int32_t val1 = signed_multiply_32x16b(mult, tmp32);
            int32_t val2 = signed_multiply_32x16t(mult, tmp32);
            val1 = signed_saturate_rshift(val1, 16, 0);
            val2 = signed_saturate_rshift(val2, 16, 0);
            *p++ = pack_16b_16b(val2, val1);
        } while (p < end);
    }

    static void applyGainThenAdd(int16_t *data, const int16_t *in, int32_t mult)
    {
        uint32_t *dst = (uint32_t *)data;
        const uint32_t *src = (uint32_t *)in;
        const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

        if (mult == MULTI_UNITYGAIN) {
            do {
                uint32_t tmp32 = *dst;
                *dst++ =  signed_add_16_and_16(tmp32, *src++);
                tmp32 = *dst;
                *dst++ =  signed_add_16_and_16(tmp32, *src++);
            } while (dst < end);
        } else {
            do {
                uint32_t tmp32 = *src++; // read 2 samples from *data
                int32_t val1 =  signed_multiply_32x16b(mult, tmp32);
                int32_t val2 =  signed_multiply_32x16t(mult, tmp32);
                val1 =  signed_saturate_rshift(val1, 16, 0);
                val2 =  signed_saturate_rshift(val2, 16, 0);
                tmp32 =  pack_16b_16b(val2, val1);
                uint32_t tmp32b = *dst;
                *dst++ =  signed_add_16_and_16(tmp32, tmp32b);
            } while (dst < end);
        }
    }

#elif defined(KINETISL)

    static void applyGain(int16_t *data, int32_t mult)
    {
        const int16_t *end = data + AUDIO_BLOCK_SAMPLES;

        do {
            int32_t val = *data * mult;
            *data++ = signed_saturate_rshift(val, 16, 0);
        } while (data < end);
    }

    static void applyGainThenAdd(int16_t *dst, const int16_t *src, int32_t mult)
    {
        const int16_t *end = dst + AUDIO_BLOCK_SAMPLES;

        if (mult == MULTI_UNITYGAIN) {
            do {
                int32_t val = *dst + *src++;
                *dst++ = signed_saturate_rshift(val, 16, 0);
            } while (dst < end);
        } else {
            do {
                int32_t val = *dst + ((*src++ * mult) >> 8); // overflow possible??
                *dst++ = signed_saturate_rshift(val, 16, 0);
            } while (dst < end);
        }
    }
#endif

template <int NN> void AudioMixer<NN>::gain(unsigned int channel, float gain) {
    if (channel >= NN) return;
    if (gain > MAX_GAIN) gain = MAX_GAIN;
    else if (gain < MIN_GAIN) gain = MIN_GAIN;
    multiplierTarget[channel] = gain * MULTI_UNITYGAIN_F; // TODO: proper roundoff?
}

template <int NN> void AudioMixer<NN>::gain(float gain) {
    for (int i = 0; i < NN; i++) {
        if (gain > MAX_GAIN) gain = MAX_GAIN;
        else if (gain < MIN_GAIN) gain = MIN_GAIN;
        multiplierTarget[i] = gain * MULTI_UNITYGAIN_F; // TODO: proper roundoff?
    } 
}

template <int NN> void AudioMixer<NN>::update() {
    audio_block_t *in, *out=NULL;
    unsigned int channel;
    for (channel=0; channel < NN; channel++) {
        multiplier[channel] = multiplierTarget[channel] * 0.01 + multiplier[channel] * 0.99;
        if (!out) {
            out = receiveWritable(channel);
            if (out) {
                int32_t mult = multiplier[channel];
                if (mult != MULTI_UNITYGAIN) applyGain(out->data, mult);
            }
        } else {
            in = receiveReadOnly(channel);
            if (in) {
                applyGainThenAdd(out->data, in->data, multiplier[channel]);
                release(in);
            }
        }
    }
    if (out) {
        transmit(out);
        release(out);
    }
}
#endif