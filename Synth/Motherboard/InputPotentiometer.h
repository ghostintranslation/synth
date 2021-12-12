#ifndef InputPotentiometer_h
#define InputPotentiometer_h

#include "PhysicalInput.h"

class InputPotentiometer : public PhysicalInput
{
public:
  InputPotentiometer(int index, String name);
  
  bool isDirectToTeensy() {return false;}
    
  void read() override;

  String getClassName() override {return "InputPotentiometer";}
};

inline InputPotentiometer::InputPotentiometer(int index, String name):PhysicalInput{index, name}{
  this->midiMode = Multiply;
}

inline void InputPotentiometer::read()
{
  pinMode(this->pin, INPUT);
  unsigned int reading = analogRead(this->pin);
  
  unsigned int  val = 0.9*previousReading + 0.1*reading; // low pass filter

  unsigned int newval = map(constrain(val, 35, 4086), 35, 4086, 0, 4095);

  this->setTarget(newval);
    
  this->previousReading = newval;
}

// Prevents to have to write the namespace when using this class
#define InputPotentiometer MotherboardNamespace::InputPotentiometer

#endif
