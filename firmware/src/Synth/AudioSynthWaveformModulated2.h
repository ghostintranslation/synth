/**
Thanks to @murdog for creating this class.
More info: https://forum.pjrc.com/index.php?threads/fm-synthesis-unpredictable.68650/
*/

#ifndef synth_waveform2_h_
#define synth_waveform2_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "arm_math.h"

// waveforms.c
extern "C" {
extern const int16_t AudioWaveformSine[257];
}


#define WAVEFORM_SINE              0
#define WAVEFORM_SAWTOOTH          1
#define WAVEFORM_SQUARE            2
#define WAVEFORM_TRIANGLE          3
#define WAVEFORM_ARBITRARY         4
#define WAVEFORM_PULSE             5
#define WAVEFORM_SAWTOOTH_REVERSE  6
#define WAVEFORM_SAMPLE_HOLD       7
#define WAVEFORM_TRIANGLE_VARIABLE 8

class AudioSynthWaveformModulated2 : public AudioStream
{
public:
	AudioSynthWaveformModulated2(void) : AudioStream(3, inputQueueArray),
		phase_accumulator(0), phase_increment(0), modulation_factor(32768), fmodulation_factor(32768),
		magnitude(0), arbdata(NULL), sample(0), tone_offset(0),
		tone_type(WAVEFORM_SINE), modulation_type(0) {
	}

	void frequency(float freq) {
		if (freq < 0.0) {
			freq = 0.0;
		} else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2) {
			freq = AUDIO_SAMPLE_RATE_EXACT / 2;
		}
		phase_increment = freq * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
		if (phase_increment > 0x7FFE0000u) phase_increment = 0x7FFE0000;
	}
	void amplitude(float n) {	// 0 to 1.0
		if (n < 0) {
			n = 0;
		} else if (n > 1.0) {
			n = 1.0;
		}
		magnitude = n * 65536.0;
	}
	void offset(float n) {
		if (n < -1.0) {
			n = -1.0;
		} else if (n > 1.0) {
			n = 1.0;
		}
		tone_offset = n * 32767.0;
	}
	void begin(short t_type) {
		tone_type = t_type;
	}
	void begin(float t_amp, float t_freq, short t_type) {
		amplitude(t_amp);
		frequency(t_freq);
		tone_type = t_type;
	}
	void arbitraryWaveform(const int16_t *data, float maxFreq) {
		arbdata = data;
	}
	void frequencyModulation(float octaves) {
		if (octaves > 12.0) {
			octaves = 12.0;
		} else if (octaves < 0.1) {
			octaves = 0.1;
		}
		modulation_factor = octaves * 4096.0;
		modulation_type = 0;
	}
	void phaseModulation(float degrees) {
		if (degrees > 9000.0) {
			degrees = 9000.0;
		} else if (degrees < 30.0) {
			degrees = 30.0;
		}
		modulation_factor = degrees * (65536.0 / 180.0);
		modulation_type = 1;
	}
  void pitchphaseModulation(float octaves, float degrees) {
    if (octaves > 12.0) {
      octaves = 12.0;
    } else if (octaves < 0.1) {
      octaves = 0.1;
    }
    fmodulation_factor = octaves * 4096.0;
    if (degrees > 9000.0) {
      degrees = 9000.0;
    } else if (degrees < 30.0) {
      degrees = 30.0;
    }
    modulation_factor = degrees * (65536.0 / 180.0);        
    modulation_type = 2;
  } 
	virtual void update(void);

private:
	audio_block_t *inputQueueArray[3];
	uint32_t phase_accumulator;
//  uint32_t phase_accumulator2;
	uint32_t phase_increment;
	uint32_t modulation_factor;
  uint32_t fmodulation_factor;
	int32_t  magnitude;
	const int16_t *arbdata;
	uint32_t phasedata[AUDIO_BLOCK_SAMPLES];
//  uint32_t phasedata2[AUDIO_BLOCK_SAMPLES+1];
	int16_t  sample; // for WAVEFORM_SAMPLE_HOLD
	int16_t  tone_offset;
	uint8_t  tone_type;
	uint8_t  modulation_type;
};


#endif