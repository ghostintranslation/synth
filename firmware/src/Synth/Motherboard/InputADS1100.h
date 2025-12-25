#ifndef InputADS1100_h
#define InputADS1100_h

#include "Registrar.h"
#include "Input.h"

#include  <Wire.h>

/**************************************************************************
	I2C ADDRESS/BITS
**************************************************************************/
	#define ADS1100_DEFAULT_ADDRESS                       (0x48)    

/**************************************************************************
	CONFIGURATION REGISTER
**************************************************************************/
	#define ADS1100_START_SINGLE_CONVERSION				(0x80)	// In single conversion mode, writing causes a conversion to start, and writing a 0 has no effect. No effect in continuous
	
	#define ADS1100_CONVERSION_MASK							(0x10)	// single conversion mode. IF not set - continuous conversion mode
	#define ADS1100_CONVERSION_SINGLE						(0x10)	// single conversion mode. IF not set - continuous conversion mode
	#define ADS1100_CONVERSION_CONTINOUS					(0x00)	// single conversion mode. IF not set - continuous conversion mode
	
	#define ADS1100_DATA_RATE_MASK							(0x0C)	// ADS1100’s data rate 128 - 8 SPS. SPS = samples per second
	#define ADS1100_DATA_RATE_128SPS						(0x00)
	#define ADS1100_DATA_RATE_32SPS							(0x04)
	#define ADS1100_DATA_RATE_16SPS							(0x08)
	#define ADS1100_DATA_RATE_8SPS							(0x0C)
	
	#define ADS1100_GAIN_MASK									(0x03)	// ADS1100’s gain 1x to 8x
	#define ADS1100_GAIN_1X										(0x00)
	#define ADS1100_GAIN_2X										(0x01)
	#define ADS1100_GAIN_4X										(0x02)
	#define ADS1100_GAIN_8X										(0x03)

class InputADS1100 : public Input, public Registrar<InputADS1100>
{
public:
    InputADS1100(int8_t index);
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
    int16_t raw_adc;
};

inline InputADS1100::InputADS1100(int8_t index) : Input(index)
{
    this->setLowPassCoeff(1.0);

    uint8_t singleMode = ADS1100_CONVERSION_CONTINOUS;
    uint8_t dataRate = ADS1100_DATA_RATE_128SPS;
    uint8_t gain = ADS1100_GAIN_1X;

    singleMode	&= ADS1100_CONVERSION_MASK;
	dataRate		&= ADS1100_DATA_RATE_MASK;
	gain			&= ADS1100_GAIN_MASK;
	uint8_t	currentRegister = singleMode+dataRate+gain;
	Wire.begin();
	Wire.beginTransmission(ADS1100_DEFAULT_ADDRESS);
    Wire.send(currentRegister);
	Wire.endTransmission();

    delay(300);
}

inline void InputADS1100::update(void)
{
    Input::update();

    // Request 2 bytes of data
    Wire.requestFrom((uint8_t)ADS1100_DEFAULT_ADDRESS, (uint8_t)2);

}

inline int16_t InputADS1100::addSampleBefore(int16_t sample)
{
    unsigned int data[2] = {0};
    // Read 2 bytes of data
    // raw_adc msb, raw_adc lsb 
    if(Wire.available() >= 2)
    {
        data[0] = Wire.read();
        data[1] = Wire.read();
        
        raw_adc = ((data[0] << 8) + data[1]);
        // raw_adc = (data[0] << 8) + data[1];
    } 

    return raw_adc;
}

inline int16_t *&InputADS1100::updateBefore(int16_t *&blockData)
{
    return blockData;
}


inline void InputADS1100::onNoteChange(void (*onNoteCallback)(byte))
{
    this->onNoteCallback = onNoteCallback;
}

inline int16_t InputADS1100::scale_signed_12_to_16(int16_t val12)
{
    // Clamp to valid 12-bit signed range
    if (val12 > 2047)  val12 = 2047;
    if (val12 < -2048) val12 = -2048;

    // Scale proportionally: 32768 / 2048 = 16
    return (int16_t)((val12 * 32768) / 2048);
}

#endif
