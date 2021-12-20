#ifndef InputTouchpad_h
#define InputTouchpad_h

#include "PhysicalInput.h"
//#include "FastTouch.h"
#include "FastTouch/src/FastTouch.cpp"

class InputTouchpad : public PhysicalInput
{
public:
  InputTouchpad(unsigned int index, String name);
  
  bool isDirectToTeensy() {return true;}
    
  void read() override;

  String getClassName() override {return "InputTouchpad";}
};

inline InputTouchpad::InputTouchpad(unsigned int index, String name):PhysicalInput{index, name}{
  this->midiMode = Multiply;
  this->min = 58;
  this->max = 225;
}

inline void InputTouchpad::read()
{
  pinMode(this->pin, INPUT_PULLDOWN);
  unsigned int reading = fastTouchRead(this->pin);
  
  unsigned int  val = 0.9*previousReading + 0.1*reading; // low pass filter

  this->setTarget(val);
    
  this->previousReading = reading;
}

// Prevents to have to write the namespace when using this class
//#define InputTouchpad MotherboardNamespace::InputTouchpad

#endif
