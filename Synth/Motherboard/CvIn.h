#ifndef CvIn_h
#define CvIn_h

#include "Input.h"

enum CvInRange{
  V4 = 0,
  V5 = 1
};

class CvIn : public Input
{
public:
  void read() override;
  bool needsGround() override;
  String getType() override;
  void setRange(CvInRange range);

private:
  CvInRange range;
};

inline void CvIn::read()
{
  pinMode(this->pin, INPUT);
  int val = analogRead(this->pin);

  switch(this->range){
    case V5:
        val = constrain(val,0,4095); // CV 5V
    break;
    case V4:
    default:
        val = map(constrain(val,0,3420), 0, 3420, 0, 4095);
    break;
    
  }
  // TODO: ADD ANALOG MIN-MAX TO INPUT

  this->setTarget(val);
}

inline bool CvIn::needsGround()
{
  return true;
}

inline String CvIn::getType()
{
  return "CvIn";
}

inline void CvIn::setRange(CvInRange range)
{
  this->range = range;
}

// From now on "CvIn" will be replaced by "new CvIn()"
#define CvIn new MotherboardNamespace::CvIn()

#endif
