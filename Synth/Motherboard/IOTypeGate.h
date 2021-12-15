#ifndef IOTypeGate_h
#define IOTypeGate_h

#include "IOType.h"

/**
 * Gate inputs/outputs have only 2 values, 0 and 4095
 */
class IOTypeGate : public IOType
{

public:
    virtual float processTarget(float target) override;
    
    virtual float processValue(float value) override;
    
    virtual unsigned int processMidiCC(unsigned int value) override;
    
    virtual float processSmoothing(float smoothing) override;
};


inline float IOTypeGate::processValue(float value){
  float newVal = 0;
  if(value < (ABSOLUTE_ANALOG_MAX+1)/2){
    newVal = ABSOLUTE_ANALOG_MIN;
  }else{
    newVal = ABSOLUTE_ANALOG_MAX;
  }

  return newVal;
}

inline float IOTypeGate::processTarget(float target){
  float newTarget = 0;
  if(target < (ABSOLUTE_ANALOG_MAX+1)/2){
    newTarget = ABSOLUTE_ANALOG_MIN;
  }else{
    newTarget = ABSOLUTE_ANALOG_MAX;
  }
  
  return newTarget;
}

inline unsigned int IOTypeGate::processMidiCC(unsigned int value){
  unsigned int newVal = 0;
  if(value < (ABSOLUTE_ANALOG_MAX+1)/2){
    newVal = ABSOLUTE_ANALOG_MIN;
  }else{
    newVal = ABSOLUTE_ANALOG_MAX;
  }
  
  return newVal;
}

inline float IOTypeGate::processSmoothing(float smoothing){
  return 1; // It's a gate, there is only 2 states, no in between.
}

#endif
