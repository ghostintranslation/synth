#ifndef Vcc_h
#define Vcc_h

#include "AudioStream.h"

/**
 * @brief Voltage Controlled Crossfader
 *
 */
class Vcc : public AudioStream
{
public:
    Vcc();
    void update(void);

private:
    audio_block_t *inputQueueArray[3];
    float crossfade = 0;
    float lowPassCoeff = 0.01;
};

inline Vcc::Vcc() : AudioStream(3, inputQueueArray)
{
}

inline void Vcc::update(void)
{
    // Receive input data
    audio_block_t *block1, *block2, *block3;
    block1 = receiveReadOnly(0);
    block2 = receiveReadOnly(1);
    block3 = receiveReadOnly(2);

    audio_block_t *outputBlock = allocate();

    // If outputBlock not available no point in processing data
    if (!outputBlock || !block1 || !block2)
    {
        return;
    }

    for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        if (block3)
        {
            // Converting the crossfade from -32768/32767 to 0/1
            float cf = ((float)block3->data[i] + INT16_MAX) / (float)(INT16_MAX * 2);

            // Update crossfade with incoming data and lowpass
            this->crossfade = (this->lowPassCoeff * cf) + (1.0f - this->lowPassCoeff) * this->crossfade;
        }

        // Set output data
        outputBlock->data[i] = block1->data[i] * (1 - this->crossfade) + block2->data[i] * this->crossfade;
    }

    release(block1);
    release(block2);

    if (block3)
    {
        release(block3);
    }

    transmit(outputBlock, 0);
    release(outputBlock);
}

#endif
