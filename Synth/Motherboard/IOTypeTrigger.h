#ifndef IOTypeTrigger_h
#define IOTypeTrigger_h

#include "IOType.h"

/**
 * Trigger inputs/outputs have only 2 values, 0 and 4095, 
 * but also stay at high level only for a few ms.
 */
class IOTypeTrigger : public IOType
{

public:
    virtual float processTarget(float target, float min, float max) override;
    
    virtual float processValue(float value, float min, float max) override;
    
    virtual unsigned int processMidiCC(unsigned int value) override;
    
    virtual float processSmoothing(float smoothing) override;

//    virtual float processTargetBeforeValueUpdate(float target) override;
    
private:
  elapsedMillis triggerClock;
  bool hasTriggered = false;

  void trigger();
};


inline float IOTypeTrigger::processValue(float value, float min, float max){
  float newVal = 0;
  if(value > (ABSOLUTE_ANALOG_MAX+1)/2){
    newVal = ABSOLUTE_ANALOG_MAX;
  }else{
    newVal = ABSOLUTE_ANALOG_MIN;
  }

  return newVal;
}

inline float IOTypeTrigger::processTarget(float target, float min, float max){
 if(target > (ABSOLUTE_ANALOG_MAX+1)/2 && !this->hasTriggered){
    this->trigger();
  }else if(target < ABSOLUTE_ANALOG_MAX/2 && this->hasTriggered){
    this->hasTriggered = false;
  }

  if(this->hasTriggered && this->triggerClock < 50){
    target = ABSOLUTE_ANALOG_MAX;
  }else {
    target = ABSOLUTE_ANALOG_MIN;
  }
  
  return target;
}

inline unsigned int IOTypeTrigger::processMidiCC(unsigned int value){
  return this->processTarget(value, 0, 0);
}

//inline float IOTypeTrigger::processTargetBeforeValueUpdate(float target){
//  float newVal = 0;
//  
//  if(this->hasTriggered && this->triggerClock < 50){
//    newVal = 4095;
//  }else {
//    newVal = 0;
//  }
//
//  return newVal;
//}

inline void IOTypeTrigger::trigger(){
  this->triggerClock = 0;
  this->hasTriggered = true;
}

inline float IOTypeTrigger::processSmoothing(float smoothing){
  return 1; // It's a trigger, there is only 2 states, no in between.
}

#endif