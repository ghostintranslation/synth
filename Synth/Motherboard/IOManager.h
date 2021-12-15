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

#define ANALOG_RESOLUTION 12
#define ABSOLUTE_ANALOG_MIN 0
#define ABSOLUTE_ANALOG_MAX 4095


// Callback types
//using TriggerDownCallback = void (*)(byte);
//using LongTriggerDownCallback = void (*)(byte);
//using TriggerUpCallback = void (*)(byte);
//using LongTriggerUpCallback = void (*)(byte);
using EdgeCallback = void (*)(String);
using ChangeCallback = void (*)(String name, float value, float diff);
using ChangeQuantizedCallback = void (*)(byte inputIndex, int value);

#include "InputPotentiometer.h"
#include "InputJack.h"
#include "InputMidiNote.h"
#include "OutputJack.h"
#include "Led.h"

class IOManager
{
public:

    // Init
    void init(byte columnNumber);

    // Update
    void update();

    // Debug
    void print();

    byte getDipswitchValue();

private:
    // Only Motherboard can access this instance
    friend class Motherboard;
  
    // Singleton
    static IOManager *getInstance();
    
    // Singleton
    static IOManager *instance;
    IOManager();

    // Inputs/Outputs
    byte maxIoNumber = 0;
    byte currentInputIndex = 0;
    byte currentOutputIndex = 0;
    byte dipswitchValue = 0;
    void iterateIO();
    void readWriteIO();
    
    // Calibration
    byte calibrationSequenceSteps[4]= {15, 7, 15, 7};
    byte calibrationSequenceCurrentStep = 0;
    elapsedMillis calibrationSequenceClock;
    const unsigned int calibrationSequenceClockMax = 5000;
    void calibrate();
    
    // Refresh clock
    const unsigned int intervalReadWrite = 4;
    elapsedMicros clockReadWrite;
    const unsigned int intervalIteration = 20000;
    elapsedMicros clockIteration;

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

inline void IOManager::init(byte columnNumber)
{
    this->maxIoNumber = columnNumber * 3 + columnNumber;

    // Pin modes
    pinMode(MIDI_CHANNEL_A_PIN, INPUT_PULLUP);
    pinMode(MIDI_CHANNEL_B_PIN, INPUT_PULLUP);
    pinMode(MIDI_CHANNEL_C_PIN, INPUT_PULLUP);
    pinMode(MIDI_CHANNEL_D_PIN, INPUT_PULLUP);
    pinMode(REGISTERS_LATCH_PIN, OUTPUT);
    pinMode(SPI_CLOCK_PIN, OUTPUT);
    pinMode(SPI_MOSI_PIN, OUTPUT);

    analogReadResolution(ANALOG_RESOLUTION);

    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();

    for(PhysicalInput* i : PhysicalInput::getAll()){
      i->transitionTo(new IOStateDefault());
    }
    for(PhysicalOutput* i : PhysicalOutput::getAll()){
      i->transitionTo(new IOStateDefault());
    }
    for(Led* i : Led::getAll()){
      i->transitionTo(new IOStateDefault());
    }
}

/**
 * Update
 */
inline void IOManager::update()
{
    if (this->clockIteration >= this->intervalIteration)
    {
        // Iterate to the next input and output
        this->iterateIO();

        this->clockIteration = 0;
    }
    
    if (this->clockReadWrite >= this->intervalReadWrite)
    {
        // Read the current input and set the current output
        this->readWriteIO();

        this->clockReadWrite = 0;
    }

    if (clockPWM > intervalPWM)
    {
        clockPWM = 0;
    }

    
    if(this->calibrationSequenceCurrentStep == 0){
      this->calibrationSequenceClock = 0;
    }
    if(this->calibrationSequenceCurrentStep < 4){
      if(this->calibrationSequenceSteps[this->calibrationSequenceCurrentStep] == this->getDipswitchValue()){
        this->calibrationSequenceCurrentStep++;
        this->calibrationSequenceCurrentStep %= 5;
        this->calibrationSequenceClock = 0;

        if(this->calibrationSequenceCurrentStep == 4){
          for(PhysicalInput* i : PhysicalInput::getAll()){
            i->transitionTo(new IOStateCalibrate());
          }
        }
      }
      if(this->calibrationSequenceClock > this->calibrationSequenceClockMax){
        this->calibrationSequenceClock = 0;
        this->calibrationSequenceCurrentStep = 0;
      }
    }else{
      for(Led* i : Led::getAll()){
        i->set(Led::Status::Blink, ABSOLUTE_ANALOG_MAX);
      }

      this->calibrate();
          
      if(this->getDipswitchValue() == 15){
        // Exit calibration
        this->calibrationSequenceClock = 0;
        this->calibrationSequenceCurrentStep = 0;
          for(Led* i : Led::getAll()){
            i->set(Led::Status::Off, 0);
          }
          
          for(PhysicalInput* i : PhysicalInput::getAll()){
            i->transitionTo(new IOStateDefault());
          }
      }
    }
    
}

/**
 * Iterate over the inputs
 */
inline void IOManager::iterateIO()
{  
//  if(IORegistrar::inputsSize > 0){
  if(PhysicalInput::getCount() > 0){
    do{
      // Iterate to the next or Iterate back to the first one
      this->currentInputIndex++;
      this->currentInputIndex = this->currentInputIndex % PhysicalInput::getCount();

    // If current input Position is greater than the allowed max
    // then iterate again. This way we skip useless Inputs.
    }while(PhysicalInput::getAll()[this->currentInputIndex]->getIndex() > this->maxIoNumber);
  }

//  if(IORegistrar::outputsSize > 0){
  if(PhysicalOutput::getCount() > 0){
    do{
      // Iterate to the next or Iterate back to the first one
      this->currentOutputIndex++;
      this->currentOutputIndex = this->currentOutputIndex % PhysicalOutput::getCount();

    // If current input Position is greater than the allowed max
    // then iterate again. This way we skip useless Inputs.
    }while(PhysicalOutput::getAll()[this->currentOutputIndex]->getIndex() > this->maxIoNumber);
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
   
    unsigned int currentInputPosition = PhysicalInput::getCount() > this->currentInputIndex ? PhysicalInput::getAll()[this->currentInputIndex]->getIndex() : 0;
    
    unsigned int currentOutputPosition = PhysicalOutput::getCount() > this->currentOutputIndex ? PhysicalOutput::getAll()[this->currentOutputIndex]->getIndex() : 0;

    byte currentOutputDacIndex = currentOutputPosition / 2;
    byte currentOutputDacChannel = currentOutputPosition % 2;

    // Set the latch to low (activate the shift registers)
    digitalWrite(REGISTERS_LATCH_PIN, LOW);

    // Preparing the shift register data
    unsigned long shiftRegistersData =  0x00;
    if (PhysicalInput::getCount() > this->currentInputIndex && PhysicalInput::getAll()[this->currentInputIndex]->isDirectToTeensy())
    {
        shiftRegistersData = 0x80;
    }
    
    shiftRegistersData = shiftRegistersData | currentInputPosition << 4 | (0x0F ^ 0x01 << currentOutputDacIndex);

    int ledsData = 0;

    // Preparing the LEDs data
    if(Led::getCount() > 0){
      for(unsigned int i = 0; i<Led::getCount(); i++){
        if (Led::getAll()[i]->getValue() == ABSOLUTE_ANALOG_MIN)
        {
            continue;
        }
        else if (Led::getAll()[i]->getValue() == ABSOLUTE_ANALOG_MAX)
        {
            bitSet(ledsData, Led::getAll()[i]->getIndex());
        }
        else if ((float)this->clockPWM / this->intervalPWM < (float)Led::getAll()[i]->getValue() / ABSOLUTE_ANALOG_MAX)
        {
            bitSet(ledsData, Led::getAll()[i]->getIndex());
        }
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
    if(PhysicalOutput::getCount() > 0){
      if (currentOutputDacChannel == 0)
      {
          // Channel A
          SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
          SPI.transfer16(0x1000 | constrain((int)PhysicalOutput::getAll()[this->currentOutputIndex]->getValue(), ABSOLUTE_ANALOG_MIN, ABSOLUTE_ANALOG_MAX)); // TODO: do we need the constrain here? not done earlier already?
          SPI.endTransaction();
      }
      else
      {
          // Channel B
          SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
          SPI.transfer16(0x9000 | (int)PhysicalOutput::getAll()[this->currentOutputIndex]->getValue());
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
    if(PhysicalInput::getCount() > this->currentInputIndex){
      PhysicalInput::getAll()[this->currentInputIndex]->read();
    }

    // Read the dipswitch
    this->dipswitchValue = 0;
    bool bit1 = digitalRead(MIDI_CHANNEL_A_PIN);
    bool bit2 = digitalRead(MIDI_CHANNEL_B_PIN);
    bool bit3 = digitalRead(MIDI_CHANNEL_C_PIN);
    bool bit4 = digitalRead(MIDI_CHANNEL_D_PIN);
    if(!bit1){
      bitSet(this->dipswitchValue, 0);
    }
    if(!bit2){
      bitSet(this->dipswitchValue, 1);
    }
    if(!bit3){
      bitSet(this->dipswitchValue, 2);
    }
    if(!bit4){
      bitSet(this->dipswitchValue, 3);
    }
}

inline byte IOManager::getDipswitchValue()
{
    return this->dipswitchValue;
}

inline void IOManager::calibrate(){
  Serial.println("IOManager::calibrate()");
  for(unsigned int i = 0; i<PhysicalInput::getCount(); i++){
    if(PhysicalInput::getAll()[i]->getClassName() == "InputPotentiometer"){
      
      if(PhysicalInput::getAll()[i]->getTarget() > PhysicalInput::getAll()[i]->getMin()){
        PhysicalInput::getAll()[i]->setMin(PhysicalInput::getAll()[i]->getTarget());
      }
      if(PhysicalInput::getAll()[i]->getTarget() < PhysicalInput::getAll()[i]->getMax()){
        PhysicalInput::getAll()[i]->setMax(PhysicalInput::getAll()[i]->getTarget());
      }
      
      Serial.printf("%d %d %d\n", i, PhysicalInput::getAll()[i]->getMin(), PhysicalInput::getAll()[i]->getMax());
    }
  }

}

inline void IOManager::print(){
  Serial.print("Inputs: ");
  for(unsigned int i = 0; i< PhysicalInput::getCount(); i++){
    PhysicalInput::getAll()[i]->print();
    Serial.print(" / ");
  }
  Serial.println("");

  Serial.print("Outputs: ");
  for(unsigned int i = 0; i< PhysicalOutput::getCount(); i++){
    PhysicalOutput::getAll()[i]->print();
    Serial.print(" / ");
  }

  Serial.println("");
  
  Serial.print("Leds: ");
  for(unsigned int i = 0; i< Led::getCount(); i++){
    Led::getAll()[i]->print();
    Serial.print(" / ");
  }
  
  Serial.println("");
}

#endif
