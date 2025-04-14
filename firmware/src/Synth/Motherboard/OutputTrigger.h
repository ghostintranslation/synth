#ifndef OutputTrigger_h
#define OutputTrigger_h

#include "Registrar.h"
#include "Output.h"

class OutputTrigger : public Output, public Registrar<OutputTrigger>
{
public:
    OutputTrigger(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    void trigger();
    void setThreshold(int16_t threshold);

private:
    audio_block_t *inputQueueArray[1];
    uint8_t index;
    int16_t threshold = 0;
    uint16_t triggerDuration = 256; // 256 samples at 44100Hz means 5.8ms
    uint16_t triggerSamplesleft = 0;
    int16_t previousSample = 0;
};

inline OutputTrigger::OutputTrigger(int8_t index) : Output(index)
{
}

inline void OutputTrigger::update(void)
{
    Output::update();
}

inline int16_t *&OutputTrigger::updateBefore(int16_t *&blockData)
{
    if (blockData)
    {
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            if (blockData[i] > this->threshold && this->previousSample < this->threshold && this->triggerSamplesleft == 0)
            {
                this->triggerSamplesleft = this->triggerDuration;
            }
            this->previousSample = blockData[i];

            if (this->triggerSamplesleft > 0)
            {
                this->triggerSamplesleft--;
                blockData[i] = INT16_MAX;
            }
            else
            {
                blockData[i] = INT16_MIN;
            }
        }
    }

    return blockData;
}

inline void OutputTrigger::trigger()
{
    this->triggerSamplesleft = this->triggerDuration;
}

inline void OutputTrigger::setThreshold(int16_t threshold)
{
    this->threshold = threshold;
}

#endif
