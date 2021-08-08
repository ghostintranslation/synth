#ifndef ToggleOnOffOn_h
#define ToggleOnOffOn_h

#include "Input.h"

class ToggleOnOffOn : public Input
{
public:
  void read() override;
  bool needsGround() override;
  String getType() override;
};

inline void ToggleOnOffOn::read()
{
  pinMode(this->pin, INPUT);
  int val = analogRead(this->pin);
  val = map(constrain(val,8,4095), 8, 4095, 0, 4095); // TODO: ADD ANALOG MIN-MAX TO INPUT
  if(val <= 1){
    val = 1;
  }else if(val > 1 && val < 100){
    val = -1;
  }else{
    val=0;
  }
  this->setTarget(val);
}

inline bool ToggleOnOffOn::needsGround()
{
  return false;
}

inline String ToggleOnOffOn::getType()
{
  return "ToggleOnOffOn";
}

// From now on "ToggleOnOffOn" will be replaced by "new ToggleOnOffOn()"
#define ToggleOnOffOn new MotherboardNamespace::ToggleOnOffOn()

#endif
