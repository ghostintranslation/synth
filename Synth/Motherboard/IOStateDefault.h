#ifndef IOStateDefault_h
#define IOStateDefault_h

#include "IOState.h"

/**
 * State pattern
 */
class IOStateDefault : public IOState
{
public:
    virtual void update() override;
};

inline void IOStateDefault::update(){
//  Serial.println("IO::update");
  
  // Maybe alter the smoothing
  this->io->setSmoothing(this->io->getType()->processSmoothing(this->io->getSmoothing()));
  
  // Set the target from the input
  audio_block_t *block;
  uint32_t *p, *end;
  unsigned int streamValue = 0;

  block = this->io->receiveStream();
  if (block){
    streamValue = constrain(block->data[0], 0, 4095);
    if(this->io->getPreviousStreamValue() != streamValue){
      this->io->setTarget(streamValue); // TODO: instead of using one value and dropping the 127 others, drop 1
      this->io->setPreviousStreamValue(streamValue);
    }
    this->io->releaseStream(block);
  }

//  this->io->target = this->io->getType()->processTargetBeforeValueUpdate(this->io->target);
  
  // Update the value to reach the target
  if (this->io->getTarget() != this->io->getValue())
  {
      if (this->io->getSmoothing() == 1)
      {
          this->io->setValue(this->io->getTarget());
      }
      else
      {
          this->io->setValue(this->io->getValue() + (this->io->getSmoothing() * (this->io->getTarget() - this->io->getValue()) / 1024) / (100 / this->io->getUpdateMillis()));
      }
  }

  // Rounding the float to compare to 0, because otherwise it is actually never quite 0
  // and we don't need more than 2 decimal precision
  this->io->setValue(roundf(this->io->getValue() * 100) / 100);

  // To eliminate noise
  if (this->io->getTarget() > 5 && this->io->getValue() == 0)
  {
      this->io->setValue(0);
  }

  // When value changes then call the callback.
  // Only carring about changes greater than 1.
  // This callback is defined in the Input class, and is empty in the Output class
  if (abs(this->io->getValue() - this->io->getPreviousValue()) > this->io->getThresholdOnChange())
  {
      this->io->onValueChange();
  }

  this->io->setPreviousValue(this->io->getValue());

  block = this->io->allocateStream();
  if (block){
    p = (uint32_t *)(block->data);
    end = p + AUDIO_BLOCK_SAMPLES/2;
    do {
      *p++ = this->io->getValue();
      *p++ = this->io->getValue();
      *p++ = this->io->getValue();
      *p++ = this->io->getValue();
      *p++ = this->io->getValue();
      *p++ = this->io->getValue();
      *p++ = this->io->getValue();
      *p++ = this->io->getValue();
    } while (p < end);
    this->io->transmitStream(block);
    this->io->releaseStream(block);
  }
}
#endif
