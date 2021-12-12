#ifndef IOTypeCV_h
#define IOTypeCV_h

#include "IOType.h"

/**
 * CV input/output only limit values from 0 to 4095
 */
class IOTypeCV : public IOType
{

public:
    virtual float processTarget(float target) override;
    
    virtual float processValue(float value) override;
    
    virtual unsigned int processMidiCC(unsigned int value) override;
};


inline float IOTypeCV::processValue(float value){
  return constrain(value, 0, 4095);
}

inline float IOTypeCV::processTarget(float target){
  return constrain(target, 0, 4095);
}

inline unsigned int IOTypeCV::processMidiCC(unsigned int value){
  return constrain(value, 0, 4095);
}


#endif
