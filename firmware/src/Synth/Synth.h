#ifndef Synth_h
#define Synth_h

#include <Audio.h>

#include "Motherboard/Motherboard.h"
#include "Voice.h"
#include "Mixer.h"
#include "Combine.h"

// Number of voices
const byte voiceCount = 20;

elapsedMillis timePassed;

/*
 * Synth
 */
class Synth {

private:
  // Singleton
  static Synth *instance;
  Synth();

  Voice *voices[voiceCount];
  byte actualVoiceCount = voiceCount;
  byte octaveNumber;
  bool isGateOpen = false;
  float portamento = 0;
  int16_t voctNote = 0;
  Voice* polySelectedVoice = nullptr;
  byte currentVoiceIndex = 0;

  // Inputs
  Input *monoPolyGlide;
  Input *octave;
  Input *shape;
  Input *spread;
  Input *attack;
  Input *release;
  InputADS1100 *voct;
  // InputQuantized *voct;
  InputGate *gate;
  Input *attackMod;
  Input *releaseMod;
  Input *shapeMod;
  Input *fm;

  // Leds
  OutputLed *led1;
  OutputLed *led2;
  OutputLed *led3;
  OutputLed *led4;
  OutputLed *led5;
  OutputLed *led6;
  OutputLed *led7;

  Combine<2> *octaveCombine;
  Combine<3> *shapeCombine;
  Combine<2> *spreadCombine;
  Combine<3> *attackCombine;
  Combine<3> *releaseCombine;

  // Delay for monophonic mode stereo
  AudioEffectDelay *delay;

  AudioMixer<voiceCount> *outputL;
  AudioMixer<voiceCount> *outputR;

  static void handleMonoPolyGlideChange(int16_t value);
  static void handleSpreadChange(int16_t value);
  static void handleOctaveChange(int16_t value);
  static void handleOnGateOpen();
  static void handleOnGateClose();
  static void handleOnVOctCHange(int16_t note);
  static void handleMidiNoteOn(byte note, byte velocity);
  static void handleMidiNoteOff(byte note);
  static void handleMidiReleaseChange(byte value);

  // Find an appropriate voice to play a note
  Voice *getVoiceToPlay(unsigned int note);
  Voice *getNextVoice();
  // Voice* getVoiceWithNote(unsigned int note);

  // MIDI
  MidiNotesInput *midiNotesInput;
  MidiCCInput *octaveMidiCCInput;
  MidiCCInput *shapeMidiCCInput;
  MidiCCInput *spreadMidiCCInput;
  MidiCCInput *attackMidiCCInput;
  MidiCCInput *releaseMidiCCInput;

public:
  static Synth *getInstance();
  void init();
  void noteOn(byte note, byte velocity);
  void noteOff(byte note, byte velocity);
  AudioStream *getOutputL();
  AudioStream *getOutputR();
};

// Singleton pre init
Synth *Synth::instance = nullptr;

/**
 * Constructor
 */
inline Synth::Synth() {
}

/**
 * Singleton instance
 */
inline Synth *Synth::getInstance() {
  if (!instance)
    instance = new Synth;
  return instance;
}

/**
 * Init
 */
inline void Synth::init() {
  this->led1 = new OutputLed(0);
  this->led1->setStatus(OutputLed::Status::Off);
  this->led2 = new OutputLed(1);
  this->led2->setStatus(OutputLed::Status::Off);
  this->led3 = new OutputLed(2);
  this->led4 = new OutputLed(3);
  this->led5 = new OutputLed(4);
  this->led6 = new OutputLed(5);
  this->led7 = new OutputLed(6);
  this->led7->setStatus(OutputLed::Status::Off);

  this->monoPolyGlide = new Input(0);
  this->monoPolyGlide->onChange(handleMonoPolyGlideChange);
  this->octave = new Input(1);
  this->shape = new Input(2);
  this->spread = new Input(3);
  this->spread->setLowPassCoeff(0.0001);
  this->attack = new Input(4);
  this->release = new Input(5);
  this->voct = new InputADS1100(6);
  this->voct->onChange(handleOnVOctCHange);
  this->gate = new InputGate(7);
  this->gate->onOpen(handleOnGateOpen);
  this->gate->onClose(handleOnGateClose);
  this->attackMod = new Input(8);
  // this->attackMod->setLowPassCoeff(0.2);
  this->releaseMod = new Input(9);
  // this->releaseMod->setLowPassCoeff(0.2);
  this->shapeMod = new Input(10);
  // this->shapeMod->setLowPassCoeff(0.2);
  this->fm = new Input(11);
  this->fm->setLowPassCoeff(0.2);


  // To combine signals positively
  this->octaveCombine = new Combine<2>();
  this->octaveCombine->onChange(handleOctaveChange);
  this->shapeCombine = new Combine<3>();
  this->spreadCombine = new Combine<2>();
  this->spreadCombine->onChange(handleSpreadChange);
  this->attackCombine = new Combine<3>();
  this->releaseCombine = new Combine<3>();

  // MIDI
  this->midiNotesInput = new MidiNotesInput();
  this->midiNotesInput->onNoteOn(handleMidiNoteOn);
  this->midiNotesInput->onNoteOff(handleMidiNoteOff);

  // TODO: Edit CC numbers
  // TODO: Add Settings
  this->octaveMidiCCInput = new MidiCCInput(1);
  this->shapeMidiCCInput = new MidiCCInput(2);
  this->spreadMidiCCInput = new MidiCCInput(3);
  this->releaseMidiCCInput = new MidiCCInput(4);
  this->attackMidiCCInput = new MidiCCInput(5);

  this->delay = new AudioEffectDelay();
  this->delay->delay(0, 0);

  this->outputL = new AudioMixer<voiceCount>;
  this->outputL->gain(0.2 / voiceCount);
  this->outputR = new AudioMixer<voiceCount>;
  this->outputR->gain(0.2 / voiceCount);

  for (int i = 0; i < voiceCount; i++) {
    this->voices[i] = new Voice();
    new AudioConnection(*this->octave, 0, *this->octaveCombine, 0);
    new AudioConnection(*this->octaveMidiCCInput, 0, *this->octaveCombine, 1);
    new AudioConnection(*this->octaveCombine, 0, *this->voices[i], 0);
    new AudioConnection(*this->fm, 0, *this->voices[i], 1);
    new AudioConnection(*this->fm, 0, *this->led6, 0);
    new AudioConnection(*this->attack, 0, *this->attackCombine, 0);
    new AudioConnection(*this->attackMod, 0, *this->attackCombine, 1);
    new AudioConnection(*this->attackMidiCCInput, 0, *this->attackCombine, 2);
    new AudioConnection(*this->attackCombine, 0, *this->voices[i], 2);
    new AudioConnection(*this->attackCombine, 0, *this->led3, 0);
    new AudioConnection(*this->release, 0, *this->releaseCombine, 0);
    new AudioConnection(*this->releaseMod, 0, *this->releaseCombine, 1);
    new AudioConnection(*this->releaseMidiCCInput, 0, *this->releaseCombine, 2);
    new AudioConnection(*this->releaseCombine, 0, *this->voices[i], 3);
    new AudioConnection(*this->releaseCombine, 0, *this->led4, 0);
    new AudioConnection(*this->shape, 0, *this->shapeCombine, 0);
    new AudioConnection(*this->shapeMod, 0, *this->shapeCombine, 1);
    new AudioConnection(*this->shapeMidiCCInput, 0, *this->shapeCombine, 2);
    new AudioConnection(*this->shapeCombine, 0, *this->voices[i], 4);
    new AudioConnection(*this->shapeCombine, 0, *this->led5, 0);
    new AudioConnection(*this->spread, 0, *this->spreadCombine, 0);
    new AudioConnection(*this->spreadMidiCCInput, 0, *this->spreadCombine, 1);
    new AudioConnection(*this->voices[i], 0, *this->outputL, i);
    new AudioConnection(*this->voices[i], 0, *this->outputR, i);
    new AudioConnection(*this->outputR, 0, *this->delay, 0);
  }
}

/**
 * Note on
 */
inline void Synth::noteOn(byte note, byte velocity) {
  Voice *voice = getInstance()->getVoiceToPlay(note);
  voice->noteOn(note);
}

/**
 * Note off
 */
inline void Synth::noteOff(byte note, byte velocity) {
  for (int i = 0; i < voiceCount; i++) {
    if (getInstance()->voices[i]->getNote() == note) {
      getInstance()->voices[i]->noteOff();
    }
  }
}

/**
 * Find a voice to play speficied note
 * TODO: En polyphonie, selectionner voix qui a la note la plus proche
 */
inline Voice *Synth::getVoiceToPlay(unsigned int note) {
  // Monophony
  if (getInstance()->actualVoiceCount == 1) {
    return getInstance()->voices[0];
  }

  int oldestVoice = 0;
  unsigned long oldestVoiceTime = sizeof(unsigned long);

  if (getInstance()->portamento == 0) {

    // 1. Search for an inactive voice

    for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
      if (i == 0 && getInstance()->voices[i]->isActive()) {
      }
      if (!getInstance()->voices[i]->isActive()) {
        return getInstance()->voices[i];
      }
    }

    // 2. Search for the oldest voice playing the same note

    bool foundOne = false;

    for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
      if (getInstance()->voices[i]->getNote() == note && getInstance()->voices[i]->lastPlayed < oldestVoiceTime) {
        oldestVoiceTime = getInstance()->voices[i]->lastPlayed;
        oldestVoice = i;

        foundOne = true;
      }
    }

    if (foundOne) {
      return getInstance()->voices[oldestVoice];
    }

    // 3. Search for the oldest voice no matter its note

    oldestVoice = 0;
    oldestVoiceTime = ULONG_MAX;

    for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
      if (getInstance()->voices[i]->lastPlayed < oldestVoiceTime) {
        oldestVoiceTime = getInstance()->voices[i]->lastPlayed;
        oldestVoice = i;
      }
    }

    return getInstance()->voices[oldestVoice];
  } else {
    // If there is portamento then we find the voice with the closest note
    // to play the new note, so that portamento is applied to it and it sounds better in polyphony

    bool foundOne = false;
    int closestVoice = 0;
    unsigned long closestVoiceNote = sizeof(unsigned long);

    // 1. Is there a voice inactive?
    for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
      if (!getInstance()->voices[i]->isActive()) {

        // Set all other inactive voices to that same note
        for (int j = 0; j < getInstance()->actualVoiceCount; j++) {
          if (i != j && !getInstance()->voices[j]->isActive()) {
            getInstance()->voices[j]->setNote(note);
          }
        }

        return getInstance()->voices[i];
      }
    }

    // 2. Is there an active voice playing the same note?
    for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
      if (getInstance()->voices[i]->getNote() == note) {

        // Set all other inactive voices to that same note
        for (int j = 0; j < getInstance()->actualVoiceCount; j++) {
          if (i != j && !getInstance()->voices[j]->isActive()) {
            getInstance()->voices[j]->setNote(note);
          }
        }

        return getInstance()->voices[i];
      }
    }

    // Search first for a voice that is not being played and not active and not the same note as the new note
    // for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
    //   if (!getInstance()->voices[i]->isActive() && abs((int)getInstance()->voices[i]->getNote() - (int)note) < closestVoiceNote && getInstance()->voices[i]->getNote() != note) {
    //     closestVoiceNote = getInstance()->voices[i]->getNote();
    //     closestVoice = i;
    //     foundOne = true;
    //     // break;
    //   }
    // }

    // If none is found then search for a voice that is not in sustain, so it will be active (in release phase) and not the same note
    // if (!foundOne) {
    for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
      if (!getInstance()->voices[i]->isSustain() && abs((int)getInstance()->voices[i]->getNote() - (int)note) < closestVoiceNote && getInstance()->voices[i]->getNote() != note) {
        closestVoiceNote = getInstance()->voices[i]->getNote();
        closestVoice = i;
        foundOne = true;
      }
    }
    // }

    if (foundOne) {
      return getInstance()->voices[closestVoice];
    }


    // Search for the oldest voice
    for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
      if (getInstance()->voices[i]->lastPlayed < oldestVoiceTime) {
        oldestVoiceTime = getInstance()->voices[i]->lastPlayed;
        oldestVoice = i;
      }
    }

    return getInstance()->voices[oldestVoice];
  }
}

/**
 * With Voct and Gate, just get a new voice every time.
 * With MIDI we can use a more sophisticated methods because MIDI notes are exact, as opposed to frequencies.
 */
inline Voice *Synth::getNextVoice() {
    currentVoiceIndex++;
    if(currentVoiceIndex >= actualVoiceCount){
      currentVoiceIndex = 0;
    }
    return getInstance()->voices[currentVoiceIndex];
}

// inline Voice* Synth::getVoiceWithNote(unsigned int note){
//   for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
//     if(getInstance()->voices[i]->getNote() == note){
//       return getInstance()->voices[i];
//     }
//   }

//   return nullptr;
// }

/**
 * Return the audio output
 */
inline AudioStream *Synth::getOutputL() {
  return this->outputL;
}

inline AudioStream *Synth::getOutputR() {
  return this->delay;
  // return this->outputR;
}

//TODO: Fix cracking when delay changes
void Synth::handleSpreadChange(int16_t value) {
  if (getInstance()->actualVoiceCount == 1) {
    uint8_t val = map(value, INT16_MIN, INT16_MAX, 0, 50);
    getInstance()->delay->delay(0, val);
    getInstance()->outputL->gain(0.2 / voiceCount);
    getInstance()->outputR->gain(0.2 / voiceCount);
  } else {
    getInstance()->delay->delay(0, 0);
    float val = map((float)value, INT16_MIN, INT16_MAX, 1.0, 3.0);

    getInstance()->outputL->gain(0.2 / voiceCount);
    getInstance()->outputR->gain(0.2 / voiceCount);
    for (int i = 0; i < voiceCount; i++) {
      if (i % 2 == 0) {
        getInstance()->outputL->gain(i, 0.2 / (voiceCount * val));
      } else {
        getInstance()->outputR->gain(i, 0.2 / (voiceCount * val));
      }
    }
  }
}

inline void Synth::handleOctaveChange(int16_t value){
  getInstance()->octaveNumber = (float)((value + 32668) / 65335.0) * 6;  // 6 octaves
}

inline void Synth::handleMonoPolyGlideChange(int16_t value) {
  // This delay seem required otherwise Synth does not boot and keeps crashing
  if (timePassed < 3000) {
    return;
  }

  float val = map((float)value, INT16_MIN, INT16_MAX, 0.0, 2.0);
  float valInverted = map((float)value, INT16_MIN, INT16_MAX, 2.0, 0.0);

  if (val >= 1) {
    // Giving more room to be able to stick to no portamento better
    if (valInverted <= 0.1) {
      valInverted = 0;
    }

    getInstance()->portamento = valInverted;
  } else {
    getInstance()->portamento = val;
  }

  // Mono and Poly modes
  if (val >= 1) {
    getInstance()->actualVoiceCount = voiceCount;
    // getInstance()->portamento -= 1;
  } else {
    getInstance()->actualVoiceCount = 1;
  }

  float porta = (log10(getInstance()->portamento + 0.0001) + 3) / 3.0;

  for (int i = 0; i < voiceCount; i++) {
    getInstance()->voices[i]->setGlide(porta);
  }
}

inline void Synth::handleOnGateOpen() {
  // This is to avoid Gate triggering when powering on the device
  if (timePassed < 1000) {
    return;
  }

  getInstance()->isGateOpen = true;

  if (getInstance()->actualVoiceCount > 1) {
    getInstance()->polySelectedVoice = getInstance()->getNextVoice();

    float f = 440.0 * powf(2.0, (float)(((getInstance()->voctNote - 12) + 12 * getInstance()->octaveNumber) - 69) * 0.08333333);
    getInstance()->polySelectedVoice->setFrequencyTarget(f);
    getInstance()->polySelectedVoice->noteOn();
  }else{
    getInstance()->voices[0]->noteOn();
  }

  getInstance()->led2->setStatus(OutputLed::Status::On);
}

inline void Synth::handleOnGateClose() {
  if (timePassed < 1000) {
    return;
  }
  
  getInstance()->isGateOpen = false;

  for (int i = 0; i < voiceCount; i++) {
    getInstance()->voices[i]->noteOff();
  }

  getInstance()->led2->setStatus(OutputLed::Status::Off);
}

inline void Synth::handleOnVOctCHange(int16_t value) {
  if (timePassed < 1000) {
    return;
  }

  float note = ((float)(value + 32768) / 65535) * 60;
  float f = 440.0 * powf(2.0, (float)(((note - 12) + 12 * getInstance()->octaveNumber) - 69) * 0.08333333);
  getInstance()->voctNote = note;

  if (getInstance()->actualVoiceCount > 1) {
  //   // getInstance()->polySelectedVoice->setNote(note);
    if(getInstance()->isGateOpen){
      if(getInstance()->polySelectedVoice != nullptr){
        getInstance()->polySelectedVoice->setFrequencyTarget(f);
      }
    }
  
  }else{
      getInstance()->voices[0]->setFrequencyTarget(f);
  }
  getInstance()->led1->setStatus(OutputLed::Status::BlinkOnce);
}

inline void Synth::handleMidiNoteOn(byte note, byte velocity) {
  if (timePassed < 1000) {
    return;
  }

  getInstance()->noteOn(note, velocity);
  getInstance()->led7->setStatus(OutputLed::Status::BlinkOnce);
}

inline void Synth::handleMidiNoteOff(byte note) {
  if (timePassed < 1000) {
    return;
  }

  getInstance()->noteOff(note, 0);
  getInstance()->led7->setStatus(OutputLed::Status::BlinkOnce);
}

#endif
