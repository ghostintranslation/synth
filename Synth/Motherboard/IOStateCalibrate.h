#ifndef IOStateCalibrate_h
#define IOStateCalibrate_h

#include "IOState.h"

/**
 * State pattern
 */
class IOStateCalibrate : public IOState
{
public:
    virtual void update() override;
};

inline void IOStateCalibrate::update(){
  // No update during calibration
}
#endif
