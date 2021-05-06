#ifndef Synth_h
#define Synth_h

#include <Audio.h>

#include "Motherboard.h"
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

    // Motherboard
    Motherboard *device;
    
    Voice *voices[voiceCount];
    byte actualVoiceCount = 1;
    byte synthesis;
    byte mode;
    int parameter;
    byte portamento = 255;
    unsigned int arpTime;
    elapsedMillis elapsedTime;
    byte arpIndex;
    byte arpNotesPlaying;
    byte arpNotes[voiceCount];
    unsigned int attack;
    unsigned int release;
    int modulatorFrequency;
    float modulatorAmplitude;
    float shape;
    byte updateMillis = 10;
    elapsedMillis clockUpdate;
    
    byte modeInputIndex = 0;
    byte parameterInputIndex = 1;
    byte shapeInputIndex = 2;
    byte fmInputIndex = 3;
    byte attackInputIndex = 4;
    byte releaseInputIndex = 5;

    AudioConnection* patchCords[voiceCount/4 + voiceCount]; 
    AudioMixer4 *mixers[voiceCount/2];
    AudioMixer4 *output;
    
  public:
    static Synth *getInstance();
    void init();
    void update();
    static void noteOn(byte channel, byte note, byte velocity);
    static void noteOff(byte channel, byte note, byte velocity);
    static void stop();
    AudioMixer4 * getOutput();
    
    // Callbacks
    static void onModeChange(byte inputIndex, float value, int diffToPrevious);
    static void onParamChange(byte inputIndex, float value, int diffToPrevious);
    static void onShapeChange(byte inputIndex, float value, int diffToPrevious);
    static void onFmChange(byte inputIndex, float value, int diffToPrevious);
    static void onAttackChange(byte inputIndex, float value, int diffToPrevious);
    static void onReleaseChange(byte inputIndex, float value, int diffToPrevious);
    // Midi callbacks
    static void onMidiModeChange(byte channel, byte control, byte value);
    static void onMidiParamChange(byte channel, byte control, byte value);
    static void onMidiShapeChange(byte channel, byte control, byte value);
    static void onMidiFmChange(byte channel, byte control, byte value);
    static void onMidiAttackChange(byte channel, byte control, byte value);
    static void onMidiReleaseChange(byte channel, byte control, byte value);
};

// Singleton pre init
Synth * Synth::instance = nullptr;

/**
 * Constructor
 */
inline Synth::Synth(){
  this->synthesis = 0;
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
inline Synth *Synth::getInstance()    {
  if (!instance)
     instance = new Synth;
  return instance;
}

/**
 * Init
 */
inline void Synth::init(){
  this->device = Motherboard::init(
    "Synth",
    {
      Potentiometer, Potentiometer,
      Potentiometer, Potentiometer,
      Potentiometer, Potentiometer
    }
  );

  // Device callbacks
  this->device->setHandlePotentiometerChange(0, onModeChange);
  this->device->setHandlePotentiometerChange(1, onParamChange);
  this->device->setHandlePotentiometerChange(2, onShapeChange);
  this->device->setHandlePotentiometerChange(3, onFmChange);
  this->device->setHandlePotentiometerChange(4, onAttackChange);
  this->device->setHandlePotentiometerChange(5, onReleaseChange);
  
  this->device->setHandleMidiNoteOn(noteOn);
  this->device->setHandleMidiNoteOff(noteOff);
  this->device->setHandleMidiControlChange(0, 0, "Mode",    onMidiModeChange);
  this->device->setHandleMidiControlChange(0, 1, "Param",   onMidiParamChange);
  this->device->setHandleMidiControlChange(0, 2, "Shape",   onMidiShapeChange);
  this->device->setHandleMidiControlChange(0, 3, "FM",      onMidiFmChange);
  this->device->setHandleMidiControlChange(0, 4, "Attack",  onMidiAttackChange);
  this->device->setHandleMidiControlChange(0, 5, "Release", onMidiReleaseChange);
}

/**
 * Note on
 */
inline void Synth::noteOn(byte channel, byte note, byte velocity){
  bool foundOne = false;
  int oldestVoice = 0;
  unsigned long oldestVoiceTime = 0;
  int closestVoice = 0;
  unsigned long closestVoiceNote = sizeof(unsigned long);
  
  switch (modes(getInstance()->mode)){
    case SYNTH: 
      if(getInstance()->portamento == 0){
        for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
          // If no portamento we get an inactive voice to play the new note
          // or else the oldest voice
          
          // Search for the oldest voice
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
    break;
    case ARP:
      if(getInstance()->arpNotesPlaying < voiceCount){
        getInstance()->arpNotesPlaying++;
      }
      getInstance()->arpNotes[getInstance()->arpNotesPlaying-1] = note;
    break;
    case DRONE:
      // In Drone mode, only one voice playing at a time
      for (int i = 0; i < voiceCount ; i++) {
        getInstance()->voices[i]->noteOff();
      }
      getInstance()->voices[0]->noteOn(note);
    break;
  }
  
  getInstance()->device->setLED(0, 1);
}

/**
 * Note off
 */
inline void Synth::noteOff(byte channel, byte note, byte velocity){
  switch(modes(getInstance()->mode)){
    case SYNTH: 
      for (int i = 0; i < voiceCount ; i++) {
        if(getInstance()->voices[i]->getCurrentNote() == note){
          getInstance()->voices[i]->noteOff();
        }
      }
    break;
    case ARP:
      for (int i = 0; i < voiceCount ; i++) {
        // Finding the index where the note was in the array
        if(getInstance()->arpNotes[i] == note){
          getInstance()->voices[i]->noteOff();
          // Shifting the elemts after this index
          for (int j = i; j < voiceCount; ++j){
            getInstance()->arpNotes[j] = getInstance()->arpNotes[j + 1];
          }
        }
      }

      // Decreasing the number of playing notes
      if(getInstance()->arpNotesPlaying > 0){
        getInstance()->arpNotesPlaying--;
      }
    break;

    default:
    break;
  }
  
  getInstance()->device->setLED(0, 0);
}

/**
 * Stop all the voices
 */
inline void Synth::stop(){
  for (int i = 0; i < voiceCount; i++) {
    getInstance()->voices[i]->noteOff();
  }
  
  getInstance()->device->setLED(0, 0);
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
  this->device->update();
  
  if(this->clockUpdate > updateMillis){
    
    for (byte i = 0; i < voiceCount ; i++) {
      this->voices[i]->update();
    }
    
    this->clockUpdate = 0;
  }

  
  // Arp
  if(modes(this->mode) == ARP){
    if (this->elapsedTime >= this->arpTime) {

      if(this->arpNotesPlaying > 0){
        if(this->arpIndex == 0){
          if(this->voices[(this->arpNotesPlaying-1)]->isNotePlayed()){
            this->voices[(this->arpNotesPlaying-1)]->noteOff();
          }
        }else{
          if(this->voices[(this->arpIndex-1)]->isNotePlayed()){
            this->voices[(this->arpIndex-1)]->noteOff();
          }
        }
        this->voices[this->arpIndex]->setGlide(255);
        this->voices[this->arpIndex]->noteOn(this->arpNotes[this->arpIndex]);
      }else{
        for (byte i = 0; i < voiceCount ; i++) {
          getInstance()->voices[i]->noteOff();
        }
      }
        
      this->arpIndex++;
      if(this->arpIndex > this->arpNotesPlaying-1 ){
        this->arpIndex = 0;
      }
      
      this->elapsedTime = 0;
    }
  }
}


/**
 * On Mode Change
 */
inline void Synth::onModeChange(byte inputIndex, float value, int diffToPrevious){
  byte mode = (byte)constrain(
    map(
      value,
      getInstance()->device->getAnalogMinValue(),
      getInstance()->device->getAnalogMaxValue() - 100, 
      0, 
      2
    ),
    0,
    2
  );

  bool monoPoly = map(
    (int)value,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue() / 3,
    0,
    1
  );
  
  // A little addition to the SYNTH mode
  // Mono and Poly modes
  if(monoPoly > 0){
    getInstance()->actualVoiceCount = 16;
  }else{
    getInstance()->actualVoiceCount = 1;
  }
  
  getInstance()->mode = mode;

  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setMode(mode);
    
    switch(mode){
      case ARP:
        getInstance()->voices[i]->setGlide(255);
        break;
      default:
        getInstance()->voices[i]->setGlide(getInstance()->portamento);
        break;
    }
  }

  if(modes(getInstance()->mode) == DRONE){
    getInstance()->mixers[0]->gain(0, 0.6 );
    getInstance()->mixers[0]->gain(1, 0 );
    getInstance()->mixers[0]->gain(2, 0 );
    getInstance()->mixers[0]->gain(3, 0 );
    getInstance()->output->gain(0, 1 );
    getInstance()->output->gain(1, 0 );
    getInstance()->output->gain(2, 0 );
    getInstance()->output->gain(3, 0 );
  }else{
    getInstance()->mixers[0]->gain(0, 0.6 );
    getInstance()->mixers[0]->gain(1, 0.6 );
    getInstance()->mixers[0]->gain(2, 0.6 );
    getInstance()->mixers[0]->gain(3, 0.6 );
    getInstance()->output->gain(0, 1 );
    getInstance()->output->gain(1, 1 );
    getInstance()->output->gain(2, 1 );
    getInstance()->output->gain(3, 1 );
  }
}

/**
 * On MIDI Mode Change
 */
void Synth::onMidiModeChange(byte channel, byte control, byte value){
  unsigned int mapValue = map(
    value, 
    0,
    127,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue()
  );
  
  getInstance()->onModeChange(0, mapValue, 255);
}

/**
 * On Param Change
 */
inline void Synth::onParamChange(byte inputIndex, float value, int diffToPrevious){  
  getInstance()->parameter = value;
  
  switch(modes(getInstance()->mode)){
    case SYNTH:
    {
      // Portamento
      getInstance()->portamento = constrain(
        map(
          value,
          getInstance()->device->getAnalogMinValue(),
          getInstance()->device->getAnalogMaxValue(),
          255,
          0
        ),
        0,
        255
      );

      // Set it to all voices
      for (int i = 0; i < voiceCount ; i++) {
        getInstance()->voices[i]->setGlide(getInstance()->portamento);
      }
    }
    break;
    
    case ARP: 
      // Time
      getInstance()->arpTime = map(
        value,
        getInstance()->device->getAnalogMinValue(),
        getInstance()->device->getAnalogMaxValue(),
        1,
        500
      );
    break;
    
    case DRONE: 
      // Free frequency
      for (int i = 0; i < voiceCount ; i++) {
        getInstance()->voices[i]->setFrequencyTarget(value);
      }
    break;
  }
}

/**
 * On MIDI Param Change
 */
void Synth::onMidiParamChange(byte channel, byte control, byte value){
  unsigned int mapValue = map(
    value, 
    0,
    127,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue()
  );
  
  getInstance()->device->setPotentiometer(1, mapValue);
}

/**
 * On Shape Change
 */
inline void Synth::onShapeChange(byte inputIndex, float value, int diffToPrevious){
  // Shape
  float shape = (float)map(
    (float)value, 
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue(),
    0,
    1
  );
    
  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setShape(shape);
  }
}

/**
 * On MIDI Shape Change
 */
void Synth::onMidiShapeChange(byte channel, byte control, byte value){
  unsigned int mapValue = map(
    value, 
    0,
    127,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue()
  );
  
  getInstance()->device->setPotentiometer(2, mapValue);
}


/**
 * On FM Change
 */
inline void Synth::onFmChange(byte inputIndex, float value, int diffToPrevious){
  int modulatorFrequency = 0;
  float modulatorAmplitude = 0;

  if(value < getInstance()->device->getAnalogMaxValue() / 3){
    modulatorFrequency = map(value, 0, getInstance()->device->getAnalogMaxValue() / 2, 0, 10);
    modulatorAmplitude = (float)map(value, 0, getInstance()->device->getAnalogMaxValue() / 2, 0.001, .01);
  }
  else if(value >= getInstance()->device->getAnalogMaxValue() / 3 && value < getInstance()->device->getAnalogMaxValue() / 2){
    modulatorFrequency = map(value, 0, getInstance()->device->getAnalogMaxValue() / 2, 0, 40);
    modulatorAmplitude = (float)map(value, 0, getInstance()->device->getAnalogMaxValue() / 2, 0.001, .01);
  }else{
    modulatorFrequency = map(value, getInstance()->device->getAnalogMaxValue() / 2, getInstance()->device->getAnalogMaxValue(), 0, 1000);
    modulatorAmplitude = (float)map(value, getInstance()->device->getAnalogMaxValue() / 2, getInstance()->device->getAnalogMaxValue(), 0, .5);
  }
  
  if(getInstance()->modulatorFrequency != modulatorFrequency){
    getInstance()->modulatorFrequency = modulatorFrequency;
    for (int i = 0; i < voiceCount ; i++) {
      getInstance()->voices[i]->setModulatorFrequency(modulatorFrequency);
    }
  }
  
  if(value > 50){
    if(getInstance()->modulatorAmplitude != modulatorAmplitude){
      getInstance()->modulatorAmplitude = modulatorAmplitude;
      for (int i = 0; i < voiceCount ; i++) {
        getInstance()->voices[i]->setModulatorAmplitude(modulatorAmplitude);
      }
    }
  }else{
    for (int i = 0; i < voiceCount ; i++) {
      getInstance()->voices[i]->setModulatorAmplitude(0);
    }
  }
}

/**
 * On MIDI FM Change
 */
void Synth::onMidiFmChange(byte channel, byte control, byte value){
  unsigned int mapValue = map(
    value, 
    0,
    127,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue()
  );
  
  getInstance()->device->setPotentiometer(3, mapValue);
}

/**
 * On Attack Change
 */
inline void Synth::onAttackChange(byte inputIndex, float value, int diffToPrevious){
  unsigned int attack = map(
    value,
    getInstance()->device->getAnalogMinValue(),
    getInstance()->device->getAnalogMaxValue(),
    0,
    2000
  );
  
  getInstance()->attack = attack;
  
  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setAttack(attack);
  }
}

/**
 * On MIDI Attack Change
 */
void Synth::onMidiAttackChange(byte channel, byte control, byte value){
  unsigned int mapValue = map(
    value, 
    0,
    127,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue()
  );
  
  getInstance()->device->setPotentiometer(4, mapValue);
}

/**
 * On Release Change
 */
inline void Synth::onReleaseChange(byte inputIndex, float value, int diffToPrevious){
  unsigned int release = map(
    value, 
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue(),
    0,
    2000
  );

  getInstance()->release = release;
  
  for (int i = 0; i < voiceCount ; i++) {
    getInstance()->voices[i]->setRelease(release);
  }
}

/**
 * On MIDI Release Change
 */
void Synth::onMidiReleaseChange(byte channel, byte control, byte value){
  unsigned int mapValue = map(
    value, 
    0,
    127,
    getInstance()->device->getAnalogMinValue(), 
    getInstance()->device->getAnalogMaxValue()
  );
  
  getInstance()->device->setPotentiometer(5, mapValue);
}

#endif
