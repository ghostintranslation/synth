#ifndef Potentiometer_h
#define Potentiometer_h

#include "Input.h"

class Potentiometer : public Input
{
public:
  void read() override;
  bool needsGround() override;
  String getType() override;
};

inline void Potentiometer::read()
{
  pinMode(this->pin, INPUT);
  int val = analogRead(this->pin);
  val = map(constrain(val, 8, 4095), 8, 4095, 0, 4095); // TODO: ADD ANALOG MIN-MAX TO INPUT

  this->setTarget(val);
}

inline bool Potentiometer::needsGround()
{
  return true;
}

inline String Potentiometer::getType()
{
  return "Potentiometer";
}

// From now on "Potentiometer" will be replaced by "new Potentiometer()"
#define Potentiometer new MotherboardNamespace::Potentiometer()

#endif
