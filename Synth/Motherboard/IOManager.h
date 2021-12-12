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
//using TriggerDownCallback = void (*)(byte);
//using LongTriggerDownCallback = void (*)(byte);
//using TriggerUpCallback = void (*)(byte);
//using LongTriggerUpCallback = void (*)(byte);
using EdgeCallback = void (*)(String);
using ChangeCallback = void (*)(String name, float value, float diff);
using ChangeQuantizedCallback = void (*)(byte inputIndex, int value);


//#include "IORegistrar.h"
#include "InputPotentiometer.h"
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

    float getInputValue(byte index);
//    float getOutputValue(byte index);
//    void setOutputValue(byte index, unsigned int value);
//    void setLED(byte index, Led::Status status, unsigned int brightness);
//    void setSmoothing(byte smoothing);
    unsigned int getAnalogMaxValue();
    unsigned int getAnalogMinValue();
    byte getDipswitchValue();

    
    // Callbacks
//    void setHandleTriggerDown(byte inputIndex, TriggerDownCallback fptr);
//    void setHandleLongTriggerDown(byte inputIndex, LongTriggerDownCallback fptr);
//    void setHandleTriggerUp(byte inputIndex, TriggerUpCallback fptr);
//    void setHandleLongTriggerUp(byte inputIndex, LongTriggerUpCallback fptr);
//    void setHandleChange(byte inputIndex, ChangeCallback fptr);
//    void setHandleChangeQuantized(byte inputIndex, ChangeQuantizedCallback fptr);

    // Registrar
    

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
    byte analogResolution = 12;
    byte dipswitchValue = 0;
    void iterateIO();
    void readWriteIO();
    
    static void handleMidiControlChange(byte channel, byte controlNumber, byte value);

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
    
    // Regitrar
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

    analogReadResolution(getInstance()->analogResolution);

    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();

//    for(unsigned int i = 0; i<IORegistrar::inputsSize; i++){
//      IORegistrar::getInputs()[i]->transitionTo(new IOStateDefault());
//    }
//    for(unsigned int i = 0; i<IORegistrar::outputsSize; i++){
//      IORegistrar::getOutputs()[i]->transitionTo(new IOStateDefault());
//    }
//    for(unsigned int i = 0; i<IORegistrar::ledsSize; i++){
//      IORegistrar::getLeds()[i]->transitionTo(new IOStateDefault());
//    }
    for(PhysicalInput* i : PhysicalInput::getEntities()){
      i->transitionTo(new IOStateDefault());
    }
    for(PhysicalOutput* i : PhysicalOutput::getEntities()){
      i->transitionTo(new IOStateDefault());
    }
    for(Led* i : Led::getEntities()){
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
//      for(unsigned int i = 0; i<IO::inputsSize; i++){
//        Serial.println(IO::getInputs()[i]->getClassName());
//      }
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
          // We switch now to calibrate mode
//          for(unsigned int i = 0; i<IORegistrar::inputsSize; i++){
//            IORegistrar::getInputs()[i]->transitionTo(new IOStateCalibrate);
//          }

          for(PhysicalInput* i : PhysicalInput::getEntities()){
            i->transitionTo(new IOStateCalibrate());
          }

//          for(unsigned int i = 0; i<IO::outputsSize; i++){
//            IO::getOutputs()[i]->transitionTo(new IOStateCalibrate());
//          }
//          for(unsigned int i = 0; i<IO::ledsSize; i++){
//            IO::getLeds()[i]->transitionTo(new IOStateCalibrate());
//          }
        }
      }
      if(this->calibrationSequenceClock > this->calibrationSequenceClockMax){
        this->calibrationSequenceClock = 0;
        this->calibrationSequenceCurrentStep = 0;
      }
    }else{
//      for(unsigned int i = 0; i<IORegistrar::ledsSize; i++){
//        IORegistrar::getLeds()[i]->set(Led::Status::Blink, 4095);
//      }
          for(Led* i : Led::getEntities()){
            i->set(Led::Status::Blink, 4095);
          }

      this->calibrate();
          
      if(this->getDipswitchValue() == 15){
        // Exit calibration
        this->calibrationSequenceClock = 0;
        this->calibrationSequenceCurrentStep = 0;
//        for(unsigned int i = 0; i<IORegistrar::ledsSize; i++){
//          IORegistrar::getLeds()[i]->set(Led::Status::Off, 0);
//        }
          for(Led* i : Led::getEntities()){
            i->set(Led::Status::Off, 0);
          }
        
//        for(unsigned int i = 0; i<IORegistrar::inputsSize; i++){
//          IORegistrar::getInputs()[i]->transitionTo(new IOStateDefault);
//        }
          for(PhysicalInput* i : PhysicalInput::getEntities()){
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
    }while(PhysicalInput::getEntities()[this->currentInputIndex]->getIndex() > this->maxIoNumber);
  }

//  if(IORegistrar::outputsSize > 0){
  if(PhysicalOutput::getCount() > 0){
    do{
      // Iterate to the next or Iterate back to the first one
      this->currentOutputIndex++;
      this->currentOutputIndex = this->currentOutputIndex % PhysicalOutput::getCount();

    // If current input Position is greater than the allowed max
    // then iterate again. This way we skip useless Inputs.
    }while(PhysicalOutput::getEntities()[this->currentOutputIndex]->getIndex() > this->maxIoNumber);
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
   
    unsigned int currentInputPosition = PhysicalInput::getCount() > this->currentInputIndex ? PhysicalInput::getEntities()[this->currentInputIndex]->getIndex() : 0;
    
    unsigned int currentOutputPosition = PhysicalOutput::getCount() > this->currentOutputIndex ? PhysicalOutput::getEntities()[this->currentOutputIndex]->getIndex() : 0;

    byte currentOutputDacIndex = currentOutputPosition / 2;
    byte currentOutputDacChannel = currentOutputPosition % 2;

    // Set the latch to low (activate the shift registers)
    digitalWrite(REGISTERS_LATCH_PIN, LOW);

    // Preparing the shift register data
    unsigned long shiftRegistersData =  0x00;
    if (PhysicalInput::getCount() > this->currentInputIndex && PhysicalInput::getEntities()[this->currentInputIndex]->isDirectToTeensy())
    {
        shiftRegistersData = 0x80;
    }
    
    shiftRegistersData = shiftRegistersData | currentInputPosition << 4 | (0x0F ^ 0x01 << currentOutputDacIndex);

    int ledsData = 0;

    // Preparing the LEDs data
    if(Led::getCount() > 0){
      for(unsigned int i = 0; i<Led::getCount(); i++){
        if (Led::getEntities()[i]->getValue() == 0)
        {
            continue;
        }
        else if (Led::getEntities()[i]->getValue() == 4095)
        {
            bitSet(ledsData, Led::getEntities()[i]->getIndex());
        }
        else if ((float)this->clockPWM / this->intervalPWM < (float)Led::getEntities()[i]->getValue() / 4095)
        {
            bitSet(ledsData, Led::getEntities()[i]->getIndex());
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
          SPI.transfer16(0x1000 | constrain((int)PhysicalOutput::getEntities()[this->currentOutputIndex]->getValue(), 0, 4095));
          SPI.endTransaction();
      }
      else
      {
          // Channel B
          SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
          SPI.transfer16(0x9000 | (int)PhysicalOutput::getEntities()[this->currentOutputIndex]->getValue());
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
      PhysicalInput::getEntities()[this->currentInputIndex]->read();
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


//inline void IOManager::setSmoothing(byte smoothing){
//  for(APhysicalInput* i : IO::getInputs()){
//    i->setSmoothing(smoothing);
//  }
//  for(APhysicalOutput* o : IO::getOutputs()){
//    o->setSmoothing(smoothing);
//  }
//}


///**
// * Handle press down on a button
// */
//inline void IOManager::setHandleTriggerDown(byte index, TriggerDownCallback fptr)
//{
//    this->inputs[index]->setTriggerDownCallback(fptr);
//}
//
///**
// * Handle press up on a button
// */
//inline void IOManager::setHandleTriggerUp(byte index, TriggerUpCallback fptr)
//{
//    this->inputs[index]->setTriggerUpCallback(fptr);
//}
//
///**
// * Handle long trigger down
// */
//inline void IOManager::setHandleLongTriggerDown(byte index, LongTriggerDownCallback fptr)
//{
//    this->inputs[index]->setLongTriggerDownCallback(fptr);
//}
//
///**
// * Handle long trigger up
// */
//inline void IOManager::setHandleLongTriggerUp(byte index, LongTriggerUpCallback fptr)
//{
//    this->inputs[index]->setLongTriggerUpCallback(fptr);
//}
//
///**
// * Handle change
// */
//inline void IOManager::setHandleChange(byte index, ChangeCallback fptr)
//{
//    this->inputs[index]->setChangeCallback(fptr);
//}
//
///**
// * Handle potentiometer
// */
//inline void IOManager::setHandleChangeQuantized(byte index, ChangeQuantizedCallback fptr)
//{
//    this->inputs[index]->setChangeQuantizedCallback(fptr);
//}

//inline void IOManager::setOutputValue(byte index, unsigned int value)
//{
//    IO::getOutputs()[index]->setTarget(value);
//}

//inline void IOManager::setLED(byte index, Led::Status status, unsigned int brightness = 4095)
//{
//    this->leds[index]->set(status, brightness);
//}

/**
 * Get input value
 * @param byte index The index of the input
 */
inline float IOManager::getInputValue(byte index)
{
//  if(IO::getInputs().size() > 0 && index < IO::getInputs().size()){
//    return IO::getInputs()[index]->getValue();
//  }else{
    return 0;
//  }
}

/**
 * Get output value
 * @param byte index The index of the output
 */
//inline float IOManager::getOutputValue(byte index)
//{
//  if(IO::getOutputs().size() > 0 && index < IO::getOutputs().size()){
//    return IO::getOutputs()[index]->getValue();
//  }else{
//    return 0;
//  }
//}

inline byte IOManager::getDipswitchValue()
{
    return this->dipswitchValue;
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

inline void IOManager::handleMidiControlChange(byte channel, byte controlNumber, byte value){
  //TODO: use the channel
  
//    for(APhysicalInput* i : IO::getInputs()){
//        if(i->getMidiControlNumber() == controlNumber){
//          int target = map(value, 0, 127, 0, 4095);
//          i->setTarget(target);
//        }
//    }
}

inline void IOManager::calibrate(){
  Serial.println("IOManager::calibrate()");
  for(unsigned int i = 0; i<PhysicalInput::getCount(); i++){
    if(PhysicalInput::getEntities()[i]->getClassName() == "InputPotentiometer"){
      
      if(PhysicalInput::getEntities()[i]->getTarget() > PhysicalInput::getEntities()[i]->getMin()){
        PhysicalInput::getEntities()[i]->setMin(PhysicalInput::getEntities()[i]->getTarget());
      }
      if(PhysicalInput::getEntities()[i]->getTarget() < PhysicalInput::getEntities()[i]->getMax()){
        PhysicalInput::getEntities()[i]->setMax(PhysicalInput::getEntities()[i]->getTarget());
      }
      
      Serial.printf("%d %d %d\n", i, PhysicalInput::getEntities()[i]->getMin(), PhysicalInput::getEntities()[i]->getMax());
    }
  }

}



inline void IOManager::print(){
  Serial.print("Inputs: ");
  for(unsigned int i = 0; i< PhysicalInput::getCount(); i++){
    PhysicalInput::getEntities()[i]->print();
    Serial.print(" / ");
  }
  Serial.println("");

  Serial.print("Outputs: ");
  for(unsigned int i = 0; i< PhysicalOutput::getCount(); i++){
    PhysicalOutput::getEntities()[i]->print();
    Serial.print(" / ");
  }

  Serial.println("");
  
  Serial.print("Leds: ");
  for(unsigned int i = 0; i< Led::getCount(); i++){
    Led::getEntities()[i]->print();
    Serial.print(" / ");
  }
  
  Serial.println("");
}



#endif
