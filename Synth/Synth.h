#ifndef Synth_h
#define Synth_h

#include <Audio.h>
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE(); // MIDI library init

#include "Motherboard6.h"
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
    Motherboard6 *device;
    
    Voice *voices[voiceCount];
    byte actualVoiceCount = 1;
    byte synthesis;
    byte mode;
    int parameter;
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
    Voice **getVoices();
    AudioMixer4 * getOutput();

};

// Singleton pre init
Synth * Synth::instance = nullptr;

/**
 * Constructor
 */
inline Synth::Synth(){

  this->device = Motherboard6::getInstance();
  
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
  // 0 = empty, 1 = button, 2 = potentiometer, 3 = encoder
  byte controls[9] = {2,2, 2,2, 2,2};
  this->device->init(controls);

  MIDI.setHandleNoteOn(noteOn);
  MIDI.setHandleNoteOff(noteOff);
  MIDI.begin(this->device->getMidiChannel());
  usbMIDI.setHandleNoteOn(noteOn);
  usbMIDI.setHandleNoteOff(noteOff);
  usbMIDI.setHandleStop(stop);
  usbMIDI.setHandleSystemReset(stop);
}

/**
 * Note on
 */
inline void Synth::noteOn(byte channel, byte note, byte velocity){
  
  bool foundOne = false;
  int oldestVoice = 0;
  unsigned long oldestVoiceTime = 0;
  
  switch (modes(getInstance()->mode)){
    case SYNTH: 
      for (int i = 0; i < getInstance()->actualVoiceCount; i++) {
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
  
  MIDI.read(this->device->getMidiChannel());
  usbMIDI.read(this->device->getMidiChannel());
  
  if(this->clockUpdate > updateMillis){
    
    for (int i = 0; i < voiceCount ; i++) {
      this->voices[i]->update();
    }

    // Mode
    byte mode = (byte)constrain(map(this->device->getInput(this->modeInputIndex), this->device->getAnalogMinValue(), this->device->getAnalogMaxValue(), 0, 2), 0, 2);
    bool monoPoly = map(this->device->getInput(this->modeInputIndex), this->device->getAnalogMinValue(), this->device->getAnalogMaxValue() / 3, 0, 1);
    
    // A little addition to the SYNTH mode
    // Mono and Poly modes
    if(monoPoly > 0){
      this->actualVoiceCount = 16;
    }else{
      this->actualVoiceCount = 1;
    }
    
    if(this->mode != mode){
      this->mode = mode;
      
      for (int i = 0; i < voiceCount ; i++) {
        this->voices[i]->setMode(mode);
      }
  
      if(modes(this->mode) == DRONE){
        this->mixers[0]->gain(0, 0.6 );
        this->mixers[0]->gain(1, 0 );
        this->mixers[0]->gain(2, 0 );
        this->mixers[0]->gain(3, 0 );
        this->output->gain(0, 1 );
        this->output->gain(1, 0 );
        this->output->gain(2, 0 );
        this->output->gain(3, 0 );
      }else{
        this->mixers[0]->gain(0, 0.6 );
        this->mixers[0]->gain(1, 0.6 );
        this->mixers[0]->gain(2, 0.6 );
        this->mixers[0]->gain(3, 0.6 );
        this->output->gain(0, 1 );
        this->output->gain(1, 1 );
        this->output->gain(2, 1 );
        this->output->gain(3, 1 );
      }
    }
  
    // Parameter
    int parameter = this->device->getInput(this->parameterInputIndex);
  
    if(this->parameter != parameter){
      this->parameter = parameter;
      
      switch(modes(this->mode)){
        case SYNTH:
        {
          // Glide
          int voiceGlide = constrain(map(parameter, this->device->getAnalogMinValue(), this->device->getAnalogMaxValue(), 0, 255), 0, 255);

          for (int i = 0; i < voiceCount ; i++) {
            this->voices[i]->setGlide(voiceGlide);
          }
        }
        break;
        case ARP: 
          // Time
          this->arpTime = map(parameter, this->device->getAnalogMinValue(), this->device->getAnalogMaxValue(), 1, 500);
        break;
        case DRONE: 
          // Free frequency
          for (int i = 0; i < voiceCount ; i++) {
            this->voices[i]->setFrequencyTarget(parameter);
          }
        break;
      }
    }
  
    // Arp
    if(modes(this->mode) == ARP){
      if (this->elapsedTime >= this->arpTime) {
  
        if(this->arpNotesPlaying > 0){
          this->voices[0]->noteOn(this->arpNotes[this->arpIndex]);
        }
        
          
        this->arpIndex++;
        if(this->arpIndex > this->arpNotesPlaying-1 ){
          this->arpIndex = 0;
        }
        
        this->elapsedTime = 0;
      }
    }
      
    
    // Shape
    float shape = (float)map(
      (float)this->device->getInput(
      this->shapeInputIndex), 
      this->device->getAnalogMinValue(), 
      this->device->getAnalogMaxValue(),
      0,
      1
    );
    
    if(this->shape != shape){
      this->shape = shape;
      for (int i = 0; i < voiceCount ; i++) {
        this->voices[i]->setShape(shape);
      }
    } 
  
    // Attack
    unsigned int attack = map(this->device->getInput(this->attackInputIndex), this->device->getAnalogMinValue(), this->device->getAnalogMaxValue(), 0, 2000);
    
    if(this->attack != attack){
      this->attack = attack;
      for (int i = 0; i < voiceCount ; i++) {
        this->voices[i]->setAttack(attack);
      }
    }
  
  
    // Release
    unsigned int release = map(this->device->getInput(this->releaseInputIndex), this->device->getAnalogMinValue(), this->device->getAnalogMaxValue(), 0, 2000);
  
    if(this->release != release){
      this->release = release;
      for (int i = 0; i < voiceCount ; i++) {
        this->voices[i]->setRelease(release);
      }
    }
  
    // FM
    int fm = this->device->getInput(this->fmInputIndex);
    int modulatorFrequency = 0;
    float modulatorAmplitude = 0;
    
    if(fm < this->device->getAnalogMaxValue() / 3){
      modulatorFrequency = map(fm, 0, this->device->getAnalogMaxValue() / 2, 0, 10);
      modulatorAmplitude = (float)map((float)fm, 0, this->device->getAnalogMaxValue() / 2, 0.001, .01);
    }
    else if(fm >= this->device->getAnalogMaxValue() / 3 && fm < this->device->getAnalogMaxValue() / 2){
      modulatorFrequency = map(fm, 0, this->device->getAnalogMaxValue() / 2, 0, 40);
      modulatorAmplitude = (float)map((float)fm, 0, this->device->getAnalogMaxValue() / 2, 0.001, .01);
    }else{
      modulatorFrequency = map(fm, this->device->getAnalogMaxValue() / 2, this->device->getAnalogMaxValue(), 0, 1000);
      modulatorAmplitude = (float)map((float)fm, this->device->getAnalogMaxValue() / 2, this->device->getAnalogMaxValue(), 0, .5);
    }
    
    if(this->modulatorFrequency != modulatorFrequency){
      this->modulatorFrequency = modulatorFrequency;
      for (int i = 0; i < voiceCount ; i++) {
        this->voices[i]->setModulatorFrequency(modulatorFrequency);
      }
    }
    if(fm > 50){
      if(this->modulatorAmplitude != modulatorAmplitude){
        this->modulatorAmplitude = modulatorAmplitude;
        for (int i = 0; i < voiceCount ; i++) {
          this->voices[i]->setModulatorAmplitude(modulatorAmplitude);
        }
      }
    }else{
      for (int i = 0; i < voiceCount ; i++) {
        this->voices[i]->setModulatorAmplitude(0);
      }
    }

    
    this->clockUpdate = 0;
  }
}

inline Voice** Synth::getVoices(){
  return this->voices;
}

#endif
