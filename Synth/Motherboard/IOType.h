#ifndef IOType_h
#define IOType_h

class IO; // Forward declaration

/**
 * Stratgy pattern here to process values differently 
 * depending on the type of input
 */
class IOType
{
public:
    virtual float processTarget(float target, float min, float max){return target;};
    
    virtual float processValue(float value, float min, float max){return value;};
    
    virtual unsigned int processMidiCC(unsigned int value){return value;};
    
    virtual float processSmoothing(float smoothing){return smoothing;};  

    virtual float processTargetBeforeValueUpdate(float target){return target;};  
};
#endif
