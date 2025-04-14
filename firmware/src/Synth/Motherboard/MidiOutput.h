#ifndef MidiOutput_h
#define MidiOutput_h

#include "MidiIO.h"

class MidiOutput : public MidiIO, public AudioStream
{
public:
    MidiOutput(Setting *setting);
    void update(void);
    virtual void send() = 0;

private:
    uint32_t testval = 0;
    audio_block_t *inputQueueArray[1];
};

inline MidiOutput::MidiOutput(Setting *setting = nullptr) : MidiIO(setting), AudioStream(1, inputQueueArray)
{
    this->active = true;
}

inline void MidiOutput::update(void)
{
    // Receive input data
    audio_block_t *block;
    block = receiveReadOnly(0);

    if (block)
    {
        this->mappedValue = (float)this->mappedValue * 0.95 + (float)block->data[0] * 0.05;
        byte newValue = (this->mappedValue + INT16_MAX) >> 9;

        if (newValue != this->value)
        {
            this->value = newValue;
            this->send();
        }

        release(block);
    }
}

#endif