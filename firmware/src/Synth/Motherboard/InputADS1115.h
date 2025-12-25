#ifndef InputADS1115_h
#define InputADS1115_h

#include "Registrar.h"
#include "Input.h"
#include "lib/ADS1X15/ADS1X15.h"

ADS1115 ADS(0x48);

class InputADS1115 : public Input, public Registrar<InputADS1115>
{
public:
    InputADS1115(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    int16_t addSampleBefore(int16_t sample);
    void onNoteChange(void (*onNoteCallback)(byte note));
    int16_t scale_signed_12_to_16(int16_t val12);

protected:
    byte note=0;
    uint16_t cycleCount = 0;
    void (*onNoteCallback)(byte) = nullptr;

    // MCP342X myADC;
    int16_t raw_adc=0;
};

inline InputADS1115::InputADS1115(int8_t index) : Input(index)
{
    this->setLowPassCoeff(1.0);

	Wire.begin();
    Wire.setClock(100000);

    ADS.begin();
    ADS.setGain(0);      //  6.144 volt
    ADS.setDataRate(7);  //  0 = slow   4 = medium   7 = fast
    ADS.setMode(0);      //  continuous mode
    ADS.readADC(0);      //  first read to trigger

    // ADS.requestADC(0);
}

inline void InputADS1115::update(void)
{
    Input::update();
    
    // For some reason if calling this in addSampleBefore Teensy crashes
    int16_t val = ADS.getValue(); 

    if(val < 0){
        val = 0;
    }

    // 27362 seems to be the max read
    // 26750 seems to be the magic number to obtain the same notes as with MIDI
    this->raw_adc = (float)val / 32768 * 65535 - 32768;
// Serial.println(this->raw_adc);
}

inline int16_t InputADS1115::addSampleBefore(int16_t sample)
{
    return this->raw_adc;
}

inline int16_t *&InputADS1115::updateBefore(int16_t *&blockData)
{
    return blockData;
}


inline void InputADS1115::onNoteChange(void (*onNoteCallback)(byte))
{
    this->onNoteCallback = onNoteCallback;
}

#endif
