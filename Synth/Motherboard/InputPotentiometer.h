#ifndef InputPotentiometer_h
#define InputPotentiometer_h

#include "PhysicalInput.h"

class InputPotentiometer : public PhysicalInput
{
public:
  InputPotentiometer(unsigned int index, String name);
  
  bool isDirectToTeensy() {return false;}
    
  void read() override;

  String getClassName() override {return "InputPotentiometer";}
};

inline InputPotentiometer::InputPotentiometer(unsigned int index, String name):PhysicalInput{index, name}{
  this->midiMode = Multiply;
  this->min = 23;
}

inline void InputPotentiometer::read()
{
  pinMode(this->pin, INPUT_PULLDOWN);
  unsigned int reading = analogRead(this->pin);
  
  unsigned int  val = 0.9*previousReading + 0.1*reading; // low pass filter

  this->setTarget(val);
    
  this->previousReading = reading;
}

// Prevents to have to write the namespace when using this class
//#define InputPotentiometer MotherboardNamespace::InputPotentiometer

#endif
