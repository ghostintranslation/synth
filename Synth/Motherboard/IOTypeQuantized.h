#ifndef IOTypeQuantized_h
#define IOTypeQuantized_h

#include "IOType.h"

/**
 * CV input/output only limit values from 0 to 4095
 */
class IOTypeQuantized : public IOType
{
private:
    unsigned int quantize(float value);
    
public:
    virtual float processTarget(float target, float min, float max) override;
    
//    virtual float processValue(float value) override;
    
//    virtual unsigned int processMidiCC(unsigned int value) override;
    
    virtual float processSmoothing(float smoothing) override;
};


//inline float IOTypeQuantized::processValue(float value){
//  Serial.print(value);
//  Serial.print(" ");
//  Serial.println((value/4096) * (4096/5/12));
//  return this->quantize(value);
//}

inline float IOTypeQuantized::processTarget(float target, float min, float max){
//  Serial.print(target);
//  Serial.print(" ");
//  Serial.println((target/4096) * (4096/5/12));
  return this->quantize(target);
}

//inline unsigned int IOTypeQuantized::processMidiCC(unsigned int value){
//  // TODO
//  return value;
//}

/**
 * Max value = 5 volts = 4095 (for a total of 4096 values including the 0)
 * 5 octaves in 5 volts (1v/oct)
 * 12 Notes per Octave
 */
inline unsigned int IOTypeQuantized::quantize(float value){
  return (value/4096) * (4096/5/12);
}

inline float IOTypeQuantized::processSmoothing(float smoothing){
  return 1;
}

#endif
