#ifndef Synth_h
#define Synth_h

#include <Audio.h>

#include "Motherboard/Motherboard.h"
#include "Voice.h"

// Number of voices
const byte voiceCount = 16; // max = 16


/*
 * Synth
 */
class Synth{
  
  private:
    // Singleton
    static Synth *instance;
    Synth();
    
    Voice *voices[voiceCount];
    byte actualVoiceCount = 1;
    byte activeVoices = 0;
    byte mode;
    byte octave;
    byte portamento = 255;
//    unsigned int arpTime;
//    elapsedMillis elapsedTime;
//    byte arpIndex;
//    byte arpNotesPlaying;
//    byte arpNotes[voiceCount];
    unsigned int attack;
    unsigned int release;
    int modulatorFrequency;
    float modulatorAmplitude;
    float shape;
    byte updateMillis = 10;
    elapsedMillis clockUpdate;

    byte cvNoteIndex = 0;
    
    byte modeInputIndex = 0;
    byte parameterInputIndex = 1;
    byte shapeInputIndex = 2;
    byte fmInputIndex = 3;
    byte attackInputIndex = 4;
    byte releaseInputIndex = 5;

    InputPotentiometer* inputMode;
    InputPotentiometer* inputOctave;
    InputPotentiometer* inputShape;
    InputPotentiometer* inputLevel;
    InputPotentiometer* inputAttack;
    InputPotentiometer* inputRelease;
    InputJack* inputNote;
    InputJack* inputGate;
    Led* led1;
    Led* led2;
    AudioConnection* patchCords[voiceCount/4 + voiceCount]; 
    AudioMixer4 *mixers[voiceCount/2];
    AudioMixer4 *output;
    
  public:
    static Synth *getInstance();
    void init();
    void update();
    static void onVelocityChange(IO* io);
    static void noteOn(byte channel, byte note, byte velocity);
    static void noteOff(byte channel, byte note, byte velocity);
    static void stop();
    AudioMixer4 * getOutput();
    
    // Callbacks
    static void onModeChange(IO* io);
    static void onParamChange(IO* io);
    static void onShapeChange(IO* io);
    static void onFmChange(IO* io);
    static void onAttackChange(IO* io);
    static void onReleaseChange(IO* io);
    static void onNoteChange(IO* io);
//    static void onGateOpen(byte inputIndex);
//    static void onGateClose(byte inputIndex);
    
    // Midi callbacks
//    static void onMidiModeChange(byte channel, byte control, byte value);
//    static void onMidiParamChange(byte channel, byte control, byte value);
//    static void onMidiShapeChange(byte channel, byte control, byte value);
//    static void onMidiFmChange(byte channel, byte control, byte value);
//    static void onMidiAttackChange(byte channel, byte control, byte value);
//    static void onMidiReleaseChange(byte channel, byte control, byte value);
};

// Singleton pre init
Synth * Synth::instance = nullptr;

/**
 * Constructor
 */
inline Synth::Synth(){
  this->attack = 10;
  this->release = 2000;

  this->output = new AudioMixer4();
  this->output->gain(0, 1 );
  this->output->gain(1, 1 );
  this->output->gain(2, 1 );
  this->output->gain(3, 1 );

  for (int i = 0; i < voiceCount/4; i++) {
    this->mixers[i] = new AudioMixer4();
    this->mixers[i]->gain(0, 0.6 );
    this->mixers[i]->gain(1, 0.6 );
    this->mixers[i]->gain(2, 0.6 );
    this->mixers[i]->gain(3, 0.6 );
    
    this->patchCords[i] = new AudioConnection(*this->mixers[i], 0, *this->output, i%4);
  }
  
  for (int i = 0; i < voiceCount; i++) {
    this->voices[i] = new Voice();
    this->voices[i]->setAR(this->attack, this->release);
    this->voices[i]->setUpdateMillis(this->updateMillis);
    this->patchCords[i] = new AudioConnection(*this->voices[i]->getOutput(), 0, *this->mixers[i/4], i%4);
  }
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
inline void Synth::init(){

  // Mode
  this->inputMode = new InputPotentiometer(0, "Mode");
  this->inputMode->setOnChange(onModeChange);

  // Octave
  this->inputOctave = new InputPotentiometer(1, "Octave");
  this->inputOctave->setOnChange(onParamChange);
  this->inputOctave->setSmoothing(1);

  // Shape
  this->inputShape = new InputPotentiometer(2, "Shape");
  this->inputShape->setOnChange(onShapeChange);

  // Level
  this->inputLevel = new InputPotentiometer(3, "Level");
  this->inputLevel->setOnChange(onFmChange);

  // Attack
  this->inputAttack = new InputPotentiometer(4, "Attack");
  this->inputAttack->setOnChange(onAttackChange);

  // Release
  this->inputRelease = new InputPotentiometer(5, "Release");
  this->inputRelease->setOnChange(onReleaseChange);
  
  // Note
  this->inputNote = new InputJack(6, "Note");
  this->inputNote->setType("Quantized");
  this->inputNote->setOnChange(onNoteChange);
//  this->inputNote->setSmoothing(1);

  // Gate
  this->inputGate = new InputJack(7, "Gate");
//  this->inputGate->setSmoothing(1);
  this->inputGate->setType("Gate");
////  in8->setOnGateOpen(onGateOpen);
////  in8->setOnGateClose(onGateClose);
//  in8->setOnChange(onVelocityChange);
//  in8->setSmoothing(0);

  this->led1 = new Led(0);
  this->led2 = new Led(0);

  InputMidiNote* inputMidiNote = new InputMidiNote("inputMidiNote1", 1);
  inputMidiNote->setHandleMidiNoteOn(noteOn);
  inputMidiNote->setHandleMidiNoteOff(noteOff);
  
  Motherboard.setDebug(true);
  Motherboard.init("Synth", 2);
}

inline void Synth::onNoteChange(IO* io){
  // In mono, sets the note of the 1 voice
  // In poly, triggers new notes when Gate is open, shut off notes when receiving an already playing note when Gate still open  
//  Serial.println(((int)((io->getValue()/4096) * (4096/5/12) * 100)) / 100);
}


inline void Synth::onVelocityChange(IO* io){
    float gainValue = map(io->getValue(), 0, 4095, 0, 1);
    getInstance()->voices[0]->getOutput()->gain(1, gainValue);
}

//inline void Synth::onGateOpen(byte inputIndex){
// In mono, triggers the enveloppe of the 1 voice
// In poly, does nothing
//}

//inline void Synth::onGateClose(byte inputIndex){
// In mono, close the enveloppe of the 1 voice
// In poly, close the enveloppe of all voices
//}

/**
 * Note on
 */
inline void Synth::noteOn(byte channel, byte note, byte velocity){
  bool foundOne = false;
  int oldestVoice = 0;
  unsigned long oldestVoiceTime = 0;
  int closestVoice = 0;
  unsigned long closestVoiceNote = sizeof(unsigned long);
  
//  switch (modes(getInstance()->mode)){
//    case SYNTH:
      if(getInstance()->portamento == 0){
        for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
          // If no portamento we get an inactive voice to play the new note
          // or else the oldest voice
          
          // Search for the oldest voice
          // TODO: should probably be < because last_played = millis() which means this number is bigger for newer notes
          if(getInstance()->voices[i]->last_played > oldestVoiceTime){ 
            oldestVoiceTime = getInstance()->voices[i]->last_played;
            oldestVoice = i;
          }
          
          // Search for an inactive voice
          if(!getInstance()->voices[i]->isActive()){
            getInstance()->voices[i]->noteOn(note);
            foundOne = true;
            break;
          }
        }
        
        // No inactive voice then will take over the oldest note
        if(!foundOne){
          getInstance()->voices[oldestVoice]->noteOn(note);
        }
      }else{
        // If there is portamento then we find the voice with the closest note
        // to play the new note, so that portamento is applied to it and it sounds better in polyphony

        // Search first for a voice that is not being played and not active and not the same note as the new note
        for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
          if(!getInstance()->voices[i]->isNotePlayed() &&
             !getInstance()->voices[i]->isActive() &&
            abs(getInstance()->voices[i]->getCurrentNote() - note) < closestVoiceNote &&
            getInstance()->voices[i]->getCurrentNote() != note){
            closestVoiceNote = getInstance()->voices[i]->getCurrentNote();
            closestVoice = i;
            foundOne = true;
            break;
          }
        }

        // If none is found then search for a voice that is just not being played and not the same note
        if(!foundOne){
          for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
            if(!getInstance()->voices[i]->isNotePlayed() &&
              abs(getInstance()->voices[i]->getCurrentNote() - note) < closestVoiceNote &&
              getInstance()->voices[i]->getCurrentNote() != note){
              closestVoiceNote = getInstance()->voices[i]->getCurrentNote();
              closestVoice = i;
              foundOne = true;
            }
          }
        }

        // Make sure that the voice selected will not start with a note that is too far to avoid bad sounding portamento
        if(getInstance()->voices[closestVoice]->getCurrentNote() - note > 12){
          getInstance()->voices[closestVoice]->setCurrentNote(note + 12);
        }else if(getInstance()->voices[closestVoice]->getCurrentNote() - note < -12){
          getInstance()->voices[closestVoice]->setCurrentNote(note - 12);
        }
        
        getInstance()->voices[closestVoice]->noteOn(note);
      }

  getInstance()->activeVoices++;
  getInstance()->led1->set(Led::Status::On, 200*getInstance()->activeVoices);
}

/**
 * Note off
 */
inline void Synth::noteOff(byte channel, byte note, byte velocity){
//  switch(modes(getInstance()->mode)){
//    case SYNTH: 
      for (int i = 0; i < voiceCount ; i++) {
        if(getInstance()->voices[i]->getCurrentNote() == note){
          getInstance()->voices[i]->noteOff();
        }
      }
//    break;
//    case ARP:
//      for (int i = 0; i < voiceCount ; i++) {
//        // Finding the index where the note was in the array
//        if(getInstance()->arpNotes[i] == note){
//          getInstance()->voices[i]->noteOff();
//          // Shifting the elemts after this index
//          for (int j = i; j < voiceCount; ++j){
//            getInstance()->arpNotes[j] = getInstance()->arpNotes[j + 1];
//          }
//        }
//      }
//
//      // Decreasing the number of playing notes
//      if(getInstance()->arpNotesPlaying > 0){
//        getInstance()->arpNotesPlaying--;
//      }
//    break;

//    default:
//    break;
//  }
  
//  Motherboard.setLED(0, MotherboardNamespace::Led::Status::Off);
  getInstance()->activeVoices--;
  if(getInstance()->activeVoices == 0){
    getInstance()->led1->set(Led::Status::Off, 0);
  }
}

/**
 * Stop all the voices
 */
inline void Synth::stop(){
  for (int i = 0; i < voiceCount; i++) {
    getInstance()->voices[i]->noteOff();
  }
  
//  Motherboard.setLED(0, MotherboardNamespace::Led::Status::Off);
  getInstance()->led1->set(Led::Status::Off, 0);
}

/**
 * Return the audio output
 */
inline AudioMixer4 * Synth::getOutput(){
  return this->output;
}


/**
 * Update
 */
inline void Synth::update(){
  Motherboard.update();
  
  if(this->clockUpdate > updateMillis){

    for (byte i = 0; i < voiceCount ; i++) {
      this->voices[i]->update();
    }
    
    this->clockUpdate = 0;
  }

  
  // Arp
//  if(modes(this->mode) == ARP){
//    if (this->elapsedTime >= this->arpTime) {
//
//      if(this->arpNotesPlaying > 0){
//        if(this->arpIndex == 0){
//          if(this->voices[(this->arpNotesPlaying-1)]->isNotePlayed()){
//            this->voices[(this->arpNotesPlaying-1)]->noteOff();
//          }
//        }else{
//          if(this->voices[(this->arpIndex-1)]->isNotePlayed()){
//            this->voices[(this->arpIndex-1)]->noteOff();
//          }
//        }
//        this->voices[this->arpIndex]->setGlide(255);
//        this->voices[this->arpIndex]->noteOn(this->arpNotes[this->arpIndex]);
//      }else{
//        for (byte i = 0; i < voiceCount ; i++) {
//          getInstance()->voices[i]->noteOff();
//        }
//      }
//        
//      this->arpIndex++;
//      if(this->arpIndex > this->arpNotesPlaying-1 ){
//        this->arpIndex = 0;
//      }
//      
//      this->elapsedTime = 0;
//    }
//  }
}


/**
 * On Mode Change
 */
inline void Synth::onModeChange(IO* io){
  float monoPoly = map(io->getValue(), 0, 4096, 0, 2);
  
  // Mono and Poly modes
  if(monoPoly >= 1){
    getInstance()->actualVoiceCount = 16;
  }else{
    getInstance()->actualVoiceCount = 1;
  }
  
  // Portamento
  getInstance()->portamento = ((unsigned int)constrain(map(io->getValue(), 0, 4095, 511, 0), 0, 511)) % 256;

  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setGlide(getInstance()->portamento);
  }
}

/**
 * On MIDI Mode Change
 */
//void Synth::onMidiModeChange(byte channel, byte control, byte value){
//  unsigned int mapValue = map(
//    value, 
//    0,
//    127,
//    
//    0, 
//    4095
//  );
//  
//  getInstance()->onModeChange(0, mapValue, 255);
//}

/**
 * On Param Change
 */
inline void Synth::onParamChange(IO* io){  
  getInstance()->octave = map(io->getValue(), 0, 4095, 0, 8);

  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setOctave(getInstance()->octave);
  }
}

/**
 * On MIDI Param Change
 */
//void Synth::onMidiParamChange(byte channel, byte control, byte value){
//  unsigned int mapValue = map(
//    value, 
//    0,
//    127,
//    0, 
//    4095
//  );
//  
//  Motherboard.setPotentiometer(1, mapValue);
//}

/**
 * On Shape Change
 */
inline void Synth::onShapeChange(IO* io){
  // Shape
  float shape = (float)map((float)io->getValue(), 0, 4095, 0, 1);
    
  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setShape(shape);
  }
}

/**
 * On MIDI Shape Change
 */
//void Synth::onMidiShapeChange(byte channel, byte control, byte value){
//  unsigned int mapValue = map(
//    value, 
//    0,
//    127,
//    0, 
//    4095
//  );
//  
//  Motherboard.setPotentiometer(2, mapValue);
//}


/**
 * On FM Change
 */
inline void Synth::onFmChange(IO* io){
//  float level = map(io->getValue(), 0, 4095, 0, 1);
//  getInstance()->output->gain(0, level );
//  getInstance()->output->gain(1, level );
//  getInstance()->output->gain(2, level );
//  getInstance()->output->gain(3, level );

//  int modulatorFrequency = 0;
//  float modulatorAmplitude = 0;
//
//  if(value < 4095 / 3){
//    modulatorFrequency = map(value, 0, 4095 / 2, 0, 10);
//    modulatorAmplitude = (float)map(value, 0, 4095 / 2, 0.001, .01);
//  }
//  else if(value >= 4095 / 3 && value < 4095 / 2){
//    modulatorFrequency = map(value, 0, 4095 / 2, 0, 40);
//    modulatorAmplitude = (float)map(value, 0, 4095 / 2, 0.001, .01);
//  }else{
//    modulatorFrequency = map(value, 4095 / 2, 4095, 0, 1000);
//    modulatorAmplitude = (float)map(value, 4095 / 2, 4095, 0, .5);
//  }
//  
//  if(getInstance()->modulatorFrequency != modulatorFrequency){
//    getInstance()->modulatorFrequency = modulatorFrequency;
//    for (int i = 0; i < voiceCount ; i++) {
//      getInstance()->voices[i]->setModulatorFrequency(modulatorFrequency);
//    }
//  }
//  
//  if(value > 50){
//    if(getInstance()->modulatorAmplitude != modulatorAmplitude){
//      getInstance()->modulatorAmplitude = modulatorAmplitude;
//      for (int i = 0; i < voiceCount ; i++) {
//        getInstance()->voices[i]->setModulatorAmplitude(modulatorAmplitude);
//      }
//    }
//  }else{
//    for (int i = 0; i < voiceCount ; i++) {
//      getInstance()->voices[i]->setModulatorAmplitude(0);
//    }
//  }
}

/**
 * On MIDI FM Change
 */
//void Synth::onMidiFmChange(byte channel, byte control, byte value){
//  unsigned int mapValue = map(
//    value, 
//    0,
//    127,
//    0, 
//    4095
//  );
//  
//  Motherboard.setPotentiometer(3, mapValue);
//}

/**
 * On Attack Change
 */
inline void Synth::onAttackChange(IO* io){
  unsigned int attack = map(io->getValue(), 0, 4095, 0, 2000);
 
  getInstance()->attack = attack;
  
  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setAttack(attack);
  }
}

/**
 * On MIDI Attack Change
 */
//void Synth::onMidiAttackChange(byte channel, byte control, byte value){
//  unsigned int mapValue = map(
//    value, 
//    0,
//    127,
//    0, 
//    4095
//  );
//  
//  Motherboard.setPotentiometer(4, mapValue);
//}

/**
 * On Release Change
 */
inline void Synth::onReleaseChange(IO* io){
  unsigned int release = map(io->getValue(), 0, 4095, 0, 2000);

  getInstance()->release = release;
  
  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setRelease(release);
  }
}

/**
 * On MIDI Release Change
 */
//void Synth::onMidiReleaseChange(byte channel, byte control, byte value){
//  unsigned int mapValue = map(
//    value, 
//    0,
//    127,
//    0, 
//    4095
//  );
//  
//  Motherboard.setPotentiometer(5, mapValue);
//}

#endif
