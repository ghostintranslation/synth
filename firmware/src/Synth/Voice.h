#ifndef Voice_h
#define Voice_h

#include <Audio.h>
// #include "effect_envelope2.h" // This is a copy of the envelope from the audio library in order to have the latest version which includes a fix and is not yet in Teensyduino 1.60
#include "Motherboard/Vcc.h"
// #include "Envelope.h"

/*
 * Voice
 */
class Voice : public AudioStream {
protected:
  audio_block_t *inputQueueArray[5];

  AudioPlayQueue *shapeQueue;
  // AudioPlayQueue *gateQueue;
  AudioPlayQueue *fmQueue;
  AudioRecordQueue *audioOutQueue;

  AudioSynthWaveformModulated *sine;
  AudioSynthWaveformModulated *sawtooth;
  Vcc *shapeCrossfader;
  AudioEffectEnvelope *envelope;

  byte note = 0;
  byte octave = 0;
  bool isLastPlayedMidi = false;
  float frequency = 0;
  float frequencyTarget = 0;
  float glide = 0;

  // int16_t fmData[AUDIO_BLOCK_SAMPLES] = { 0 };
  // int16_t shapeData[AUDIO_BLOCK_SAMPLES] = { 0 };

public:
  Voice();
  void update();

  void noteOn(byte midiNote);
  void noteOn();
  void noteOff();
  void setNote(byte midiNote);
  void setFrequency(float freq);
  void setFrequencyTarget(int freq);
  void setModulatorFrequencyRatio(float ratio);
  void setModulatorAmplitude(float amp);
  void setGlide(float glide);
  void setOctave(byte octave);
  bool isActive();
  bool isSustain();
  byte getNote();
  unsigned long lastPlayed = 0;
};

/**
 * Constructor
 */
inline Voice::Voice()
  : AudioStream(5, inputQueueArray) {

  this->sine = new AudioSynthWaveformModulated();
  this->sine->begin(WAVEFORM_SINE);
  this->sine->frequencyModulation(1);
  this->sine->frequency(this->frequency);
  this->sine->amplitude(1);

  this->sawtooth = new AudioSynthWaveformModulated();
  this->sawtooth->begin(WAVEFORM_SAWTOOTH);
  this->sawtooth->frequencyModulation(1);
  this->sawtooth->frequency(this->frequency);
  this->sawtooth->amplitude(1);

  this->envelope = new AudioEffectEnvelope();
  this->envelope->sustain(1);
  this->envelope->attack(5);
  this->envelope->release(500);

  this->shapeCrossfader = new Vcc();

  this->shapeQueue = new AudioPlayQueue();
  // this->gateQueue = new AudioPlayQueue();
  this->fmQueue = new AudioPlayQueue();
  this->audioOutQueue = new AudioRecordQueue();
  this->audioOutQueue->begin();

  new AudioConnection(*this->fmQueue, 0, *this->sine, 0);
  new AudioConnection(*this->fmQueue, 0, *this->sawtooth, 0);
  new AudioConnection(*this->sine, 0, *this->shapeCrossfader, 0);
  new AudioConnection(*this->sawtooth, 0, *this->shapeCrossfader, 1);
  new AudioConnection(*this->shapeQueue, 0, *this->shapeCrossfader, 2);
  new AudioConnection(*this->shapeCrossfader, 0, *this->envelope, 0);
  // new AudioConnection(*this->gateQueue, 0, *this->envelope, 1);
  new AudioConnection(*this->envelope, 0, *this->audioOutQueue, 0);
}

/**
 * Update
 */
inline void Voice::update() {
  // Receive input data
  audio_block_t *octaveBlock;
  audio_block_t *fmBlock;
  audio_block_t *attackBlock;
  audio_block_t *releaseBlock;
  audio_block_t *shapeBlock;

  // Octave block
  octaveBlock = receiveReadOnly(0);

  if (octaveBlock) {
    this->octave = (float)((octaveBlock->data[0] + 32668) / 65335.0) * 6;  // 6 octaves
    release(octaveBlock);
  }

  // Setting the frequency
  this->frequency = this->frequency * this->glide + this->frequencyTarget * (1.0 - this->glide);
  this->sine->frequency(this->frequency);
  this->sawtooth->frequency(this->frequency);

  // FM block
  fmBlock = receiveReadOnly(1);
  if (fmBlock) {
    // This is connected to the sine object via an AudioConnection
    this->fmQueue->play(fmBlock->data, AUDIO_BLOCK_SAMPLES);
    release(fmBlock);
  }

  // Attack Block
  attackBlock = receiveReadOnly(2);
  if (attackBlock) {
    float attack = ((float)(attackBlock->data[0] + 32768) / 65335) * 2000 - 3;
    if (attack < 5) {
      attack = 5;
    }
    this->envelope->attack(attack);
    release(attackBlock);
  }

  // Release Block
  releaseBlock = receiveReadOnly(3);
  if (releaseBlock) {
    this->envelope->release(((float)(releaseBlock->data[0] + 32768) / 65335) * 2000);
    release(releaseBlock);
  }

  // Shape block
  shapeBlock = receiveReadOnly(4);
  if (shapeBlock) {
    // This is connected to the shapeCrossfader object via an AudioConnection
    this->shapeQueue->play(shapeBlock->data, AUDIO_BLOCK_SAMPLES);
    release(shapeBlock);
  }

  // // Gate block
  // gateBlock = receiveReadOnly(5);
  // if (gateBlock) {
  //   this->gateQueue->play(gateBlock->data, AUDIO_BLOCK_SAMPLES);
  //   release(gateBlock);
  // }

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
inline void Voice::noteOn(byte midiNote) {
  this->isLastPlayedMidi = true;
  this->setNote(midiNote);
  this->noteOn();
}

inline void Voice::noteOn() {
  this->isLastPlayedMidi = false;
  this->lastPlayed = millis();
  this->envelope->noteOn();
}

/**
 * Note off
 */
inline void Voice::noteOff() {
  this->envelope->noteOff();
}

inline void Voice::setNote(byte midiNote) {
  this->note = midiNote;
  this->frequencyTarget = 440.0 * powf(2.0, (float)(((this->note - 12) + 12 * this->octave) - 69) * 0.08333333);  // 440Hz = midi 69
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
inline void Voice::setFrequencyTarget(int frequencyTarget) {
  this->frequencyTarget = frequencyTarget;
}

/**
 * Set the octave
 */
inline void Voice::setOctave(byte octave) {
  this->octave = octave;
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

/**
 * Get the voice's current note
 */
inline byte Voice::getNote() {
  return this->note;
}

#endif
