#ifndef IOManager_h
#define IOManager_h

// Teensy pins
#define MIDI_CHANNEL_A_PIN 2
#define MIDI_CHANNEL_B_PIN 3
#define MIDI_CHANNEL_C_PIN 4
#define MIDI_CHANNEL_D_PIN 5
#define REGISTERS_LATCH_PIN 9
#define SPI_CLOCK_PIN 13
#define SPI_MOSI_PIN 11
#define ANALOG_IN_1_PIN 14

// Callback types
using PressDownCallback = void (*)(byte);
using LongPressDownCallback = void (*)(byte);
using PressUpCallback = void (*)(byte);
using LongPressUpCallback = void (*)(byte);
using ChangeCallback = void (*)(byte inputIndex, float value, float diff);

#include "Potentiometer.h"
#include "Button.h"
#include "RotaryEncoder.h"
#include "TouchPad.h"
#include "CvIn.h"
#include "ToggleOnOn.h"
#include "ToggleOnOffOn.h"
#include "Led.h"
#include "CvOut.h"
#include "None.h"

class IOManager
{
public:

    // Init
    void init(byte columnNumber, std::vector<Input *> inputs, std::vector<Output *> outputs);

    // Update
    void update();

    // Debug
    void print();

    float getInputValue(byte index);
    float getOutputValue(byte index);
    void setOutputValue(byte index, unsigned int value);
    void setLED(byte index, Led::Status status, unsigned int brightness);
    unsigned int getAnalogMaxValue();
    unsigned int getAnalogMinValue();
    byte getMidiChannel();

    // Callbacks
    void setHandlePressDown(byte inputIndex, PressDownCallback fptr);
    void setHandleLongPressDown(byte inputIndex, LongPressDownCallback fptr);
    void setHandlePressUp(byte inputIndex, PressUpCallback fptr);
    void setHandleLongPressUp(byte inputIndex, LongPressUpCallback fptr);
    void setHandleChange(byte inputIndex, ChangeCallback fptr);

private:
    // Only Motherboard can access this instance
    friend class Motherboard;
  
    // Singleton
    static IOManager *getInstance();
    
    // Singleton
    static IOManager *instance;
    IOManager();

    // Inputs/Outputs
    int inputNumber;
    byte outputNumber;
    byte ledNumber;
    byte currentInputIndex;
    byte currentOutputIndex;

    Input **inputs;
    Output **outputs;
    Led **leds;
    byte analogResolution = 12;
    byte midiChannel;

    void iterateIO();
    void updateIO();
    void readWriteIO();

    // Refresh clock
    const unsigned int intervalRefresh = 2;
    elapsedMicros clockRefresh;

    // PWM clock
    const float intervalPWM = 20000;
    elapsedMicros clockPWM;
};

// Instance pre init
IOManager *IOManager::instance = nullptr;

/**
 * Constructor
 */
inline IOManager::IOManager()
{
}

/**
 * Singleton instance
 */
inline IOManager *IOManager::getInstance()
{
    if (!instance)
        instance = new IOManager;
    return instance;
}

inline void IOManager::init(byte columnNumber, std::vector<Input *> inputs, std::vector<Output *> outputs)
{
    Serial.println("init");

    byte i = 0;
    this->inputNumber = inputs.size();
    this->inputs = new Input *[this->inputNumber];
    for (Input *input : inputs)
    {
        this->inputs[i] = input;
        this->inputs[i]->setIndex(i);
        
        input->setPin(ANALOG_IN_1_PIN); //TODO: MAKE IT DYNAMIC
        i++;
    }

    i = 0;
    this->outputNumber = outputs.size();
    this->outputs = new Output *[this->outputNumber];
    for (Output *output : outputs)
    {
        this->outputs[i] = output;
        this->outputs[i]->setIndex(i);
        i++;
    }

    // Init of the leds
    this->ledNumber = columnNumber * 3;
    this->leds = new Led *[getInstance()->ledNumber];
    for (int i = 0; i < this->ledNumber; i++)
    {
        this->leds[i] = new Led();
        this->leds[i]->setIndex(i);
    }

    Serial.println("init2");

    // Pin modes
    pinMode(MIDI_CHANNEL_A_PIN, INPUT_PULLUP);
    pinMode(MIDI_CHANNEL_B_PIN, INPUT_PULLUP);
    pinMode(MIDI_CHANNEL_C_PIN, INPUT_PULLUP);
    pinMode(MIDI_CHANNEL_D_PIN, INPUT_PULLUP);
    pinMode(REGISTERS_LATCH_PIN, OUTPUT);
    pinMode(SPI_CLOCK_PIN, OUTPUT);
    pinMode(SPI_MOSI_PIN, OUTPUT);

    analogReadResolution(getInstance()->analogResolution);

    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();

    Serial.println("init3");
}

/**
 * Update
 */
inline void IOManager::update()
{
    if (this->clockRefresh >= this->intervalRefresh)
    {
        // Read the current input and set the current output
        this->readWriteIO();

        // Update all inputs and outputs
        this->updateIO();

        // Iterate to the next input and output
        this->iterateIO();

        this->clockRefresh = 0;
    }
}

/**
 * Iterate over the inputs
 */
inline void IOManager::iterateIO()
{
    if(this->inputNumber > 0){
      this->currentInputIndex++;
      this->currentInputIndex = this->currentInputIndex % this->inputNumber;
      while (this->inputs[this->currentInputIndex]->getType() == "None" && this->currentInputIndex < (this->inputNumber - 1))
      {
          this->currentInputIndex++;
      }
    }

    if(this->outputNumber > 0){
      this->currentOutputIndex++;
      this->currentOutputIndex = this->currentOutputIndex % this->outputNumber;
      while (this->outputs[this->currentOutputIndex]->getType() == "None" && this->currentOutputIndex < (this->outputNumber - 1))
      {
          this->currentOutputIndex++;
      }
    }
}

/**
 * Send data to the chips to set the LEDs, the DAC, and read the inputs
 */
inline void IOManager::readWriteIO()
{
    /**
   * The data transfer goes like this:
   * - Set the latch to low (activate the shift registers)
   * - Send the byte to set the MUX and select the DAC
   * - Send the leds byte to complete the shift registers chain
   * - Set the latch to high (shift registers actually set their pins and stop listening)
   * - Send the selected DAC data
   * - Set the latch to low (now shift registers are listening)
   * - Send the same data to the shift register except the DAC selection bit goes to high (the DAC will actually set its pin)
   * - Send the leds byte to complete the shift registers chain
   * - Set the latch to high  (shift registers actually set their pins and stop listening)
   */

    byte currentOutputDacIndex = this->currentOutputIndex / 2;
    byte currentOutputDacChannel = this->currentOutputIndex % 2;

    // Set the latch to low (activate the shift registers)
    digitalWrite(REGISTERS_LATCH_PIN, LOW);

    // Preparing the shift register data
    unsigned long shiftRegistersData;
    if (this->inputs[this->currentInputIndex]->needsGround())
    {
        shiftRegistersData = 0x80;
    }
    else
    {
        shiftRegistersData = 0x00;
    }
    shiftRegistersData = shiftRegistersData | this->currentInputIndex << 4 | (0x0F ^ 0x01 << currentOutputDacIndex);

    int ledsData = 0;

    // Preparing the LEDs data
    for (int i = 0; i < this->ledNumber; i++)
    {
        if (this->leds[i]->getValue() == 0)
        {
            continue;
        }
        else if (this->leds[i]->getValue() == 4095)
        {
            bitSet(ledsData, this->leds[i]->getIndex());
        }
        else if ((float)this->clockPWM / this->intervalPWM < (float)this->leds[i]->getValue() / 4095)
        {
            bitSet(ledsData, this->leds[i]->getIndex());
        }
    }

    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    // Send the byte to set the MUX and select the DAC
    SPI.transfer(shiftRegistersData);
    // Send the leds byte to complete the shift registers chain
    SPI.transfer(ledsData);
    SPI.endTransaction();

    // Set the latch to high (shift registers actually set their pins and stop listening)
    digitalWrite(REGISTERS_LATCH_PIN, HIGH);

    // Send the selected DAC data
    if(this->outputNumber > 0 && this->outputs[this->currentOutputIndex]->getType() != "None"){
      if (currentOutputDacChannel == 0)
      {
          // Channel A
          SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
          SPI.transfer16(0x1000 | (int)this->outputs[this->currentOutputIndex]->getValue());
          SPI.endTransaction();
      }
      else
      {
          // Channel B
          SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
          SPI.transfer16(0x9000 | (int)this->outputs[this->currentOutputIndex]->getValue());
          SPI.endTransaction();
      }
    }

    // Set the latch to low (now shift registers are listening)
    digitalWrite(REGISTERS_LATCH_PIN, LOW);

    // Send the same data to the shift registers except the DAC selection bit goes to high (the DAC will actually set its pin)
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(shiftRegistersData ^ 0x01 << currentOutputDacIndex);
    SPI.transfer(ledsData);
    SPI.endTransaction();

    // Set the latch to high  (shift registers actually set their pins and stop listening)
    digitalWrite(REGISTERS_LATCH_PIN, HIGH);

    // Read the current input
    this->inputs[this->currentInputIndex]->read();
}

inline void IOManager::updateIO()
{

    if (clockPWM > intervalPWM)
    {
        clockPWM = 0;
    }

    for (int i = 0; i < this->inputNumber; i++)
    {
        inputs[i]->update(this->intervalRefresh);
    }
    for (int i = 0; i < this->outputNumber; i++)
    {
        outputs[i]->update(this->intervalRefresh);
    }
    for (int i = 0; i < this->ledNumber; i++)
    {
        leds[i]->update(this->intervalRefresh);
    }
}

/**
 * Handle press down on a button
 */
inline void IOManager::setHandlePressDown(byte index, PressDownCallback fptr)
{
    this->inputs[index]->setPressDownCallback(fptr);
}

/**
 * Handle press up on a button
 */
inline void IOManager::setHandlePressUp(byte index, PressUpCallback fptr)
{
    this->inputs[index]->setPressUpCallback(fptr);
}

/**
 * Handle long press down on a button
 */
inline void IOManager::setHandleLongPressDown(byte index, LongPressDownCallback fptr)
{
    this->inputs[index]->setLongPressDownCallback(fptr);
}

/**
 * Handle long press up on a button
 */
inline void IOManager::setHandleLongPressUp(byte index, LongPressUpCallback fptr)
{
    this->inputs[index]->setLongPressUpCallback(fptr);
}

/**
 * Handle potentiometer
 */
inline void IOManager::setHandleChange(byte index, ChangeCallback fptr)
{
    this->inputs[index]->setChangeCallback(fptr);
}

inline void IOManager::setOutputValue(byte index, unsigned int value)
{
    this->outputs[index]->setTarget(value);
}

inline void IOManager::setLED(byte index, Led::Status status, unsigned int brightness = 4095)
{
    this->leds[index]->set(status, brightness);
}

/**
 * Get input value
 * @param byte index The index of the input
 */
inline float IOManager::getInputValue(byte index)
{
  if(this->inputNumber > 0 && index < this->inputNumber){
    return this->inputs[index]->getValue();
  }else{
    return 0;
  }
}

/**
 * Get output value
 * @param byte index The index of the output
 */
inline float IOManager::getOutputValue(byte index)
{
  if(this->outputNumber > 0 && index < this->outputNumber){
    return this->outputs[index]->getValue();
  }else{
    return 0;
  }
}

inline byte IOManager::getMidiChannel()
{
    return this->midiChannel;
}

/**
 * Get min analog value according to resolution
 */
inline unsigned int IOManager::getAnalogMinValue()
{
    return 0;
}

/**
 * Get max analog value according to resolution
 */
inline unsigned int IOManager::getAnalogMaxValue()
{
    return (1 << this->analogResolution) - 1;
}

inline void IOManager::print(){
  Serial.print("Inputs: ");
  for(int i=0; i<this->inputNumber; i++){
    this->inputs[i]->print();
    Serial.print(" ");
  }
  Serial.println("");

  Serial.print("Outputs: ");
  for(int i=0; i<this->outputNumber; i++){
    this->outputs[i]->print();
  }
  Serial.println("");
}

#endif
