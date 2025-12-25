#ifndef Voice_h
#define Voice_h

#include <Audio.h>
#include "Motherboard/Vcc.h"
#include "AudioSynthWaveformModulated2.h"

/*
 * Voice
 */
class Voice : public AudioStream {
protected:
  audio_block_t *inputQueueArray[6];

  AudioPlayQueue *shapeQueue;
  AudioPlayQueue *accentQueue;
  // AudioPlayQueue *fmQueue;
  AudioRecordQueue *audioOutQueue;
  AudioSynthWaveformModulated2 *sine;
  AudioSynthWaveformModulated2 *sawtooth;
  Vcc *shapeCrossfader;
  AudioEffectEnvelope *envelope;
  AudioEffectEnvelope *accentEnvelope;
  AudioSynthWaveformModulated *phaseModulator;

  byte note = 0;
  byte octave = 0;
  float pitch = 0;
  bool isLastPlayedMidi = false;
  float frequency = 0;
  float frequencyTarget = 0;
  float glide = 0;
  float accentMultiplier = 0;
  bool isHolding = false;

public:
  Voice();
  void update();

  void noteOn(byte midiNote, float vel);
  void noteOn(float vel);
  void noteOff();
  void setNote(byte midiNote);
  void setFrequency(float freq);
  void setFrequencyTarget(float freq);
  void setModulatorFrequencyRatio(float ratio);
  void setModulatorAmplitude(float amp);
  void setGlide(float glide);
  void setOctave(byte octave);
  void setPitch(float pitch);
  void setAccent(float accent);
  void setAccentMultiplier(float accentMultiplier);
  bool isActive();
  bool isSustain();
  byte getNote();
  unsigned long lastPlayed = 0;
};

/**
 * Constructor
 */
inline Voice::Voice()
  : AudioStream(6, inputQueueArray) {

  this->sine = new AudioSynthWaveformModulated2();
  this->sine->begin(WAVEFORM_SINE);
  this->sine->pitchphaseModulation(1, 180);
  this->sine->frequency(this->frequency);
  this->sine->amplitude(1);

  this->sawtooth = new AudioSynthWaveformModulated2();
  this->sawtooth->begin(WAVEFORM_SAWTOOTH);
  this->sawtooth->pitchphaseModulation(1, 180);
  this->sawtooth->frequency(this->frequency);
  this->sawtooth->amplitude(1);

  this->envelope = new AudioEffectEnvelope();
  this->envelope->sustain(1);
  this->envelope->attack(5);
  this->envelope->release(500);

  this->accentEnvelope = new AudioEffectEnvelope();
  this->accentEnvelope->sustain(1);
  this->accentEnvelope->attack(5);
  this->accentEnvelope->release(500);

  this->shapeCrossfader = new Vcc();

  this->phaseModulator = new AudioSynthWaveformModulated();
  this->phaseModulator->begin(WAVEFORM_SINE);
  this->phaseModulator->frequencyModulation(1);
  this->phaseModulator->frequency(this->frequency);
  this->phaseModulator->amplitude(0);
  this->phaseModulator->phaseModulation(180);

  this->shapeQueue = new AudioPlayQueue();
  this->accentQueue = new AudioPlayQueue();
  // this->fmQueue = new AudioPlayQueue();
  this->audioOutQueue = new AudioRecordQueue();
  this->audioOutQueue->begin();

  new AudioConnection(*this->phaseModulator, 0, *this->accentEnvelope, 0);
  new AudioConnection(*this->accentEnvelope, 0, *this->sine, 0);
  // new AudioConnection(*this->fmQueue, 0, *this->sine, 2);
  new AudioConnection(*this->accentEnvelope, 0, *this->sawtooth, 0);
  // new AudioConnection(*this->fmQueue, 0, *this->sawtooth, 2);
  new AudioConnection(*this->sine, 0, *this->shapeCrossfader, 0);
  new AudioConnection(*this->sawtooth, 0, *this->shapeCrossfader, 1);
  new AudioConnection(*this->shapeQueue, 0, *this->shapeCrossfader, 2);
  new AudioConnection(*this->shapeCrossfader, 0, *this->envelope, 0);
  new AudioConnection(*this->envelope, 0, *this->audioOutQueue, 0);
}

/**
 * Update
 */
inline void Voice::update() {
  // Receive input data
  audio_block_t *octaveBlock;
  audio_block_t *pitchBlock;
  // audio_block_t *fmBlock;
  audio_block_t *attackBlock;
  audio_block_t *releaseBlock;
  audio_block_t *shapeBlock;
  audio_block_t *holdBlock;

  // Octave block
  octaveBlock = receiveReadOnly(0);

  if (octaveBlock) {
    this->octave = (float)((octaveBlock->data[0] + 32668) / 65335.0) * 6;  // 6 octaves

    release(octaveBlock);
  }


  // Pitch block
  pitchBlock = receiveReadOnly(1);
  if (pitchBlock) {
    this->pitch = (float)((pitchBlock->data[0] + 32668) / 65335.0) - 0.5;
    release(pitchBlock);
  }

  // Setting the frequency
  this->frequency = this->frequency * this->glide + this->frequencyTarget * (1.0 - this->glide);
  this->sine->frequency(this->frequency);
  this->sawtooth->frequency(this->frequency);
  this->phaseModulator->frequency(this->frequency * this->accentMultiplier);

  // FM block
  // fmBlock = receiveReadOnly(2);
  // if (fmBlock) {
  //   // This is connected to the sine object via an AudioConnection
  //   this->fmQueue->play(fmBlock->data, AUDIO_BLOCK_SAMPLES);
  //   release(fmBlock);
  // }

  // Attack Block
  attackBlock = receiveReadOnly(2);
  if (attackBlock) {
    float attack = ((float)(attackBlock->data[0] + 32768) / 65335) * 2000 - 3;
    if (attack < 5) {
      attack = 5;
    }
    this->envelope->attack(attack);
    this->accentEnvelope->attack(attack);
    release(attackBlock);
  }

  // Release Block
  releaseBlock = receiveReadOnly(3);
  if (releaseBlock) {
    // float rel = ((float)(releaseBlock->data[0] + 32768) / 65335) * 4000 * (2 * (1 + this->isHolding) - 1);
    float rel = ((float)(releaseBlock->data[0] + 32768) / 65335) * 4000;
    this->envelope->release(rel);
    this->accentEnvelope->release(rel);
    release(releaseBlock);
  }

  // Shape block
  shapeBlock = receiveReadOnly(4);
  if (shapeBlock) {
    // This is connected to the shapeCrossfader object via an AudioConnection
    this->shapeQueue->play(shapeBlock->data, AUDIO_BLOCK_SAMPLES);
    release(shapeBlock);
  }

  // Hold block
  holdBlock = receiveReadOnly(5);
  if (holdBlock) {
    this->isHolding = holdBlock->data[0] > 0;
    release(holdBlock);
  }

  // // Transmitting the audio queue 1
  if (this->audioOutQueue->available()) {
    while (this->audioOutQueue->available()) {
      audio_block_t *audioBlock = allocate();
      if (audioBlock) {
        int16_t *queueData = this->audioOutQueue->readBuffer();
        for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
          audioBlock->data[i] = queueData[i];
        }
        transmit(audioBlock, 0);
        release(audioBlock);
        this->audioOutQueue->freeBuffer();
      }
    }
  }
}

/**
 * Note on
 */
inline void Voice::noteOn(byte midiNote, float vel) {
  if (vel > 1) {
    vel = 1;
  }
  
  if (vel < 0) {
    vel = 0;
  }
  
  this->isLastPlayedMidi = true;
  this->setNote(midiNote);
  this->noteOn(vel);
}

inline void Voice::noteOn(float vel) {
  if (vel > 1) {
    vel = 1;
  }

  if (vel < 0) {
    vel = 0;
  }

  this->isLastPlayedMidi = false;
  this->lastPlayed = millis();
  // this->sine->phaseModulation(180);
  // this->sawtooth->phaseModulation(180);
  // this->phaseModulator->phaseModulation(180);
  this->sine->amplitude(vel);
  this->sawtooth->amplitude(vel);
  this->envelope->noteOn();
  this->accentEnvelope->noteOn();
}

/**
 * Note off
 */
inline void Voice::noteOff() {

  if (this->isHolding) {
    // Not listening to note off if holding
    return;
  }

  this->envelope->noteOff();
  this->accentEnvelope->noteOff();
}

inline void Voice::setNote(byte midiNote) {
  this->note = midiNote;
  this->frequencyTarget = 440.0 * powf(2.0, (float)(((this->note - 12) + 12 * (this->octave + this->pitch)) - 69) * 0.08333333);  // 440Hz = midi 69
}

/**
 * Is the voice active
 */
inline bool Voice::isActive() {
  return this->envelope->isActive();
}

inline bool Voice::isSustain() {
  return this->envelope->isSustain();
}

/**
 * Set the trgeted frequency
 */
inline void Voice::setFrequencyTarget(float frequencyTarget) {
  this->frequencyTarget = frequencyTarget;
}

/**
 * Set the octave
 */
inline void Voice::setOctave(byte octave) {
  this->octave = octave;
}

/**
 * Set the pitch
 */
inline void Voice::setPitch(float pitch) {
  this->pitch = pitch;
}

/**
 * Set the glide
 */
inline void Voice::setGlide(float glide) {
  if (glide > 1.0) {
    glide = 1.0;
  }

  if (glide < 0.0) {
    glide = 0.0;
  }

  this->glide = glide;
}

inline void Voice::setAccent(float accent) {
  if (accent > 1) {
    accent = 1;
  }

  if (accent < 0) {
    accent = 0;
  }

  this->phaseModulator->amplitude(accent);
}

inline void Voice::setAccentMultiplier(float accentMultiplier) {
  if (accentMultiplier > 1) {
    accentMultiplier = 1;
  }

  if (accentMultiplier < 0) {
    accentMultiplier = 0;
  }

  this->accentMultiplier = accentMultiplier * 4;
}

/**
 * Get the voice's current note
 */
inline byte Voice::getNote() {
  return this->note;
}

#endif
