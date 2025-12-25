/**
Thanks to @murdog for creating this class.
More info: https://forum.pjrc.com/index.php?threads/fm-synthesis-unpredictable.68650/
*/
#include <Arduino.h>
#include "AudioSynthWaveformModulated2.h"
#include "arm_math.h"
#include "utility/dspinst.h"

void AudioSynthWaveformModulated2::update(void)
{
	audio_block_t *block, *moddata, *shapedata, *fmoddata;
	int16_t *bp, *bpf, *end;
	int32_t val1, val2;
	int16_t magnitude15;
	uint32_t i, ph, index, index2, scale, priorphase;
	const uint32_t inc = phase_increment;
  
	moddata = receiveReadOnly(0);
	shapedata = receiveReadOnly(1);
  fmoddata = receiveReadOnly(2);

	ph = phase_accumulator;

	priorphase = phasedata[AUDIO_BLOCK_SAMPLES-1];
	if (moddata && modulation_type == 0) {
		// Frequency Modulation
		bp = moddata->data;
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			int32_t n = (*bp++) * modulation_factor; // n is # of octaves to mod
			int32_t ipart = n >> 27; // 4 integer bits
			n &= 0x7FFFFFF;          // 27 fractional bits
			n = (n + 134217728) << 3;
			n = multiply_32x32_rshift32_rounded(n, n);
			n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
			n = n + 715827882;
			uint32_t scale = n >> (14 - ipart);
			uint64_t phstep = (uint64_t)inc * scale;
			uint32_t phstep_msw = phstep >> 32;
			if (phstep_msw < 0x7FFE) {
				ph += phstep >> 16;
			} else {
				ph += 0x7FFE0000;
			}
			phasedata[i] = ph;
		}
		release(moddata);
    if (fmoddata) release(fmoddata);
//    return;   
	} else if (moddata && modulation_type == 1) {

		// Phase Modulation
		bp = moddata->data;
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			// more than +/- 180 deg shift by 32 bit overflow of "n"
			uint32_t n = (uint16_t)(*bp++) * modulation_factor;
			phasedata[i] = ph + n;
			ph += inc;
		}
		release(moddata);
    if (fmoddata) release(fmoddata);
	} else if (moddata && fmoddata && modulation_type == 2) {
    bpf = fmoddata->data;
    uint32_t inc2[AUDIO_BLOCK_SAMPLES];
    for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
      int32_t n = (*bpf++) * fmodulation_factor; // n is # of octaves to mod
      int32_t ipart = n >> 27; // 4 integer bits
      n &= 0x7FFFFFF;          // 27 fractional bits
      n = (n + 134217728) << 3;
      n = multiply_32x32_rshift32_rounded(n, n);
      n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
      n = n + 715827882;
      uint32_t scale = n >> (14 - ipart);
      uint64_t phstep = (uint64_t)inc * scale;
      uint32_t phstep_msw = phstep >> 32;
      if (phstep_msw < 0x7FFE) {
        inc2[i] = phstep >> 16;
      } else {
        inc2[i] = 0x7FFE0000;
      }
    }
    release(fmoddata);
 
    bp = moddata->data;
    for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
      // more than +/- 180 deg shift by 32 bit overflow of "n"
      uint32_t n = (uint16_t)(*bp++) * modulation_factor;
      phasedata[i] = ph + n;
      ph += inc2[i];
    }
    release(moddata);

  } else if (fmoddata && modulation_type == 2) {

    bpf = fmoddata->data;
    // uint32_t inc2[AUDIO_BLOCK_SAMPLES];
    for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
      int32_t n = (*bpf++) * fmodulation_factor; // n is # of octaves to mod
      int32_t ipart = n >> 27; // 4 integer bits
      n &= 0x7FFFFFF;          // 27 fractional bits
      n = (n + 134217728) << 3;
      n = multiply_32x32_rshift32_rounded(n, n);
      n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
      n = n + 715827882;
      uint32_t scale = n >> (14 - ipart);
      uint64_t phstep = (uint64_t)inc * scale;
      uint32_t phstep_msw = phstep >> 32;

      if (phstep_msw < 0x7FFE) {
        ph += phstep >> 16;
      } else {
        ph += 0x7FFE0000;
      }
      phasedata[i] = ph;    
    
    
    }
    release(fmoddata);
  } else if (moddata && modulation_type == 2) {
    // Phase Modulation
    bp = moddata->data;
    for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
      // more than +/- 180 deg shift by 32 bit overflow of "n"
      uint32_t n = (uint16_t)(*bp++) * modulation_factor;
      phasedata[i] = ph + n;
      ph += inc;
    }
    release(moddata);
    if (fmoddata) release(fmoddata);		
	}	else {
		// No Modulation Input
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			phasedata[i] = ph;
			ph += inc;
		}
    if (fmoddata) release(fmoddata);
	}
	phase_accumulator = ph;

	// If the amplitude is zero, no output, but phase still increments properly
	if (magnitude == 0) {
		if (shapedata) release(shapedata);
		return;
	}
	block = allocate();
	if (!block) {
		if (shapedata) release(shapedata);
		return;
	}
	bp = block->data;

	// Now generate the output samples using the pre-computed phase angles
	switch(tone_type) {
	case WAVEFORM_SINE:
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			ph = phasedata[i];
			index = ph >> 24;
			val1 = AudioWaveformSine[index];
			val2 = AudioWaveformSine[index+1];
			scale = (ph >> 8) & 0xFFFF;
			val2 *= scale;
			val1 *= 0x10000 - scale;
			*bp++ = multiply_32x32_rshift32(val1 + val2, magnitude);
		}
		break;

	case WAVEFORM_ARBITRARY:
		if (!arbdata) {
			release(block);
			if (shapedata) release(shapedata);
			return;
		}
		// len = 256
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			ph = phasedata[i];
			index = ph >> 24;
			index2 = index + 1;
			if (index2 >= 256) index2 = 0;
			val1 = *(arbdata + index);
			val2 = *(arbdata + index2);
			scale = (ph >> 8) & 0xFFFF;
			val2 *= scale;
			val1 *= 0x10000 - scale;
			*bp++ = multiply_32x32_rshift32(val1 + val2, magnitude);
		}
		break;

	case WAVEFORM_PULSE:
		if (shapedata) {
			magnitude15 = signed_saturate_rshift(magnitude, 16, 1);
			for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
				uint32_t width = ((shapedata->data[i] + 0x8000) & 0xFFFF) << 16;
				if (phasedata[i] < width) {
					*bp++ = magnitude15;
				} else {
					*bp++ = -magnitude15;
				}
			}
			break;
		} // else fall through to orginary square without shape modulation

	case WAVEFORM_SQUARE:
		magnitude15 = signed_saturate_rshift(magnitude, 16, 1);
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			if (phasedata[i] & 0x80000000) {
				*bp++ = -magnitude15;
			} else {
				*bp++ = magnitude15;
			}
		}
		break;

	case WAVEFORM_SAWTOOTH:
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			*bp++ = signed_multiply_32x16t(magnitude, phasedata[i]);
		}
		break;

	case WAVEFORM_SAWTOOTH_REVERSE:
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			*bp++ = signed_multiply_32x16t(0xFFFFFFFFu - magnitude, phasedata[i]);
		}
		break;

	case WAVEFORM_TRIANGLE_VARIABLE:
		if (shapedata) {
			for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
				uint32_t width = (shapedata->data[i] + 0x8000) & 0xFFFF;
				uint32_t rise = 0xFFFFFFFF / width;
				uint32_t fall = 0xFFFFFFFF / (0xFFFF - width);
				uint32_t halfwidth = width << 15;
				uint32_t n;
				ph = phasedata[i];
				if (ph < halfwidth) {
					n = (ph >> 16) * rise;
					*bp++ = ((n >> 16) * magnitude) >> 16;
				} else if (ph < 0xFFFFFFFF - halfwidth) {
					n = 0x7FFFFFFF - (((ph - halfwidth) >> 16) * fall);
					*bp++ = (((int32_t)n >> 16) * magnitude) >> 16;
				} else {
					n = ((ph + halfwidth) >> 16) * rise + 0x80000000;
					*bp++ = (((int32_t)n >> 16) * magnitude) >> 16;
				}
				ph += inc;
			}
			break;
		} // else fall through to orginary triangle without shape modulation

	case WAVEFORM_TRIANGLE:
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			ph = phasedata[i];
			uint32_t phtop = ph >> 30;
			if (phtop == 1 || phtop == 2) {
				*bp++ = ((0xFFFF - (ph >> 15)) * magnitude) >> 16;
			} else {
				*bp++ = (((int32_t)ph >> 15) * magnitude) >> 16;
			}
		}
		break;
	case WAVEFORM_SAMPLE_HOLD:
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
			ph = phasedata[i];
			if (ph < priorphase) { // does not work for phase modulation
				sample = random(magnitude) - (magnitude >> 1);
			}
			priorphase = ph;
			*bp++ = sample;
		}
		break;
	}
 
	if (tone_offset) {
		bp = block->data;
		end = bp + AUDIO_BLOCK_SAMPLES;
		do {
			val1 = *bp;
			*bp++ = signed_saturate_rshift(val1 + tone_offset, 16, 0);
		} while (bp < end);
	}
	if (shapedata) release(shapedata);
	transmit(block, 0);
	release(block);
}