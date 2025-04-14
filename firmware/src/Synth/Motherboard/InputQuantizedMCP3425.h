#ifndef InputQuantizedMCP3425_h
#define InputQuantizedMCP3425_h

#include "Registrar.h"
#include "Input.h"

#include  <Wire.h>
// #include  <MCP342X.h>

// #define Addr 0x68


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
	#define ADS1100_DATA_RATE_128SPS							(0x00)
	#define ADS1100_DATA_RATE_32SPS							(0x04)
	#define ADS1100_DATA_RATE_16SPS							(0x08)
	#define ADS1100_DATA_RATE_8SPS							(0x0C)
	
	#define ADS1100_GAIN_MASK									(0x03)	// ADS1100’s gain 1x to 8x
	#define ADS1100_GAIN_1X										(0x00)
	#define ADS1100_GAIN_2X										(0x01)
	#define ADS1100_GAIN_4X										(0x02)
	#define ADS1100_GAIN_8X										(0x03)

class InputQuantizedMCP3425 : public Input, public Registrar<InputQuantizedMCP3425>
{
public:
    InputQuantizedMCP3425(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    void onNoteChange(void (*onNoteCallback)(byte note));

protected:
    byte note=0;
    byte cycleCount = 0;
    void (*onNoteCallback)(byte) = nullptr;

    // MCP342X myADC;
    int16_t raw_adc;
};

inline InputQuantizedMCP3425::InputQuantizedMCP3425(int8_t index) : Input(index)
{
    this->setLowPassCoeff(0.1);
    // this->myADC.configure( MCP342X_MODE_CONTINUOUS |
    //                MCP342X_CHANNEL_1 |
    //                MCP342X_SIZE_16BIT |
    //                MCP342X_GAIN_1X
    //              );
    // myADC.startConversion();


//MCP
    // Initialise I2C communication as MASTER
    // Wire.begin();
    // // Start I2C Transmission
    // Wire.beginTransmission(Addr);
    // // Send configuration command
    // // Continuous conversion mode, 12-bit resolution
    // // Wire.write(0x10);
    // // Wire.write(0x18);
    // Wire.write(0x0C);
    // // Stop I2C Transmission
    // Wire.endTransmission();

//AD
    uint8_t singleMode = ADS1100_CONVERSION_CONTINOUS;
    uint8_t dataRate = ADS1100_DATA_RATE_8SPS;
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

inline void InputQuantizedMCP3425::update(void)
{
    Input::update();
//     myADC.checkforResult(&result);
//    Serial.println(this->result);

    unsigned int data[2] = {0};
  
    // Start I2C Transmission
    // Wire.beginTransmission(Addr);
    // // Select data register
    // Wire.write(0x02);
    // // Stop I2C Transmission
    // Wire.endTransmission();
    
    // Request 2 bytes of data
    Wire.requestFrom((uint8_t)ADS1100_DEFAULT_ADDRESS, (uint8_t)2);
    
    // Read 2 bytes of data
    // raw_adc msb, raw_adc lsb 
    if(Wire.available() >= 2)
    {
        data[0] = Wire.read();
        data[1] = Wire.read();
    }
    
    // Convert the data to 12-bits
    // int raw_adc = (data[0] & 0x0F) * 256 + data[1];
    raw_adc = (data[0] << 8) + data[1];
}

inline int16_t *&InputQuantizedMCP3425::updateBefore(int16_t *&blockData)
{
    if(raw_adc < 0){
        raw_adc = 0;
    }
    byte newNote = (byte)((float)raw_adc / 32767 *12 * 5);
      if(newNote != this->note){
            Serial.println(newNote);
            if(this->onNoteCallback){
                this->onNoteCallback(newNote);
            }
        }
        this->note = newNote;
        
    return blockData;
}


inline void InputQuantizedMCP3425::onNoteChange(void (*onNoteCallback)(byte))
{
    this->onNoteCallback = onNoteCallback;
}
#endif
