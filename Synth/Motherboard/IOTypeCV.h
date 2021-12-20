#ifndef IOTypeCV_h
#define IOTypeCV_h

#include "IOType.h"

/**
 * CV input/output only limit values from 0 to 4095
 */
class IOTypeCV : public IOType
{

public:
    virtual float processTarget(float target, float min, float max) override;
    
    virtual float processValue(float value, float min, float max) override;
    
    virtual unsigned int processMidiCC(unsigned int value) override;
};


inline float IOTypeCV::processValue(float value, float min, float max){
  return value;
}

inline float IOTypeCV::processTarget(float target, float min, float max){
  target = constrain(target, min, max);
  target = map(target, min, max, ABSOLUTE_ANALOG_MIN, ABSOLUTE_ANALOG_MAX);
  return target;
}

inline unsigned int IOTypeCV::processMidiCC(unsigned int value){
  return value;
}


#endif
