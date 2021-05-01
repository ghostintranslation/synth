#ifndef Motherboard_h
#define Motherboard_h

#include <vector> // This could be replaced with a custom type to reduce the program size
#include <EEPROM.h>
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE(); // MIDI library init

/**
 * Motherboard
 * v1.0.0
 */

enum InputType {
  None = 0,
  Button = 1,
  Potentiometer = 2,
  RotaryEncoder = 3
};

class Motherboard{

  private:
    static const byte midiControlsListSize = 64;

    // MidiControl type
    struct MidiControl{
      char controlName[20];
      byte midiCC = 0;
      byte midiChannel = 0;
    };

    // Config
    struct Config {
      // The serializable version of the config
      // Basically a duplicate but with a fixed size array instead of a vector or pointer array
      struct SerializableConfig {
        char deviceName[20] = "";
        byte midiChannel = 0;
        MidiControl midiControlsArray[midiControlsListSize] = {};
      };
      
      char deviceName[20] = "Motherboard";
      byte midiChannel = 0;
      std::vector<MidiControl> midiControls = {};
      
      void save(){
        Serial.println("Saving config");
        SerializableConfig conf = {
          deviceName: {},
          midiChannel: 0,
          {}
        };

        // Set the Midi channel
        strcpy(conf.deviceName, this->deviceName);
        conf.midiChannel = this->midiChannel;

        // Set the Midi controls
        byte i=0;
        for(MidiControl mc : this->midiControls) {
          MidiControlChangeCallback *c = Motherboard::getInstance()->getMidiControlChangeCallback(mc.controlName);
          if(c != nullptr && c->callback != nullptr){
            conf.midiControlsArray[i] = mc;
            i++;
          }
        }
        
        // Clear the memory
        for (unsigned int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.write(i, 0);
        }

        // Save in memory
        EEPROM.put( 0, conf );
        delay(10);
      }

      void load(){
        SerializableConfig conf = {
          deviceName: {},
          midiChannel: 0,
          {},
        };

        EEPROM.get( 0, conf);
        delay(10);
        
        // If the current device name is different than what's in memory,
        // erase the memory
        if (strcmp(this->deviceName, conf.deviceName) != 0){
          Serial.println("Device name does not match : Erasing memory");
          for (int i = 0 ; i < EEPROM.length() ; i++) {
            EEPROM.write(i, 0);
          }
          this->save();
        }

        this->midiChannel = conf.midiChannel;

        this->midiControls.clear();
        for(byte i=0; i<midiControlsListSize; i++){
          if(conf.midiControlsArray[i].controlName[0] != 0 &&
             conf.midiControlsArray[i].midiChannel != 255){
            this->midiControls.push_back(conf.midiControlsArray[i]);
          }
        }  
      }

      String toJSON(){
        String json = "{\"name\":\"";
        json += this->deviceName;
        json += "\",";
        json += "\"midiChannel\":\"";
        json += this->midiChannel;
        json += "\",";
        json += "\"midi\":[";

        bool hasAtLeastOneEntry = false;
        for(MidiControl mc : this->midiControls) {
          if(mc.controlName[0] != 0){
            hasAtLeastOneEntry = true;
            json += "{\"name\":\"" + (String)mc.controlName + "\", \"midiCC\":\"" + mc.midiCC + "\", \"midiChannel\":\"" + mc.midiChannel + "\"},";
          }
        }
        if(hasAtLeastOneEntry){
          // Removing last comma
          json.remove(json.length()-1,1);
        }
        json += "]}";

        return json;
      }
    } config;
    
    static Motherboard *instance;
    Motherboard();

    // Global states
    byte currentRow = 0;
    byte currentLed = 0;
    byte currentInput = 0;
    byte columnsNumber = 0;
    byte ioNumber;
    byte analogResolution = 10;
    byte midiChannel = 0;

    // Inputs
    InputType *inputs;

    // LEDs
    byte *leds;
    byte *ledsBrightness;
    unsigned int *ledsDuration;
    
    // Buttons
    bool *buttons;
    
    // Potentiometers
    float *potentiometers;
    float *potentiometersPrevious;
    unsigned int *potentiometersTarget;
    elapsedMicros clockUpdatePotentiometers;
    byte updatePotentiometersMillis = 50;
    float potentiometersSmoothness = 1;
    unsigned int *potentiometersTemp;
    unsigned int *potentiometersTempPrevious;
    byte *potentiometersReadings;

    // Encoders
    int *encoders;
    bool *encodersSwitch;
    byte *encodersState;
    byte currentEncPinA;
    byte currentEncPinB;
    #define R_START 0x0
    #define R_CW_FINAL 0x1
    #define R_CW_BEGIN 0x2
    #define R_CW_NEXT 0x3
    #define R_CCW_BEGIN 0x4
    #define R_CCW_FINAL 0x5
    #define R_CCW_NEXT 0x6
    // No complete step yet.
    #define DIR_NONE 0x0
    // Clockwise step.
    #define DIR_CW 0x10
    // Anti-clockwise step.
    #define DIR_CCW 0x20
    const byte ttable[7][4] = {
      // R_START
      {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
      // R_CW_FINAL
      {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
      // R_CW_BEGIN
      {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
      // R_CW_NEXT
      {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
      // R_CCW_BEGIN
      {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
      // R_CCW_FINAL
      {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
      // R_CCW_NEXT
      {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
    };

    // Debug clock
    elapsedMillis clockDebug;
    // Main clock
    elapsedMicros clockMain;
    const unsigned int intervalClockMain = 5000;
    // Leds clocks
    const unsigned int intervalDisplay = 1000;
    elapsedMicros clockDisplay;
    const unsigned int intervalDisplayFlash = 400;
    elapsedMillis clockDisplayFlash;
    // Inputs clock
    const unsigned int intervalInputs = 100;
    elapsedMicros clockInputs;

    // Callbacks
    using PressDownCallback = void (*)(byte);
    PressDownCallback *inputsPressDownCallback;
    using LongPressDownCallback = void (*)(byte);
    LongPressDownCallback *inputsLongPressDownCallback;
    using PressUpCallback = void (*)(byte);
    PressUpCallback *inputsPressUpCallback;
    using LongPressUpCallback = void (*)(byte);
    LongPressUpCallback *inputsLongPressUpCallback;
    elapsedMillis *inputsPressTime;
    bool *inputsLongPressDownFired;
    using PotentiometerChangeCallback = void (*)(byte, float, int);
    PotentiometerChangeCallback *inputsPotentiometerChangeCallback;
    using RotaryChangeCallback = void (*)(bool);
    RotaryChangeCallback *inputsRotaryChangeCallback;

    // Callbacks triggers
    void triggerPressCallbacks(byte inputIndex, bool value);
    void triggerPotentiometerChangeCallback(byte inputIndex, float value, unsigned int diff);
    void triggerRotaryChangeCallback(byte inputIndex, bool value);

    // MIDI Callbacks
    using MidiNoteOnCallback = void (*)(byte, byte, byte);
    MidiNoteOnCallback midiNoteOnCallback;
    using MidiNoteOffCallback = void (*)(byte, byte, byte);
    MidiNoteOffCallback midiNoteOffCallback;
    using GlobalMidiControlChangeCallback = void (*)(byte, byte, byte);
    GlobalMidiControlChangeCallback globalMidiControlChangeCallback = nullptr;
    using MidiSysExCallback = void (*)(const uint8_t*, uint16_t, bool);
    MidiSysExCallback midiSysExCallback = nullptr;
    using MidiControlChangeCallbackFunction = void (*)(byte, byte, byte);
    struct MidiControlChangeCallback{
      String controlName = "";
      MidiControlChangeCallbackFunction callback = nullptr;
    };
    std::vector<MidiControlChangeCallback> midiControlChangeCallbacks;
    MidiControlChangeCallback *getMidiControlChangeCallback(String name);
    
    // Handle MIDI
    static void handleMidiSysEx(const uint8_t* data, uint16_t length, bool last);
    static void handleMidiControlChange(byte channel, byte control, byte value);
    
    // Inputs/Outputs
    void updateDisplay();
    void iterateDisplay();
    void iterateInputs();
    void readCurrentInput();
    void readButton(byte inputIndex);
    void readPotentiometer(byte inputIndex);
    void readEncoder(byte inputIndex);
    void readMidiChannel();
    void setMainMuxOnLeds();
    void setMainMuxOnLeds2();
    void setMainMuxOnPots();
    void setMainMuxOnPots2();
    void setMainMuxOnEncoders1();
    void setMainMuxOnEncoders2();
    void setMainMuxOnChannel();
    void printInputs();
    void printLeds();

  public:
    static Motherboard *getInstance();
    static Motherboard* init(String deviceName, std::initializer_list<InputType> inputs);
    void update();
    
    void setLED(byte ledIndex, byte ledStatus, byte ledBrightness=255);
    void setAllLED(unsigned int binary, byte ledStatus); 
    void toggleLED(byte index);
    void resetAllLED();
    void writeLED(byte index);
    void initSequence();
    void setPotentiometer(byte index, unsigned int value);
    void setPotentiometersSmoothness(byte smoothness);
    int getInput(byte index);
    bool getEncoderSwitch(byte index);
    int getAnalogMaxValue();
    int getAnalogMinValue();
    byte getMidiChannel();

    // Callbacks
    void setHandlePressDown(byte inputIndex, PressDownCallback fptr);
    void setHandleLongPressDown(byte inputIndex, LongPressDownCallback fptr);
    void setHandlePressUp(byte inputIndex, PressUpCallback fptr);
    void setHandleLongPressUp(byte inputIndex, LongPressUpCallback fptr);
    void setHandlePotentiometerChange(byte inputIndex, PotentiometerChangeCallback fptr);
    void setHandleRotaryChange(byte inputIndex, RotaryChangeCallback fptr);

    // MIDI Callbacks
    void setHandleMidiNoteOn(MidiNoteOnCallback fptr);
    void setHandleMidiNoteOff(MidiNoteOffCallback fptr);
    void setHandleGlobalMidiControlChange(GlobalMidiControlChangeCallback fptr);
    void setHandleMidiControlChange(byte control, String controlName, MidiControlChangeCallbackFunction fptr);
    void setHandleMidiControlChange(byte midiChannel, byte midiCC, String controlName, MidiControlChangeCallbackFunction fptr);
    void setHandleMidiSysEx(MidiSysExCallback fptr);
};

// Instance pre init
Motherboard * Motherboard::instance = nullptr;

/**
 * Constructor
 */
inline Motherboard::Motherboard() {
}

/**
 * Singleton instance
 */
inline Motherboard *Motherboard::getInstance()    {
  if (!instance)
    instance = new Motherboard;
  return instance;
}

/**
 * Init
 */
inline Motherboard* Motherboard::init(String deviceName, std::initializer_list<InputType> inputs) {

  // Total number of inputs
  getInstance()->ioNumber = inputs.size();
  getInstance()->columnsNumber = getInstance()->ioNumber / 3;
  getInstance()->inputs = new InputType[getInstance()->ioNumber];
  
  // Init of the inputs
  byte i = 0;
  for(auto input : inputs){
    getInstance()->inputs[i] = input;
    i++;
  }

  getInstance()->leds = new byte[getInstance()->ioNumber];
  getInstance()->ledsBrightness = new byte[getInstance()->ioNumber];
  getInstance()->ledsDuration = new unsigned int[getInstance()->ioNumber];
  getInstance()->buttons = new bool[getInstance()->ioNumber];
  getInstance()->potentiometers = new float[getInstance()->ioNumber];
  getInstance()->potentiometersPrevious = new float[getInstance()->ioNumber];
  getInstance()->potentiometersTarget = new unsigned int[getInstance()->ioNumber];
  getInstance()->potentiometersTemp = new unsigned int[getInstance()->ioNumber];
  getInstance()->potentiometersTempPrevious = new unsigned int[getInstance()->ioNumber];
  getInstance()->potentiometersReadings = new byte[getInstance()->ioNumber];
  getInstance()->encoders = new int[getInstance()->ioNumber];
  getInstance()->encodersState = new byte[getInstance()->ioNumber];
  getInstance()->encodersSwitch = new bool[getInstance()->ioNumber];
  getInstance()->inputsPressDownCallback = new PressDownCallback[getInstance()->ioNumber];
  getInstance()->inputsLongPressDownCallback = new PressDownCallback[getInstance()->ioNumber];
  getInstance()->inputsPressUpCallback = new PressUpCallback[getInstance()->ioNumber];
  getInstance()->inputsLongPressUpCallback = new PressUpCallback[getInstance()->ioNumber];
  getInstance()->inputsPressTime = new elapsedMillis[getInstance()->ioNumber];
  getInstance()->inputsLongPressDownFired = new bool[getInstance()->ioNumber];
  getInstance()->inputsPotentiometerChangeCallback = new PotentiometerChangeCallback[getInstance()->ioNumber];
  getInstance()->inputsRotaryChangeCallback = new RotaryChangeCallback[getInstance()->ioNumber];

  for (byte i = 0; i < getInstance()->ioNumber; i++) {
    getInstance()->leds[i] = 0;
    getInstance()->ledsBrightness[i] = 255;
    getInstance()->ledsDuration[i] = 0;
    getInstance()->buttons[i] = true;
    getInstance()->potentiometers[i] = 0;
    getInstance()->potentiometersPrevious[i] = 1;
    getInstance()->potentiometersTarget[i] = 0;
    getInstance()->potentiometersTemp[i] = 0;
    getInstance()->potentiometersTempPrevious[i] = 0;
    getInstance()->potentiometersReadings[i] = 0;
    getInstance()->encoders[i] = 0;
    getInstance()->encodersState[i] = 0;
    getInstance()->encodersSwitch[i] = true;
    getInstance()->inputsPressDownCallback[i] = nullptr;
    getInstance()->inputsLongPressDownCallback[i] = nullptr;
    getInstance()->inputsPressUpCallback[i] = nullptr;
    getInstance()->inputsLongPressUpCallback[i] = nullptr;
    getInstance()->inputsPressTime[i] = 0;
    getInstance()->inputsLongPressDownFired[i] = false;
    getInstance()->inputsPotentiometerChangeCallback[i] = nullptr;
    getInstance()->inputsRotaryChangeCallback[i] = nullptr;
  }

  
  // Main multiplexer
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);

  // 2nd level multiplexers
  pinMode(5, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(14, OUTPUT);

  // Matrix Rows
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);

  // Inputs / LEDs
  pinMode(22, INPUT_PULLUP);

  analogReadResolution(getInstance()->analogResolution);

  getInstance()->readMidiChannel();
  
  // Init sequence
  getInstance()->initSequence();

  // MIDI init
  MIDI.setHandleControlChange(getInstance()->handleMidiControlChange);
  MIDI.begin();
  usbMIDI.setHandleSystemExclusive(getInstance()->handleMidiSysEx);
  usbMIDI.setHandleControlChange(getInstance()->handleMidiControlChange);

  return getInstance();
}

/**
 * Update
 */
inline void Motherboard::update() {
  // Main clock
  if (this->clockMain >= this->intervalClockMain) {
    this->clockMain = 0;
  }

  if (this->clockMain > this->intervalClockMain / 2) {
    // Leds

    // Clock for flashing the LEDs
    if (this->clockDisplayFlash >= this->intervalDisplayFlash) {
      this->clockDisplayFlash = 0;
    }

    // Clorck for iterating over each LED
    if (this->clockDisplay >= this->intervalDisplay) {
      this->iterateDisplay();
      this->clockDisplay = 0;
    }

    this->updateDisplay();
  } else {
    // Inputs

    // At the end of the clock we iterate to next input
    if (this->clockInputs >= this->intervalInputs) {
      this->iterateInputs();
      this->clockInputs = 0;
    } else {
      // Reading the current input
      this->readCurrentInput();
    }
  }

  // Updating potentiometers smoothed values
  if(this->clockUpdatePotentiometers > this->updatePotentiometersMillis){
    // Every 10ms loop and update every potentiometer's value to get closer to target
    for(byte i = 0; i < this->ioNumber; i++){
      if(this->potentiometersTarget[i] != this->potentiometers[i]){
        this->potentiometers[i] += (potentiometersSmoothness * (this->potentiometersTarget[i] - this->potentiometers[i]) / 1024)  / (100 / (float)this->updatePotentiometersMillis);
      }

      if(roundf(this->potentiometers[i] * 100) / 100 == 0){
        this->potentiometers[i] = 0;
      }
  
      if(this->potentiometers[i] != this->potentiometersPrevious[i]){
        // Calling the potentiometer callback if there is one
        this->triggerPotentiometerChangeCallback(i, this->potentiometers[i], this->potentiometers[i] - this->potentiometersPrevious[i]);
      }

      this->potentiometersPrevious[i] = this->potentiometers[i];
    }

    this->clockUpdatePotentiometers = 0;
  }
  
  // Debug
  if (this->clockDebug >= 100) {
//    this->printInputs();
    //    this->printLeds();
    this->clockDebug = 0;
  }
  
  MIDI.read();
  usbMIDI.read();
}


/**
 * Main multiplexer on LEDs
 */
inline void Motherboard::setMainMuxOnLeds() {
  pinMode(22, OUTPUT);
  digitalWriteFast(2, LOW);
  digitalWriteFast(3, LOW);
  digitalWriteFast(4, LOW);
}

/**
 * Main multiplexer on LEDs 2
 */
inline void Motherboard::setMainMuxOnLeds2() {
  pinMode(22, OUTPUT);
  digitalWriteFast(2, HIGH);
  digitalWriteFast(3, LOW);
  digitalWriteFast(4, LOW);
}

/**
 * Main multiplexer on Potentiometers
 */
inline void Motherboard::setMainMuxOnPots() {
  pinMode(22, INPUT);
  digitalWriteFast(2, LOW);
  digitalWriteFast(3, HIGH);
  digitalWriteFast(4, LOW);
}

/**
 * Main multiplexer on Potentiometers 2
 */
inline void Motherboard::setMainMuxOnPots2() {
  pinMode(22, INPUT);
  digitalWriteFast(2, HIGH);
  digitalWriteFast(3, HIGH);
  digitalWriteFast(4, LOW);
}

/**
 * Main multiplexer on Encoders
 */
inline void Motherboard::setMainMuxOnEncoders1() {
  pinMode(22, INPUT_PULLUP);
  digitalWriteFast(2, LOW);
  digitalWriteFast(3, LOW);
  digitalWriteFast(4, HIGH);
}

/**
 * Main multiplexer on Encoder's switches
 */
inline void Motherboard::setMainMuxOnEncoders2() {
  pinMode(22, INPUT_PULLUP);
  digitalWriteFast(2, HIGH);
  digitalWriteFast(3, LOW);
  digitalWriteFast(4, HIGH);
}

/**
 * Main multiplexer on Channel
 */
inline void Motherboard::setMainMuxOnChannel() {
  pinMode(22, INPUT_PULLUP);
  digitalWriteFast(2, LOW);
  digitalWriteFast(3, HIGH);
  digitalWriteFast(4, HIGH);
}

/**
 * Iterate LEDs
 */
inline void Motherboard::iterateDisplay() {
  // Iterating only on the acive leds
  // to save time between one led's iterations
  // and so improve brightness
  for (byte i = this->currentLed + 1; i < this->currentLed + 1 + this->ioNumber; i++) {
    byte j = i % this->ioNumber;
    if (this->leds[j] > 0) {
      this->currentLed = j;
      break;
    }
  }
}

/**
 * Refresh the leds
 */
inline void Motherboard::updateDisplay() {
  if (this->currentLed < 8) {
    this->setMainMuxOnLeds();
  } else {
    this->setMainMuxOnLeds2();
  }

  byte r0 = bitRead(this->currentLed, 0);
  byte r1 = bitRead(this->currentLed, 1);
  byte r2 = bitRead(this->currentLed, 2);
  digitalWriteFast(5, r0);
  digitalWriteFast(9, r1);
  digitalWriteFast(14, r2);

  switch(this->leds[this->currentLed]){
    case 1:
      // Solid lightw
      this->writeLED(this->currentLed);
    break;
    
    case 2:
      // Slow flashing
      digitalWriteFast(22, HIGH);
      if (this->clockDisplayFlash % 400 > 200) {
        this->writeLED(this->currentLed);
      }
    break;
    
    case 3:
      // Fast flashing
      digitalWriteFast(22, HIGH);
      if (this->clockDisplayFlash % 200 > 100) {
        this->writeLED(this->currentLed);
      }
    break;

    case 4:
      // Solid for 50 milliseconds
      this->writeLED(this->currentLed);
      if (this->ledsDuration[this->currentLed] == 0) {
        this->ledsDuration[this->currentLed] = (clockDisplayFlash + 50) % intervalDisplayFlash;
      }
  
      if (this->clockDisplayFlash >= this->ledsDuration[this->currentLed]) {
        digitalWriteFast(22, HIGH);
        this->leds[this->currentLed] = 0;
        this->ledsDuration[this->currentLed] = 0;
      }
    break;

    case 5:
      // Solid low birghtness
      this->ledsBrightness[this->currentLed] = 128;
      this->writeLED(this->currentLed);
    break;

    default:
      digitalWriteFast(22, HIGH);
    break;
  }
}


/**
 * Iterate over the inputs
 */
inline void Motherboard::iterateInputs() {
  this->currentInput++;
  this->currentInput = this->currentInput % this->ioNumber;
}

/**
 * Read value of current inout
 */
inline void Motherboard::readCurrentInput() {
  switch (this->inputs[this->currentInput]) {
    default:
    case 0:
      // Silence is golden
      break;

    case 1:
      this->readButton(this->currentInput);
      break;
    case 2:
      this->readPotentiometer(this->currentInput);
      break;
    case 3:
      this->readEncoder(this->currentInput);
      break;
  }
}

/**
 * Get button value
 * @param byte inputeIndex The index of the input
 */
inline void Motherboard::readButton(byte inputIndex) {
  this->setMainMuxOnEncoders2();

  byte rowNumber = inputIndex / this->columnsNumber;

  for (byte i = 0; i < 3; i++) {
    if (i == rowNumber) {
      digitalWriteFast(15 + i, LOW);
    } else {
      digitalWriteFast(15 + i, HIGH);
    }
  }

  byte columnNumber = inputIndex % this->columnsNumber;

  byte r0 = bitRead(columnNumber, 0);
  byte r1 = bitRead(columnNumber, 1);
  byte r2 = bitRead(columnNumber, 2);
  digitalWriteFast(5, r0);
  digitalWriteFast(9, r1);
  digitalWriteFast(14, r2);

  // Giving some time to the Mux and pins to switch
  if (this->clockInputs > this->intervalInputs / 1.5) {
    
    // Reading the new value
    bool newReading = digitalReadFast(22);

    // If there is a short or a long press callback on that input
    if(this->inputsPressDownCallback[inputIndex] != nullptr ||
       this->inputsPressUpCallback[inputIndex] != nullptr ||
       this->inputsLongPressDownCallback[inputIndex] != nullptr ||
       this->inputsLongPressUpCallback[inputIndex] != nullptr){
        
      // Inverted logic, 0 = button pushed
      // If previous value is not pushed and now is pushed
      // So if it's pushed
      if(this->buttons[inputIndex] && !newReading){ 
        // Start the counter of that input
        this->inputsPressTime[inputIndex] = 0;
        this->inputsLongPressDownFired[inputIndex] = false;
        
        // If there is a short press down callback on that input, and there is no Long Press down
        if(this->inputsLongPressDownCallback[inputIndex] == nullptr &&
           this->inputsPressDownCallback[inputIndex] != nullptr){
          this->inputsPressDownCallback[inputIndex](inputIndex);
        }
      }

      // If it stayed pressed for 200ms and Long Press Down callback hasn't been fired yet
      if(!this->buttons[inputIndex] && !newReading){ 
        if(this->inputsPressTime[inputIndex] >= 200 && !this->inputsLongPressDownFired[inputIndex]){
          
          if(this->inputsLongPressDownCallback[inputIndex] != nullptr){
            // Fire the callback
            this->inputsLongPressDownCallback[inputIndex](inputIndex);
            this->inputsLongPressDownFired[inputIndex] = true;
          }
        }
      }

      // If it's released
      if(!this->buttons[inputIndex] && newReading){ 
        // How long was it pressed
        if(this->inputsPressTime[inputIndex] < 200){
          // Short press
          
          // If there is a short press callback on that input
          if(this->inputsPressUpCallback[inputIndex] != nullptr){
            this->inputsPressUpCallback[inputIndex](inputIndex);
          }
        }else{
          // Long press
          
          // If there is a long press callback on that input
          if(this->inputsLongPressUpCallback[inputIndex] != nullptr){
            this->inputsLongPressUpCallback[inputIndex](inputIndex);
          }else if(this->inputsPressUpCallback[inputIndex] != nullptr){
            // If the input was pressed for a long time but there is only a short press callback
            // the short press callback should still be called
            this->inputsPressUpCallback[inputIndex](inputIndex);
          }
        }
      }
    }
  
    // Updating the value
    this->buttons[inputIndex] = newReading;
  }
}

inline void Motherboard::triggerPressCallbacks(byte inputIndex, bool value){
  if(value){
    this->inputsPressDownCallback[inputIndex](inputIndex);
  }else{
    this->inputsPressUpCallback[inputIndex](inputIndex);
  }
}


/**
 * Get potentiometer value
 * @param byte inputeIndex The index of the input
 */
inline void Motherboard::readPotentiometer(byte inputIndex) {
  if (inputIndex < 8) {
    this->setMainMuxOnPots();
  } else {
    this->setMainMuxOnPots2();
  }

  byte r0 = bitRead(inputIndex, 0);
  byte r1 = bitRead(inputIndex, 1);
  byte r2 = bitRead(inputIndex, 2);
  digitalWrite(5, r0);
  digitalWrite(9, r1);
  digitalWrite(14, r2);

  this->potentiometersReadings[inputIndex] = this->potentiometersReadings[inputIndex] + 1;
  this->potentiometersTemp[inputIndex] += analogRead(22);
  
  if(this->potentiometersReadings[inputIndex] == 255){
    if((int)(this->potentiometersTemp[inputIndex] / 255 - this->potentiometersTempPrevious[inputIndex] / 255) < -1
    || (this->potentiometersTemp[inputIndex] / 255 - this->potentiometersTempPrevious[inputIndex] / 255) > 1){
      this->setPotentiometer(inputIndex, this->potentiometersTemp[inputIndex] / 255);
      this->potentiometersTempPrevious[inputIndex] = this->potentiometersTemp[inputIndex];
    }
    
    this->potentiometersReadings[inputIndex] = 0;
    this->potentiometersTemp[inputIndex] = 0;
  }
}

inline void Motherboard::setPotentiometer(byte index, unsigned int value){
  this->potentiometersTarget[index] = value;
  this->potentiometersTarget[index] = map(this->potentiometersTarget[index], this->getAnalogMinValue(), this->getAnalogMaxValue(), 0, 1023);
  this->potentiometersTarget[index] = constrain(this->potentiometersTarget[index], (unsigned int)0, (unsigned int)1023);
}

inline void Motherboard::triggerPotentiometerChangeCallback(byte inputIndex, float value, unsigned int diff){
  if(this->inputsPotentiometerChangeCallback[inputIndex] != nullptr){
    this->inputsPotentiometerChangeCallback[inputIndex](inputIndex, value, diff);
  }
}

/**
 * Get encoder value
 * @param byte inputeIndex The index of the input
 */
inline void Motherboard::readEncoder(byte inputIndex) {
  // Activating the right row in the matrix
  byte rowNumber = inputIndex / this->columnsNumber;

  // Setting the main multiplexer on encoders
  if (this->clockInputs < this->intervalInputs / 10) {
    for (byte i = 0; i < 3; i++) {
      if (i == rowNumber) {
        digitalWriteFast(15 + i, LOW);
      } else {
        digitalWriteFast(15 + i, HIGH);
      }
    }
    this->setMainMuxOnEncoders1();
  }

  // Getting right pin numbers on the multiplexer
  byte columnNumber = inputIndex % this->columnsNumber;
  byte muxPinA = columnNumber * 2;
  byte muxPinB = columnNumber * 2 + 1;

  // Giving time for the multiplexer to switch to Pin A
  if (this->clockInputs > this->intervalInputs / 10
      && this->clockInputs < this->intervalInputs / 6) {
    byte r0 = bitRead(muxPinA, 0);
    byte r1 = bitRead(muxPinA, 1);
    byte r2 = bitRead(muxPinA, 2);
    digitalWriteFast(5, r0);
    digitalWriteFast(9, r1);
    digitalWriteFast(14, r2);

    this->currentEncPinA = digitalReadFast(22);
  }

  // Giving time for the multiplexer to switch to Pin B
  if (this->clockInputs > this->intervalInputs / 6
      && this->clockInputs < this->intervalInputs / 2) {
    int r0 = bitRead(muxPinB, 0);
    int r1 = bitRead(muxPinB, 1);
    int r2 = bitRead(muxPinB, 2);
    digitalWriteFast(5, r0);
    digitalWriteFast(9, r1);
    digitalWriteFast(14, r2);

    this->currentEncPinB = digitalReadFast(22);
  }

  // When reading of Pin A and B is done we can interpret the result
  if (this->clockInputs > this->intervalInputs / 2
      && this->clockInputs < this->intervalInputs / 1.5) {

    byte pinstate = (this->currentEncPinB << 1) | this->currentEncPinA;
    // Determine new state from the pins and state table.
    this->encodersState[inputIndex] = this->ttable[this->encodersState[inputIndex] & 0xf][pinstate];
    // Return emit bits, ie the generated event.
    byte result = this->encodersState[inputIndex] & 0x30;

    if (result == DIR_CW) {
      this->encoders[inputIndex]--;

      // Calling the decrement callback if there is one
      this->triggerRotaryChangeCallback(inputIndex, false);
    } else if (result == DIR_CCW) {
      this->encoders[inputIndex]++;
      
      // Calling the decrement callback if there is one
      this->triggerRotaryChangeCallback(inputIndex, true);
    }

    // Setting the main multiplexer on encoder's buttons
    this->setMainMuxOnEncoders2();
    byte r0 = bitRead(columnNumber, 0);
    byte r1 = bitRead(columnNumber, 1);
    byte r2 = bitRead(columnNumber, 2);
    digitalWriteFast(5, r0);
    digitalWriteFast(9, r1);
    digitalWriteFast(14, r2);
  }

  // Giving time for the multiplexer to switch to Pin B
  if (this->clockInputs > this->intervalInputs / 1.5) {
//    this->encodersSwitch[inputIndex] = digitalReadFast(22);

    // Reading the new value
    bool newReading = digitalReadFast(22);
  
    // If there is a short or a long press callback on that input
    if(this->inputsPressDownCallback[inputIndex] != nullptr ||
       this->inputsPressUpCallback[inputIndex] != nullptr ||
       this->inputsLongPressDownCallback[inputIndex] != nullptr ||
       this->inputsLongPressUpCallback[inputIndex] != nullptr){
        
      // Inverted logic, 0 = button pushed
      // If previous value is not pushed and now is pushed
      // So if it's pushed
      if(this->encodersSwitch[inputIndex] && !newReading){ 
        // Start the counter of that input
        this->inputsPressTime[inputIndex] = 0;
        this->inputsLongPressDownFired[inputIndex] = false;

        // If there is a short press down callback on that input, and there is no Long Press down
        if(this->inputsLongPressDownCallback[inputIndex] == nullptr &&
           this->inputsPressDownCallback[inputIndex] != nullptr){
          this->inputsPressDownCallback[inputIndex](inputIndex);
        }
      }

      // If it stayed pressed for 200ms and Long Press Down callback hasn't been fired yet
      if(!this->encodersSwitch[inputIndex] && !newReading){ 
        if(this->inputsPressTime[inputIndex] >= 200 && !this->inputsLongPressDownFired[inputIndex]){
          
          if(this->inputsLongPressDownCallback[inputIndex] != nullptr){
            // Fire the callback
            this->inputsLongPressDownCallback[inputIndex](inputIndex);
            this->inputsLongPressDownFired[inputIndex] = true;
          }
        }
      }
      
      // If it's released
      if(!this->encodersSwitch[inputIndex] && newReading){ 
        // How long was it pressed
        if(this->inputsPressTime[inputIndex] < 200){
          // Short press
          
          // If there is a short press callback on that input
          if(this->inputsPressUpCallback[inputIndex] != nullptr){
            this->inputsPressUpCallback[inputIndex](inputIndex);
          }
        }else{
          // Long press
          
          // If there is a long press callback on that input
          if(this->inputsLongPressUpCallback[inputIndex] != nullptr){
            this->inputsLongPressUpCallback[inputIndex](inputIndex);
          }else if(this->inputsPressUpCallback[inputIndex] != nullptr){
            // If the input was pressed for a long time but there is only a short press callback
            // the short press callback should still be called
            this->inputsPressUpCallback[inputIndex](inputIndex);
          }
        }
      }
    }
  
    // Updating the value
    this->encodersSwitch[inputIndex] = newReading;
  }
}

inline void Motherboard::triggerRotaryChangeCallback(byte inputIndex, bool value){
  if(this->inputsRotaryChangeCallback[inputIndex] != nullptr){
    this->inputsRotaryChangeCallback[inputIndex](false);
  }
}

/**
 * Read the Midi channel from the dipswitch
 */
inline void Motherboard::readMidiChannel() {
  this->setMainMuxOnChannel();
  delay(50); // Only because this function is used in Init only

  byte midiChannel = 0b00000000;
  for (byte i = 0; i < 4; i++) {
    byte r0 = bitRead(i, 0);
    byte r1 = bitRead(i, 1);
    byte r2 = bitRead(i, 2);
    digitalWriteFast(5, r0);
    digitalWriteFast(9, r1);
    digitalWriteFast(14, r2);
    delay(5); // Only because this function is used in Init only
    byte channelBit = !digitalReadFast(22);
    bitWrite(midiChannel, i, channelBit);
  }
  this->midiChannel = midiChannel + 1;
}

/**
 * Set a led status
 */
inline void Motherboard::setLED(byte ledIndex, byte ledStatus, byte ledBrightness) {
  switch(ledStatus){
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    {
      this->leds[ledIndex] = ledStatus;
      this->ledsBrightness[ledIndex] = ledBrightness;
    }
    break;
    default:
      this->leds[ledIndex] = 0;
    break;
  }
}

/**
 * Set all LEDs 
 */
inline void Motherboard::setAllLED(unsigned int binary, byte ledStatus) {
  unsigned int n = binary;
  
  for (byte i = 0; i < this->ioNumber; i++) {
    if(n & 1){
      this->setLED(i, ledStatus);
    }else{
      this->setLED(i, 0);
    }
    n /= 2;
  }
}

/**
 * Toggle one LED
 */
inline void Motherboard::toggleLED(byte index) {
  if(this->leds[index] > 0){
    this->leds[index] = 0;
  }else{
    this->leds[index] = 1;
  }
}

/**
 * Reset all LEDs
 */
inline void Motherboard::resetAllLED() {
  for (byte i = 0; i < this->ioNumber; i++) {
    if (this->leds[i] != 4) {
      this->leds[i] = 0;
    }
  }
}

/**
 * Set potentiometers smoothness
 */
inline void Motherboard::setPotentiometersSmoothness(byte smoothness){
  this->potentiometersSmoothness = map((float)smoothness, 0, 255, 1, 0.05);
}

/**
 * Get input value
 * @param byte index The index of the input
 */
inline int Motherboard::getInput(byte index) {
  switch (this->inputs[index]) {
    default:
    case 0:
      // Empty
      return 0;
      break;
    case 1:
      // Button
      return !this->buttons[index];
      break;
    case 2:
      // Potentiometer
      return this->potentiometers[index];
      break;
    case 3:
      // Encoder
      // Device is not saving the encoders values, only the latest change
      int value = this->encoders[index];
      this->encoders[index] = 0;
      return value;
      break;
  }
}

/**
 * Get encoder switch value
 * @param byte index The index of the input
 */
inline bool Motherboard::getEncoderSwitch(byte index) {
  return !this->encodersSwitch[index];
}

/**
 * Get max analog value according to resolution
 */
inline int Motherboard::getAnalogMinValue() {
  return 0;
}

/**
 * Get max analog value according to resolution
 */
inline int Motherboard::getAnalogMaxValue() {
  return (1 << this->analogResolution) - 1;
}

inline byte Motherboard::getMidiChannel() {
  return this->midiChannel;
}

/**
 * Handle press down on a button
 */
inline void Motherboard::setHandlePressDown(byte inputIndex, PressDownCallback fptr){
  // Press can only happen on a button and an encoder's switch
  if(this->inputs[inputIndex] == Button || this->inputs[inputIndex] == RotaryEncoder){
    this->inputsPressDownCallback[inputIndex] = fptr;
  }
}

/**
 * Handle press up on a button
 */
inline void Motherboard::setHandlePressUp(byte inputIndex, PressUpCallback fptr){
  // Press can only happen on a button and an encoder's switch
  if(this->inputs[inputIndex] == Button || this->inputs[inputIndex] == RotaryEncoder){
    this->inputsPressUpCallback[inputIndex] = fptr;
  }
}

/**
 * Handle long press down on a button
 */
inline void Motherboard::setHandleLongPressDown(byte inputIndex, LongPressDownCallback fptr){
  // Press can only happen on a button and an encoder's switch
  if(this->inputs[inputIndex] == Button || this->inputs[inputIndex] == RotaryEncoder){
    this->inputsLongPressDownCallback[inputIndex] = fptr;
  }
}

/**
 * Handle long press up on a button
 */
inline void Motherboard::setHandleLongPressUp(byte inputIndex, LongPressUpCallback fptr){
  // Press can only happen on a button and an encoder's switch
  if(this->inputs[inputIndex] == Button || this->inputs[inputIndex] == RotaryEncoder){
    this->inputsLongPressUpCallback[inputIndex] = fptr;
  }
}

/**
 * Handle potentiometer
 */
inline void Motherboard::setHandlePotentiometerChange(byte inputIndex, PotentiometerChangeCallback fptr){
  // Only for rotaries
  if(this->inputs[inputIndex] == Potentiometer){
    this->inputsPotentiometerChangeCallback[inputIndex] = fptr;
  }
}

/**
 * Handle rotary
 */
inline void Motherboard::setHandleRotaryChange(byte inputIndex, RotaryChangeCallback fptr){
  // Only for rotaries
  if(this->inputs[inputIndex] == RotaryEncoder){
    this->inputsRotaryChangeCallback[inputIndex] = fptr;
  }
}

inline void Motherboard::writeLED(byte index){
  byte reversedBrightness = map(this->ledsBrightness[index], 0, 255, 255, 0);
  analogWrite(22, reversedBrightness); 
}


/**
 * Handle MIDI note on
 */
inline void Motherboard::setHandleMidiNoteOn(MidiNoteOnCallback fptr){
  this->midiNoteOnCallback = fptr;
  MIDI.setHandleNoteOn(fptr);
  usbMIDI.setHandleNoteOn(fptr);
}

/**
 * Handle MIDI note off
 */
inline void Motherboard::setHandleMidiNoteOff(MidiNoteOffCallback fptr){
  this->midiNoteOffCallback = fptr;
  MIDI.setHandleNoteOff(fptr);
  usbMIDI.setHandleNoteOff(fptr);
}

/**
 * Handle MIDI control change
 */
inline void Motherboard::setHandleGlobalMidiControlChange(GlobalMidiControlChangeCallback fptr){
  this->globalMidiControlChangeCallback = fptr;
}

/**
 * Handle MIDI control change
 */
inline void Motherboard::setHandleMidiControlChange(byte control, String controlName, MidiControlChangeCallbackFunction fptr){
  this->setHandleMidiControlChange(0, control, controlName, fptr);
}

/**
 * Set a callback to a MIDI control change message
 */
inline void Motherboard::setHandleMidiControlChange(byte channel, byte control, String controlName, MidiControlChangeCallbackFunction fptr){
  // Init the midi control with what is set in the code
  MidiControl midiControl = {
    "",
    midiCC: control,
    midiChannel: channel
  };
  //  strcpy(midiControl.controlName, controlName);
  controlName.toCharArray(midiControl.controlName, 20);
  
  // Check if something is defined in the config for that callback
  bool midiControlFromConfig = false;
  for(MidiControl c : Motherboard::getInstance()->config.midiControls) {
    char controlNameCharArray[20];
    controlName.toCharArray(controlNameCharArray, 20);

    if(strcmp(c.controlName, controlNameCharArray) == 0){
      // This callback exists in the config so we'll use it instead
      midiControl = c;
      midiControlFromConfig = true;
      break;
    }
  }

  // Only if the MidiControl is not yet in the config we add it
  // The MidiControl would be in the config if it was loaded from memory on the Init
  if(!midiControlFromConfig){
    this->config.midiControls.push_back(midiControl);
  }
  // Add the callback to the list
  this->midiControlChangeCallbacks.push_back({
    controlName: controlName,
    callback: fptr
  });
}

/**
 * Handle received MIDI Control Change message
 */
inline void Motherboard::handleMidiControlChange(byte channel, byte control, byte value){  
  // Internal midi channel is from 1 to 16
  // Incoming channel is from 1 to 16
  // Callbacks channel is from 0 to 16, 0 meaning bounded with internal midi channel
  
  // If the incoming message's channel corresponds to the board's channel 
  if(Motherboard::getInstance()->getMidiChannel() == channel){
    // Callbacks on internal channel 0 are to be triggered on the board's channel
    for(MidiControl mc : Motherboard::getInstance()->config.midiControls) {
      if(mc.midiChannel == 0 && mc.midiCC == control){
        MidiControlChangeCallback *c = Motherboard::getInstance()->getMidiControlChangeCallback(mc.controlName);
        if(c != nullptr && c->callback != nullptr){
          c->callback(0, control, value);
        }
      }
    }
  }

  // Triggering any existing callback on the incoming channel and control
  for(MidiControl mc : Motherboard::getInstance()->config.midiControls) {
    if(mc.midiChannel == channel && mc.midiCC == control){
      MidiControlChangeCallback *c = Motherboard::getInstance()->getMidiControlChangeCallback(mc.controlName);
      if(c != nullptr && c->callback != nullptr){
        c->callback(channel, control, value);
      }
    }
  }

  // Triggering the global callback
  if(Motherboard::getInstance()->globalMidiControlChangeCallback != nullptr){
    Motherboard::getInstance()->globalMidiControlChangeCallback(channel, control, value);
  }
}

/**
 * Handle MIDI SysEx
 */
inline void Motherboard::setHandleMidiSysEx(MidiSysExCallback fptr){
  this->midiSysExCallback = fptr;
}

/**
 * Handle MIDI SysEx message
 */
inline void Motherboard::handleMidiSysEx(const uint8_t* data, uint16_t length, bool last){
  switch(data[1]){
    // Device config request
    // Sending reply in a JSON string
    case 0:
    {
      String response = Motherboard::getInstance()->config.toJSON();
//      Serial.println(response);

      byte byteResponse[response.length()];
      response.getBytes(byteResponse, response.length()+1);
      byte byteTypedResponse[sizeof(byteResponse)+1];
      byteTypedResponse[0] = 0;
      for(unsigned int i=0; i<sizeof(byteResponse); i++){
        byteTypedResponse[i+1] = byteResponse[i];
      }
      usbMIDI.sendSysEx(sizeof(byteTypedResponse), byteTypedResponse, false);
    }
    break;

    // Device config import
    case 1:
    {
      // The response format is:
      // 1 [Callback_Name_Variable_Length] [midiCC] [midiChannel]
      // 
      // Since the name's length is variable, and there is no delimiting character surrounding it,
      // one way of finding out if this message is for that callback is to search for known names in the message

      bool somethingToSave = false;
      
      // Converts the response into string
      String dataString = String((char *)data);

      // Loop through all known callbacks and search each callback's name in the response
      // If the name is found then the message is meant for that callback
      for(MidiControl& mc: Motherboard::getInstance()->config.midiControls) {
        MidiControlChangeCallback *c = Motherboard::getInstance()->getMidiControlChangeCallback(mc.controlName);
        if(c != nullptr && c->callback != nullptr){
          int indexOfCallbackName = dataString.lastIndexOf(mc.controlName, 4);
          if(indexOfCallbackName > -1){
            // This callback's name was found in the response

            if(mc.midiCC != data[2 + String(mc.controlName).length()] ||
               mc.midiChannel != data[2 + String(mc.controlName).length() + 1]){
                // Save only if there is actually a change
                somethingToSave = true;
                
                mc.midiCC = data[2 + String(mc.controlName).length()];
                mc.midiChannel = data[2 + String(mc.controlName).length() + 1];
               }
            break;
          }
        }
      }

      // Saving the config
      if(somethingToSave){
        Motherboard::getInstance()->config.save();
      
        // Sending OK
        byte byteMessage[2] = {1,1};
        usbMIDI.sendSysEx(2, byteMessage, false);

        Motherboard::getInstance()->initSequence();
      }else{
        // Sending KO
        byte byteMessage[2] = {1,0};
        usbMIDI.sendSysEx(2, byteMessage, false);
      }
    }
    break;
    
    default:
    break;
  }

  if(Motherboard::getInstance()->midiSysExCallback != nullptr){
    Motherboard::getInstance()->midiSysExCallback(data, length, last);
  }
}

/**
 * Get MIDI Control Change Callback
 * @param (String) name The name of the callback function
 * @return (MidiControlChangeCallback) The callback function
 */
inline Motherboard::MidiControlChangeCallback *Motherboard::getMidiControlChangeCallback(String name){
  for(MidiControlChangeCallback &c : Motherboard::getInstance()->midiControlChangeCallbacks) {
    if(c.controlName == name){
      return &c;
    }
  }

  return nullptr;
}

/**
 * LEDs init sequence
 */
inline void Motherboard::initSequence(){
  // Init sequence
  for(byte i = 0; i<this->ioNumber; i++){
    this->setLED(i, 1);
    this->iterateDisplay();
    this->updateDisplay();
    delay(50);
  }
  this->resetAllLED();
  this->updateDisplay();
}

/**
 * Debug print
 */
inline void Motherboard::printInputs() {
  Serial.println("Potentiometers:");
  for (byte j = 0; j < this->ioNumber; j++) {
    Serial.print(this->potentiometers[j]);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println("Encoders:");
  for (byte j = 0; j < this->ioNumber; j++) {
    Serial.print(this->encoders[j]);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println("Encoder switches:");
  for (byte j = 0; j < this->ioNumber; j++) {
    Serial.print(this->encodersSwitch[j]);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println("Buttons:");
  for (byte j = 0; j < this->ioNumber; j++) {
    Serial.print(this->buttons[j]);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println("Midi Channel:");
  Serial.print(this->midiChannel);
  Serial.println("");

  Serial.println("");
}

/**
 * Debug print
 */
inline void Motherboard::printLeds() {
  Serial.println("Leds:");
  for (byte j = 0; j < this->ioNumber; j++) {
    Serial.print(this->leds[j]);
    Serial.print(" ");
  }
  Serial.println("");
}
#endif
