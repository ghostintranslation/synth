#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include "Input.h"

class RotaryEncoder : public Input
{
public:
  void read() override;
  bool needsGround() override;
  String getType() override;
  
  bool isPressed();

  // Debug
  void print();

private:
  int previousReading = 0;
  bool pressed = 0;
};

inline void RotaryEncoder::read()
{
  pinMode(this->pin, INPUT_PULLUP);
  int val = analogRead(this->pin);
  val = constrain(val,0,4095);// TODO: ADD ANALOG MIN-MAX TO INPUT


  this->pressed = 0;
    
  if(val > 4095/3 && abs(val - this->previousReading) > 4095/20){
    if(val > 4095/1.5 && val > this->previousReading){
      this->target--;
    }else if(val < 4095/1.5 && val < this->previousReading){
      this->target++;
    }
    this->previousReading = val;
  }else if (val <= 4095/3){
    this->pressed = 1;
  }

}

inline bool RotaryEncoder::needsGround()
{
  return false;
}

inline String RotaryEncoder::getType()
{
  return "RotaryEncoder";
}

inline bool RotaryEncoder::isPressed(){
  return this->pressed;
}

inline void RotaryEncoder::print(){
    Serial.print(this->value);
    Serial.print("/");
    Serial.print(this->pressed);
}
// From now on "RotaryEncoder" will be replaced by "new RotaryEncoder()"
#define RotaryEncoder new MotherboardNamespace::RotaryEncoder()

#endif
