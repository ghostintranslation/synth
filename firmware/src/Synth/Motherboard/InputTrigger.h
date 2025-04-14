#ifndef InputTrigger_h
#define InputTrigger_h

#include "Registrar.h"
#include "Input.h"

class InputTrigger : public Input, public Registrar<InputTrigger>
{
public:
    InputTrigger(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    void setThreshold(int16_t threshold);

private:
    uint8_t index;
    int16_t threshold = 0;
    uint16_t triggerDuration = 256; // 256 samples at 44100Hz means 5.8ms
    uint16_t triggerSamplesleft = 0;
    int16_t previousSample = 0;
};

inline InputTrigger::InputTrigger(int8_t index) : Input(index)
{
}

inline void InputTrigger::update(void)
{
    Input::update();
}

inline int16_t *&InputTrigger::updateBefore(int16_t *&blockData)
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

    return blockData;
}

inline void InputTrigger::setThreshold(int16_t threshold)
{
    this->threshold = threshold;
}

#endif
