#ifndef Vca_h
#define Vca_h

#include "AudioStream.h"

class Vca : public AudioStream
{
public:
    Vca();
    void update(void);

private:
    audio_block_t *inputQueueArray[2];
    float multiplier = 1;
    float lowPassCoeff = 0.01;
};

inline Vca::Vca() : AudioStream(2, inputQueueArray)
{
}

inline void Vca::update(void)
{
    // Receive input data
    audio_block_t *block1, *block2;
    block1 = receiveReadOnly(0);
    block2 = receiveReadOnly(1);

    audio_block_t *outputBlock = allocate();

    // If outputBlock not available no point in processing data
    if (!outputBlock || !block1)
    {
        return;
    }

    // if receiving data on both input
    if (block1 && block2)
    {
        for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            // Converting the multiplier from -32768/32767 to 0/1
            float mult = ((float)block2->data[i] + INT16_MAX) / (float)(INT16_MAX * 2);

            // Update multipler with incoming data and lowpass
            this->multiplier = (this->lowPassCoeff * mult) + (1.0f - this->lowPassCoeff) * this->multiplier;

            // Set output data
            outputBlock->data[i] = block1->data[i] * this->multiplier;
        }

        release(block1);
        release(block2);
    }

    // if receiving only data on input 1
    else if (block1)
    {
        for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            outputBlock->data[i] = block1->data[i] * this->multiplier;
        }

        release(block1);
    }

    transmit(outputBlock, 0);
    release(outputBlock);
}

#endif
