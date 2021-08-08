#ifndef TouchPad_h
#define TouchPad_h

#include "Input.h"

class TouchPad : public Input
{
public:
  void read() override;
  bool needsGround() override;
  String getType() override;
  void setSensitivity(int sensitivity);

private:
//  int sensitivity = 100;
  int lowValue = 70; //64
  int highValue = 80;//85
};

inline void TouchPad::read()
{
  pinMode(this->pin, INPUT_PULLDOWN);
  int val = fastTouchRead(this->pin);
  val = map(constrain(val, this->lowValue, this->highValue), this->lowValue, this->highValue, 0, 4095);// TODO: ADD ANALOG MIN-MAX TO INPUT
  this->setTarget(val);
}

inline bool TouchPad::needsGround()
{
  return false;
}

inline String TouchPad::getType()
{
  return "TouchPad";
}

inline void TouchPad::setSensitivity(int sensitivity){
//  this->sensitivity = sensitivity;
  this->lowValue = map(constrain(sensitivity,0,100), 0, 100, 100, 64);;
}

// From now on "TouchPad" will be replaced by "new TouchPad()"
#define TouchPad new MotherboardNamespace::TouchPad()

#endif
