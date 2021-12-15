#ifndef PhysicalInput_h
#define PhysicalInput_h

#include "Registrar.h"
#include "PhysicalIO.h"

// TODO: SET THE NUMBER OF STREAM INPUTS to 0
class PhysicalInput : public PhysicalIO, public Registrar<PhysicalInput>
{
public:
    PhysicalInput(int index, String name);
    
    // Depending on the type of PhysicalInput, the way to read it will difer
    virtual void read() = 0;

    // Depending on the type of PhysicalInput, it could need to connect directly to Teensy
    // skipping the opamp and resistor divider, like the TouchPads
    virtual bool isDirectToTeensy() = 0;
    
    void setAnalogMinValue(unsigned int analogMinValue);
    
    void setAnalogMaxValue(unsigned int analogMaxValue);

//    static void onMidiControlChange(byte channel, byte controlNumber, byte value);
//    void setMidiControlChange(int controlNumber);
//    int getMidiControlNumber();
//    void setMidiTarget(byte channel, byte controlNumber, byte value);

protected:
    // The pin on which the PhysicalInput can be read
    byte pin = 0;

//    unsigned int analogMaxValue = 4095;
//    
//    unsigned int analogMinValue = 0;

    unsigned int previousReading = 0;
};

inline PhysicalInput::PhysicalInput(int index, String name):PhysicalIO{index, name}
{
  this->pin = ANALOG_IN_1_PIN; // TODO: DO IT ACCORDING TO INDEX
//  IORegistrar::registerInput(this);
}


//inline void PhysicalInput::setAnalogMinValue(unsigned int analogMinValue){
//  this->analogMinValue = analogMinValue;
//}
//    
//inline void PhysicalInput::setAnalogMaxValue(unsigned int analogMaxValue){
//  this->analogMaxValue = analogMaxValue;
//}

//inline void PhysicalInput::onMidiControlChange(byte channel, byte controlNumber, byte value){
//    int target = map(value, 0,127, 0, 4095);
//    setTarget(target);
//}

// TODO: Something like that
// We should be able to have multiple callbacks on one control number
// We should have a function to specify the range too, so the setTarget is set with a mapped value
//inline void PhysicalInput::setMidiControlChange(int controlNumber){
//  this->midiControlNumber = controlNumber;
//}

//inline int PhysicalInput::getMidiControlNumber(){
//  return this->midiControlNumber;
//}

//inline void PhysicalInput::setMidiTarget(byte channel, byte controlNumber, byte value){
//  
//}

//#define PhysicalInput MotherboardNamespace::PhysicalInput
#endif
