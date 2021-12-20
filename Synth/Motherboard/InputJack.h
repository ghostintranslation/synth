#ifndef InputJack_h
#define InputJack_h

#include "PhysicalInput.h"

// TODO: Maybe derive InputPotentiometer instead
class InputJack : public PhysicalInput
{
public:
  InputJack(unsigned int index, String name);
  
  bool isDirectToTeensy() {return false;}
    
  void read() override;

  String getClassName() override {return "InputJack";}
};

inline InputJack::InputJack(unsigned int index, String name):PhysicalInput{index, name}{
  this->midiMode = Multiply;
  this->min = 5;
}

inline void InputJack::read()
{
  pinMode(this->pin, INPUT);

  unsigned int reading = analogRead(this->pin);
  
  unsigned int  val = 0.7*previousReading + 0.3*reading; // low pass filter

  this->setTarget(val);
    
  this->previousReading = reading;
}

#endif
