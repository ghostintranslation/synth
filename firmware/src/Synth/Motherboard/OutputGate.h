#ifndef OutputGate_h
#define OutputGate_h

#include "Registrar.h"
#include "Output.h"

class OutputGate : public Output, public Registrar<OutputGate>
{
public:
    OutputGate(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    // void open();
    // void close();
    void setThresholds(int16_t openThreshold, int16_t closeThreshold);

private:
    audio_block_t *inputQueueArray[1];
    uint8_t index;
    // bool isOpen = false;
    int16_t openThreshold = 100;
    int16_t closeThreshold = -100;
};

inline OutputGate::OutputGate(int8_t index) : Output(index)
{
}

inline void OutputGate::update(void)
{
    Output::update();
}

inline int16_t *&OutputGate::updateBefore(int16_t *&blockData)
{
    if (blockData)
    {
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            if (blockData[i] > openThreshold)
            {
                blockData[i] = INT16_MAX;
            }
            else if (blockData[i] < closeThreshold)
            {
                blockData[i] = INT16_MIN;
            }
        }
    }

    return blockData;
}

// inline void OutputGate::open()
// {
//     this->isOpen = true;
// }

// inline void OutputGate::close()
// {
//     this->isOpen = false;
// }

inline void OutputGate::setThresholds(int16_t openThreshold, int16_t closeThreshold)
{
    this->openThreshold = openThreshold;
    this->closeThreshold = closeThreshold;
}

#endif
