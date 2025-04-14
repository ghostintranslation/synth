#ifndef OutputLed_h
#define OutputLed_h

#include "Registrar.h"
#include "Output.h"

class OutputLed : public Output, public Registrar<OutputLed>
{
public:
    enum Status
    {
        Off,
        On,
        Blink,
        BlinkFast,
        BlinkOnce,
        Default
    };

    OutputLed(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    void setStatus(OutputLed::Status status);

private:
    audio_block_t *inputQueueArray[1];
    uint8_t index;
    uint8_t multiplier = 0;
    Status status = Default;
    uint16_t blinkingDuration = 400.0 / 1000 / (1.0 / AUDIO_SAMPLE_RATE);     // 17640 samples at 44100Hz means 400ms
    uint16_t fastBlinkingDuration = 200.0 / 1000 / (1.0 / AUDIO_SAMPLE_RATE); // 8820 samples at 44100Hz means 200ms
    uint16_t blinkOnceDuration = 10.0 / 1000 / (1.0 / AUDIO_SAMPLE_RATE);     // 441 samples at 44100Hz means 10ms
    uint16_t statusSamplesleft = 0;
};

inline OutputLed::OutputLed(int8_t index) : Output(index)
{
    this->smoothing = 0.9995;
}

inline int16_t *&OutputLed::updateBefore(int16_t *&blockData)
{

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        switch (this->status)
        {
        case Blink:
            if (this->statusSamplesleft > this->blinkingDuration / 2)
            {
                blockData[i] = INT16_MIN;
            }
            else if (this->statusSamplesleft == 0)
            {
                this->statusSamplesleft = this->blinkingDuration;
            }
            else
            {
                blockData[i] = INT16_MAX;
            }
            break;

        case BlinkFast:
            if (this->statusSamplesleft > this->fastBlinkingDuration / 2)
            {
                blockData[i] = INT16_MIN;
            }
            else if (this->statusSamplesleft == 0)
            {
                this->statusSamplesleft = this->fastBlinkingDuration;
            }
            else
            {
                blockData[i] = INT16_MAX;
            }
            break;

        case BlinkOnce:
            if (this->statusSamplesleft > 0)
            {
                blockData[i] = INT16_MAX;
            }
            else
            {
                blockData[i] = INT16_MIN;
            }
            break;

        case Off:
            blockData[i] = INT16_MIN;
            break;

        case On:
            blockData[i] = INT16_MAX;
            break;

        default:
        case Default:
            break;
        }

        if (this->statusSamplesleft > 0)
        {
            this->statusSamplesleft--;
        }
    }

    return blockData;
}

inline void OutputLed::update(void)
{
    Output::update();
}

inline void OutputLed::setStatus(OutputLed::Status status)
{
    this->status = status;

    if (status == BlinkOnce)
    {
        this->statusSamplesleft = blinkOnceDuration;
    }
}
#endif
