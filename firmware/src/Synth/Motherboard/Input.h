#ifndef Input_h
#define Input_h

#include <ADC.h>
#include "DMAChannel.h"
#include "AudioStream.h"
#include "MidiInput.h"

/**
   Teensy 4.0 audio library analog inputs multiplexing.

   This class samples up to 16 inputs at 44.1kHz, or up to 32 inputs at 22.05kHz.

   Connect four 8:1 multiplexers like the CD74HCT4051 to A0, A1, A2, A3.
   Connect the 3 selector bits of the multiplexers of A0 and A2 to pins 2,3,4 and the 2 other to pins 5,6,10.
   Connect your inputs to the multiplexers. Make sure they don't go over 3.3v!
*/
class Input : public AudioStream
{
public:
    Input(byte index);
    void update(void);
    virtual int16_t *&updateBefore(int16_t *&blockData) { return blockData; };
    void setLowPassCoeff(float coeff);
    void setMidiInput(MidiInput *midiInput);
    void onChange(void (*onChangeCallback)(int16_t value));

protected:
    byte index;
    int16_t *readBuffer();
    MidiInput *midiInput = nullptr;

    static unsigned int muxIndex1;
    static unsigned int muxIndex2;
    static unsigned int inputsRealCount;
    static unsigned int inputsCount;
    static uint8_t downSamplingFactor;
    static unsigned int buffSize;
    static const unsigned int inputsMax = 32;
    static const unsigned int maxBuffers = 8;
    static float accumulator[inputsMax];
    static float prevAccumulator[inputsMax];
    static float lowPassCoeff[inputsMax];
    static int16_t queue[inputsMax][maxBuffers][AUDIO_BLOCK_SAMPLES];
    static uint16_t head[inputsMax];
    static uint16_t headQueueTempCount[inputsMax];
    static int16_t headQueueTemp[inputsMax][AUDIO_BLOCK_SAMPLES];
    static uint16_t tail[inputsMax];
    static ADC *adc;
    static DMAChannel dmaChannel1;
    static DMAChannel dmaChannel2;
    static uint8_t isr1Count;
    static uint8_t isr2Count;
    static uint8_t pinToChannel[4];
    static void adc0Isr();
    static void adc1Isr();
    static void addSample(uint16_t val, uint8_t inputIndex);
    static void iterateMux1();
    static void iterateMux2();
    static uint16_t adc1Val;
    static uint16_t adc2Val;

    void (*onChangeCallback)(int16_t value) = nullptr;
};

// Static initializations
// bool Input::updateStarted = false;
unsigned int Input::muxIndex1 = 0;
unsigned int Input::muxIndex2 = 0;
unsigned int Input::inputsRealCount = 0;
unsigned int Input::inputsCount = 0;
uint8_t Input::downSamplingFactor = 0;
unsigned int Input::buffSize = AUDIO_BLOCK_SAMPLES;
int16_t Input::queue[inputsMax][maxBuffers][AUDIO_BLOCK_SAMPLES] = {{{0}}};
uint16_t Input::head[inputsMax] = {0};
uint16_t Input::headQueueTempCount[inputsMax] = {0};
int16_t Input::headQueueTemp[inputsMax][AUDIO_BLOCK_SAMPLES] = {{0}};
uint16_t Input::tail[inputsMax] = {0};
float Input::accumulator[inputsMax] = {0};
float Input::prevAccumulator[inputsMax] = {0};
float Input::lowPassCoeff[inputsMax] = {1.0f};
ADC *Input::adc = nullptr;
DMAChannel Input::dmaChannel1;
DMAChannel Input::dmaChannel2;

uint8_t Input::isr1Count = 0;
uint8_t Input::isr2Count = 0;
uint8_t Input::pinToChannel[4] = {
    7,  // 14/A0  AD_B1_02
    8,  // 15/A1  AD_B1_03
    12, // 16/A2  AD_B1_07
    11, // 17/A3  AD_B1_06
};
uint16_t Input::adc1Val = 0;
uint16_t Input::adc2Val = 0;

inline Input::Input(byte index)
    : AudioStream(0, NULL)
{
    this->index = index;
    this->active = true;
    inputsRealCount++;
    inputsCount = ((inputsRealCount - 1) / 8 + 1) * 8;
    downSamplingFactor = inputsCount > 16 ? 2 : 1;
    //  buffSize = AUDIO_BLOCK_SAMPLES / downSamplingFactor;

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(10, OUTPUT);

    // Reset multiplexer to channel 0
    digitalWriteFast(2, LOW);
    digitalWriteFast(3, LOW);
    digitalWriteFast(4, LOW);
    digitalWriteFast(5, LOW);
    digitalWriteFast(6, LOW);
    digitalWriteFast(10, LOW);

    adc = new ADC();

    adc->adc0->setAveraging(1);   // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
    //  adc->adc0->enableInterrupts(adc0Isr);
    adc->adc0->enableDMA();
    adc->adc0->startSingleRead(A0);

    dmaChannel1.source(ADC1_R0);
    dmaChannel1.destination((volatile uint16_t &)adc1Val);
    dmaChannel1.transferSize(2);
    dmaChannel1.transferCount(1);
    dmaChannel1.interruptAtCompletion();
    dmaChannel1.attachInterrupt(adc0Isr);
    dmaChannel1.triggerAtHardwareEvent(DMAMUX_SOURCE_ADC1);
    dmaChannel1.enable();

    if (inputsCount > 8)
    {
        adc->adc1->setAveraging(1);   // set number of averages
        adc->adc1->setResolution(12); // set bits of resolution
        adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
        adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
        //    adc->adc1->enableInterrupts(adc1Isr);
        adc->adc1->enableDMA();
        adc->adc1->startSingleRead(A1);

        dmaChannel2.source(ADC2_R0);
        dmaChannel2.destination((volatile uint16_t &)adc2Val);
        dmaChannel2.transferSize(2);
        dmaChannel2.transferCount(1);
        dmaChannel2.interruptAtCompletion();
        dmaChannel2.attachInterrupt(adc1Isr);
        dmaChannel2.triggerAtHardwareEvent(DMAMUX_SOURCE_ADC2);
        dmaChannel2.enable();
    }

    // Start conversions
    adc->adc0->startTimer(AUDIO_SAMPLE_RATE * 8);
    if (inputsCount > 8)
    {
        adc->adc1->startTimer(AUDIO_SAMPLE_RATE * 8);
    }

    lowPassCoeff[index] = 0.0005;
    accumulator[index] = INT16_MIN;
    headQueueTemp[index][0] = INT16_MIN;
    prevAccumulator[index] = 0;
}

inline void Input::update(void)
{
    // TODO: Interpolate instead of filling the remainings
    // If the sampling was too slow then samples are missing, filling the remaining with the last value
    if (headQueueTempCount[this->index] < buffSize)
    {
        for (unsigned int i = headQueueTempCount[this->index]; i < buffSize; i++)
        {
            headQueueTemp[this->index][i] = headQueueTemp[this->index][i - 1];
            headQueueTempCount[this->index]++;
        }
    }

    // Try to move head, we'll see if that works with the tail just bellow before actually doing it
    uint32_t h = head[this->index] + 1;

    // Ciruclar buffer, the head goes back to index 0 after reaching the max
    if (h >= maxBuffers)
    {
        h = 0;
    }

    if (h != tail[this->index])
    {
        // The temp buffer is full and head is in a good position to write
        // let's write the temp buffer in the queue
        memcpy(queue[this->index][head[this->index]], headQueueTemp[this->index], AUDIO_BLOCK_SAMPLES * sizeof *headQueueTemp[this->index]);

        // Reset also the samples count of the temp buffer
        headQueueTempCount[this->index] = 0;

        // Moving the head
        head[this->index] = h;
    }

    audio_block_t *block;

    // allocate the audio blocks to transmit
    block = allocate();

    // Raw output
    if (block)
    {
        int16_t *inputBuffer = this->readBuffer();

        if (inputBuffer != NULL)
        {
            // Allows for derived class to alter the data before being transmitted
            inputBuffer = this->updateBefore(inputBuffer);
            memcpy(block->data, inputBuffer, AUDIO_BLOCK_SAMPLES * sizeof *inputBuffer);

            if (this->onChangeCallback && prevAccumulator[this->index] != accumulator[this->index])
            {
                this->onChangeCallback(accumulator[this->index]);
                prevAccumulator[this->index] = accumulator[this->index];
            }
        }

        if (this->midiInput != nullptr)
        {
            // Combining the MIDI input's value with the input's value
            int16_t *midiBlockData = this->midiInput->getBlockData();

            for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                block->data[i] = constrain((block->data[i] + INT16_MAX) + (midiBlockData[i] + INT16_MAX) - INT16_MAX, INT16_MIN, INT16_MAX);
            }
        }

        transmit(block, 0);
        release(block);
    }
}

inline int16_t *Input::readBuffer()
{
    uint32_t t = tail[this->index];
    int16_t *result = queue[this->index][tail[this->index]];

    if (++t >= maxBuffers)
    {
        t = 0;
    }

    tail[this->index] = t;

    return result;
}

elapsedMicros count;

inline void Input::adc0Isr()
{
    if (isr1Count == 0)
    {
        addSample(adc1Val, muxIndex1);

        if (inputsCount > 16)
        {
            // ADC1_HC0 = pinToChannel[2] | ADC_HC_AIEN;
            adc->adc0->startSingleRead(A2);
            isr1Count = 1;
        }
        else
        {
            iterateMux1();
        }
    }
    else if (isr1Count == 1)
    {
        addSample(adc1Val, muxIndex1 + 16);
        iterateMux1();
        isr1Count = 0;
        // ADC1_HC0 = pinToChannel[0] | ADC_HC_AIEN;
        adc->adc0->startSingleRead(A0);
    }

    dmaChannel1.clearInterrupt();
    asm("DSB");
}

inline void Input::adc1Isr()
{
    if (isr2Count == 0)
    {
        addSample(adc2Val, muxIndex2 + 8);

        if (inputsCount > 24)
        {
            // ADC2_HC0 = pinToChannel[3] | ADC_HC_AIEN;
            adc->adc1->startSingleRead(A3);
            isr2Count = 1;
        }
        else
        {
            iterateMux2();
        }
    }
    else if (isr2Count == 1)
    {
        addSample(adc2Val, muxIndex2 + 24);
        iterateMux2();
        isr2Count = 0;
        // ADC2_HC0 = pinToChannel[1] | ADC_HC_AIEN;
        adc->adc1->startSingleRead(A1);
    }

    dmaChannel2.clearInterrupt();
    asm("DSB");
}

inline void Input::iterateMux1()
{
    muxIndex1++;
    muxIndex1 = muxIndex1 % 8;

    digitalWriteFast(2, muxIndex1 & 1);
    digitalWriteFast(3, muxIndex1 & 2);
    digitalWriteFast(4, muxIndex1 & 4);
}

inline void Input::iterateMux2()
{
    muxIndex2++;
    muxIndex2 = muxIndex2 % 8;

    digitalWriteFast(5, muxIndex2 & 1);
    digitalWriteFast(6, muxIndex2 & 2);
    digitalWriteFast(10, muxIndex2 & 4);
}

inline void Input::setLowPassCoeff(float coeff)
{
    if (coeff < 0.0f)
    {
        coeff = 0;
    }
    if (coeff > 1.0f)
    {
        coeff = 1;
    }
    lowPassCoeff[this->index] = coeff;
}

inline void Input::addSample(uint16_t val, uint8_t inputIndex)
{
    if (inputIndex >= inputsCount)
    {
        return;
    }

    if (headQueueTempCount[inputIndex] >= buffSize)
    {
        return;
    }

    int32_t newVal = val * 16 - 32768;
    newVal = constrain(newVal, -32300, 32750);
    newVal = (float)(newVal + 32300) / (32300 + 32750) * UINT16_MAX - INT16_MIN;
    if (newVal < INT16_MIN)
    {
        newVal = INT16_MIN;
    }

    for (uint8_t i = 0; i < downSamplingFactor; i++)
    {
        accumulator[inputIndex] = (lowPassCoeff[inputIndex] * newVal) + (1.0f - lowPassCoeff[inputIndex]) * accumulator[inputIndex];
        headQueueTemp[inputIndex][headQueueTempCount[inputIndex]] = accumulator[inputIndex];
        headQueueTempCount[inputIndex]++;
    }
}

inline void Input::setMidiInput(MidiInput *midiInput)
{
    this->midiInput = midiInput;
}

inline void Input::onChange(void (*onChangeCallback)(int16_t value))
{
    this->onChangeCallback = onChangeCallback;
}

#endif
