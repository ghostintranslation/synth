#ifndef Motherboard_h
#define Motherboard_h

#include <SPI.h>
#include <map>
#include <vector>
//#include "FastTouch.h"

namespace MotherboardNamespace{

class Motherboard;

#include "IOManager.h"
#include "MidiManager.h"

/** 
 * Motherboard
 * v2.0.0
 */
class Motherboard
{

private:
  static Motherboard *instance;
  Motherboard();

  IOManager* ioManager;
  MidiManager* midiManager;

  bool debug = false;
  elapsedMillis clockDebug;

public:
  static Motherboard *getInstance();
  void init(String deviceName, int columnNumber);
  void update();

//  void setLED(byte index, Led::Status status, unsigned int brightness = 4095);
//  void toggleLED(byte index);
//  void resetAllLED();
  void initSequence();
  float getInputValue(byte index);
//  void setOutput(byte index, unsigned int value);
//  void setSmoothing(byte smoothing);
  int getAnalogMaxValue();
  int getAnalogMinValue();
//  byte getMidiChannel();

//  MidiManager* getMidiManager();

  // MIDI Callbacks
//  void setHandleMidiNoteOn(MidiNoteOnCallback fptr);
//  void setHandleMidiNoteOff(MidiNoteOffCallback fptr);
//  // void setHandleGlobalMidiControlChange(GlobalMidiControlChangeCallback fptr);
//  void setHandleMidiControlChange(byte control, String controlName, MidiControlChangeCallbackFunction fptr);
//  void setHandleMidiControlChange(byte midiChannel, byte midiCC, String controlName, MidiControlChangeCallbackFunction fptr);
//  void setHandleMidiSysEx(MidiSysExCallback fptr);

  void setDebug(bool debug);
};

// Instance pre init
Motherboard *Motherboard::instance = nullptr;

/**
 * Constructor
 */
inline Motherboard::Motherboard()
{
}

/**
 * Singleton instance
 */
inline Motherboard *Motherboard::getInstance()
{
  if (!instance)
    instance = new Motherboard;
  return instance;
}

/**
 * Init
 */
inline void Motherboard::init(String deviceName, int columnNumber)
{
  if (this->debug) {
    Serial.println("IOManager init...");
  }
  this->ioManager = IOManager::getInstance();
  ioManager->init(columnNumber);
  
  if (this->debug) {
    Serial.println("MidiManager init...");
  }
  this->midiManager = MidiManager::getInstance();
  midiManager->init();
  
  //  ioManager->getMidiChannel();
  //
  //  // Init sequence
  //  this->initSequence();

  if (this->debug) {
    Serial.println("Ready!");
  }
}

/**
 * Update
 */
inline void Motherboard::update()
{
  ioManager->update();
  midiManager->update();

  // Debug
   if (this->debug && this->clockDebug >= 10) {
      ioManager->print();
//      midiManager->print();
      Serial.println("");
     this->clockDebug = 0;
   }
}


//inline void Motherboard::setOutput(byte index, unsigned int value)
//{
//  ioManager->setOutputValue(index, value);  
//}

/**
 * Set a led status
 */
//inline void Motherboard::setLED(byte index, Led::Status status, unsigned int brightness)
//{
//  ioManager->setLED(index, status, brightness);
//}

/**
 * Toggle one LED
 */
//inline void Motherboard::toggleLED(byte index)
//{
//  //   ioManager->toggleLed(index);
//}

/**
 * Reset all LEDs
 */
//inline void Motherboard::resetAllLED()
//{
//  //   ioManager->resetAllLED(index);
//}

/**
 * Reset all LEDs
 */
//inline void Motherboard::setSmoothing(byte smoothing)
//{
//  ioManager->setSmoothing(smoothing);
//}

/**
 * Get input value
 * @param byte index The index of the input
 */
inline float Motherboard::getInputValue(byte index)
{
  return ioManager->getInputValue(index);
}

/**
 * Get max analog value according to resolution
 */
inline int Motherboard::getAnalogMinValue()
{
  return ioManager->getAnalogMinValue();
}

/**
 * Get max analog value according to resolution
 */
inline int Motherboard::getAnalogMaxValue()
{
    return ioManager->getAnalogMaxValue();
}

//inline byte Motherboard::getMidiChannel()
//{
//  return ioManager->getMidiChannel();
//}

/**
 * LEDs init sequence
 */
inline void Motherboard::initSequence()
{
}

inline void Motherboard::setDebug(bool debug)
{
  this->debug = debug;
}


/**
 * Set handle MIDI note on
 */
//inline void Motherboard::setHandleMidiNoteOn(MidiNoteOnCallback fptr){
//  midiManager->setHandleMidiNoteOn(fptr);
//}
//
///**
// * Set handle MIDI note off
// */
//inline void Motherboard::setHandleMidiNoteOff(MidiNoteOffCallback fptr){
//  midiManager->setHandleMidiNoteOff(fptr);
//}
//
//
//inline void Motherboard::setHandleMidiControlChange(byte control, String controlName, MidiControlChangeCallbackFunction fptr){
//  midiManager->setHandleMidiControlChange(control, controlName, fptr);
//}
//
//inline void Motherboard::setHandleMidiControlChange(byte midiChannel, byte midiCC, String controlName, MidiControlChangeCallbackFunction fptr){
//  midiManager->setHandleMidiControlChange(midiChannel, midiCC, controlName, fptr);
//}
//
//inline void Motherboard::setHandleMidiSysEx(MidiSysExCallback fptr){
//  midiManager->setHandleMidiSysEx(fptr);
//}
//
//inline MidiManager* Motherboard::getMidiManager(){
//  return this->midiManager;
//}


}
// Instanciating
MotherboardNamespace::Motherboard &Motherboard = *MotherboardNamespace::Motherboard::getInstance();

#endif
