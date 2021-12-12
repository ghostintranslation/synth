#ifndef Led_h
#define Led_h

#include "Registrar.h"
#include "PhysicalOutput.h"

class Led : public PhysicalIO, public Registrar<Led>
{

public:
  enum Status
  {
    Off,
    On,
    Blink,
    BlinkFast,
    BlinkOnce
  };
  
  Led(int index);

  void print() override;

  void update() override;
  void set(Status status, int brightness);
  void setStatus(Status status);
  void setTarget(float target) override;

  String getClassName() override{return "Led";}

private:
  Status status = Off;

  // The value target requested
  unsigned int requestedTarget = 0;

  // Time counter for the blinking
  elapsedMillis blinkTime;
};

inline Led::Led(int index):PhysicalIO{index, (String)"Led" + index}{
//  IORegistrar::registerLed(this);
}

inline void Led::set(Status status, int brightness)
{
  this->status = status;
  this->requestedTarget = brightness;

  switch (this->status)
  {
  case Off:
    this->requestedTarget = 0;
    break;
  case BlinkOnce:
    this->blinkTime = 0;
    break;
  default:
    break;
  }
}

inline void Led::setStatus(Status status){
  this->status = status;
}

inline void Led::update()
{
  PhysicalIO::update();

  switch (this->status)
  {
  case Blink:
    if (this->blinkTime % 400 < 200)
    {
      this->target = 0;
    }
    else
    {
      this->target = this->requestedTarget;
    }
    break;
  case BlinkFast:
    if (this->blinkTime % 200 < 100)
    {
      this->target = 0;
    }
    else
    {
      this->target = this->requestedTarget;
    }
    break;
  case BlinkOnce:
    if (this->blinkTime > 100)
    {
      this->target = 0;
    }else{
      this->target = this->requestedTarget;
    }
    break;

  default:
      this->target = this->requestedTarget;
    break;
  }
}

inline void Led::setTarget(float target)
{
  PhysicalIO::setTarget(target);
  
  this->requestedTarget = this->target;
}

//inline void Led::onMidiCC(unsigned int value){
//  IO::onMidiCC(value);
//  
//  this->requestedTarget = this->midiValue;
//}

inline void Led::print()
{
  Serial.printf("%07.2f", this->value);
}

#define Led MotherboardNamespace::Led
#endif
