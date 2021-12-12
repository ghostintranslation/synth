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
  if(value < 4096/2){
    newVal = 0;
  }else{
    newVal = 4095;
  }

  return newVal;
}

inline float IOTypeGate::processTarget(float target){
  float newTarget = 0;
  if(target < 4096/2){
    newTarget = 0;
  }else{
    newTarget = 4095;
  }
  
  return newTarget;
}

inline unsigned int IOTypeGate::processMidiCC(unsigned int value){
  unsigned int newVal = 0;
  if(value < 4096/2){
    newVal = 0;
  }else{
    newVal = 4095;
  }
  
  return newVal;
}

inline float IOTypeGate::processSmoothing(float smoothing){
  return 1; // It's a gate, there is only 2 states, no in between.
}

#endif
