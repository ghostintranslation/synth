#ifndef MidiInput_h
#define MidiInput_h

#include "MidiIO.h"

// TODO: Add a change callback
using MidiControlChangeCallbackFunction = void (*)(int16_t);

class MidiInput : public MidiIO, public AudioStream
{
public:
    MidiInput(Setting *setting);
    void update(void);
    // TODO: setChannelSetting
};

inline MidiInput::MidiInput(Setting *setting = nullptr) : MidiIO(setting), AudioStream(0, NULL)
{
    this->active = true;
}

inline void MidiInput::update(void)
{
    audio_block_t *block;

    // allocate the audio blocks to transmit
    block = allocate();

    if (block)
    {
        int16_t newMappedValue = map(this->value, 0, 127, INT16_MIN, INT16_MAX);
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            this->mappedValue = (float)this->mappedValue * 0.9 + (float)newMappedValue * 0.1;
            block->data[i] = this->mappedValue;
            this->blockData[i] = this->mappedValue;
        }

        transmit(block, 0);
        release(block);
    }
}
#endif