#ifndef Motherboard_h
#define Motherboard_h

#include <SPI.h>
#include <map>
#include <vector>
//#include "FastTouch.h"

//namespace MotherboardNamespace{

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
  void initSequence();
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


//}
// Instanciating
//MotherboardNamespace::Motherboard &Motherboard = *MotherboardNamespace::Motherboard::getInstance();
Motherboard &Motherboard = *Motherboard::getInstance();
#endif
