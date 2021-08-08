#ifndef led_h
#define led_h

#include "Output.h"

class Led : public Output
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

  String getType() override;

  void update(unsigned int updateMillis);
  void set(Status status, int brightness);

private:
  Status status;

  // The value target requested
  unsigned int requestedTarget;

  // Time counter for the blinking
  elapsedMillis blinkTime;
};

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

inline void Led::update(unsigned int updateMillis)
{
  Output::update(updateMillis);

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

inline String Led::getType()
{
  return "Led";
}
#endif
