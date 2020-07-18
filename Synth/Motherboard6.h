#ifndef Motherboard6_h
#define Motherboard6_h

/*
 * Motherboard6
 */
class Motherboard6{
  
  private:
    static Motherboard6 *instance;
    Motherboard6();
    
    byte currentRow = 0;
    byte currentLed = 0;
    byte currentInput = 0;
    byte columnsNumber = 2;
    byte ioNumber;
    byte analogResolution = 10;
    byte midiChannel = 0;
    
    byte *inputs;
    byte *leds;
    unsigned int *ledsDuration;
    // Buttons
    bool *buttons;
    // Potentiometers
    unsigned int *potentiometers;
    // For smoothing purposes
    unsigned int *potentiometersTemp;
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
    const unsigned int intervalDisplay = 10;
    elapsedMicros clockDisplay;
    const unsigned int intervalDisplayFlash = 400;
    elapsedMillis clockDisplayFlash;
    // Inputs clock
    const unsigned int intervalInputs = 100;
    elapsedMicros clockInputs;

    void updateDisplay();
    void iterateDisplay();
    void iterateInputs();
    void readCurrentInput();
    void readButton(byte inputIndex);
    void readPotentiometer(byte inputIndex);
    void readEncoder(byte inputIndex);
    void readMidiChannel();
    void setMainMuxOnLeds();
    void setMainMuxOnPots();
    void setMainMuxOnEncoders1();
    void setMainMuxOnEncoders2();
    void setMainMuxOnChannel();
    void printInputs();
    void printLeds();
    
  public:
    static Motherboard6 *getInstance();
    void init(byte *inputs);
    void update();
    void setDisplay(byte ledIndex, byte ledStatus);
    void resetDisplay();
    int getInput(byte index);
    int getEncoderSwitch(byte index);
    int getAnalogMaxValue();
    int getAnalogMinValue();
    byte getMidiChannel();
};

// Instance pre init
Motherboard6 * Motherboard6::instance = nullptr;

/**
 * Constructor
 */
inline Motherboard6::Motherboard6(){
  this->ioNumber = 3*this->columnsNumber;

  this->leds = new byte[this->ioNumber];
  this->buttons = new bool[this->ioNumber];
  this->potentiometers = new unsigned int[this->ioNumber];
  this->potentiometersTemp = new unsigned int[this->ioNumber];
  this->potentiometersReadings = new byte[this->ioNumber];
  this->encoders = new int[this->ioNumber];
  this->encodersState = new byte[this->ioNumber];
  this->encodersSwitch = new bool[this->ioNumber];

  for(byte i = 0; i < this->ioNumber; i++){
    this->inputs[i] = 0;
    this->leds[i] = 0;
    this->ledsDuration[i] = 0;
    this->buttons[i] = true;
    this->potentiometers[i] = 0;
    this->potentiometersTemp[i] = 0;
    this->potentiometersReadings[i] = 0;
    this->encoders[i] = 0;
    this->encodersState[i] = 0;
    this->encodersSwitch[i] = true;
  }

}

/**
 * Singleton instance
 */
inline static Motherboard6 *Motherboard6::getInstance()    {
  if (!instance)
     instance = new Motherboard6;
  return instance;
}

/**
 * Init
 */
inline void Motherboard6::init(byte *inputs){
  // Init of the inputs
  for(byte i = 0; i < this->ioNumber; i++){
    this->inputs[i] = inputs[i];
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

  analogReadResolution(this->analogResolution);
  this->readMidiChannel();
  
  // Init sequence
  for(byte i = 0; i<this->ioNumber; i++){
    this->setDisplay(i, 1);
    this->iterateDisplay();
    this->updateDisplay();
    delay(50);
  }
  this->resetDisplay();
  this->updateDisplay();
}

/**
 * Update
 */
inline void Motherboard6::update(){
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
  }else{
    // Inputs

    // At the end of the clock we iterate to next input
    if (this->clockInputs >= this->intervalInputs) {
      this->iterateInputs();
      this->clockInputs = 0;
    }else{
      // Reading the current input
      this->readCurrentInput();
    }
  }

  // Debug
  if (this->clockDebug >= 100) {
    this->printInputs();
//    this->printLeds();
    this->clockDebug = 0;
  }
}


/**
 * Main multiplexer on LEDs
 */
inline void Motherboard6::setMainMuxOnLeds(){
  pinMode(22, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
}

/**
 * Main multiplexer on Potentiometers
 */
inline void Motherboard6::setMainMuxOnPots(){
  pinMode(22, INPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);
  digitalWrite(4, LOW);
}

/**
 * Main multiplexer on Encoders
 */
inline void Motherboard6::setMainMuxOnEncoders1(){
  pinMode(22, INPUT_PULLUP);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
}

/**
 * Main multiplexer on Encoder's switches
 */
inline void Motherboard6::setMainMuxOnEncoders2(){
  pinMode(22, INPUT_PULLUP);
  digitalWrite(2, HIGH);
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
}

/**
 * Main multiplexer on Channel
 */
inline void Motherboard6::setMainMuxOnChannel(){
  pinMode(22, INPUT_PULLUP);
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
}

/**
 * Iterate LEDs
 */
inline void Motherboard6::iterateDisplay(){
  // Iterating only on the acive leds
  // to save time between one led's iterations
  // and so improve brightness
  for(byte i = this->currentLed+1; i < this->currentLed+1 + this->ioNumber; i++){
    byte j = i % this->ioNumber;
    if(this->leds[j] > 0){
      this->currentLed = j;
      break;
    }
  }
}

/**
 * Refresh the leds
 */
inline void Motherboard6::updateDisplay(){
  this->setMainMuxOnLeds();
  
//  if(this->clockDisplay > this->intervalDisplay / 2
//  && this->clockDisplay < this->intervalDisplay / 1.75) {
    byte r0 = bitRead(this->currentLed, 0);   
    byte r1 = bitRead(this->currentLed, 1);    
    byte r2 = bitRead(this->currentLed, 2);
    digitalWrite(5, r0);
    digitalWrite(9, r1);
    digitalWrite(14, r2);
//  }

//    digitalWrite(22, HIGH);
    
//  if(this->clockDisplay > this->intervalDisplay / 1.75) {
    if(this->leds[this->currentLed] == 1){
      // Solid light
      digitalWrite(22, LOW);
    }else if(this->leds[this->currentLed] == 2){
      // Slow flashing
      digitalWrite(22, HIGH);
      if(this->clockDisplayFlash%400 > 200){
        digitalWrite(22, LOW);
      }
    }else if(this->leds[this->currentLed] == 3){
      // Fast flashing
      digitalWrite(22, HIGH);
      if(this->clockDisplayFlash%200 > 100){
        digitalWrite(22, LOW);
      }
    
    }else if(this->leds[this->currentLed] == 4){
      // Solid for 50 milliseconds
      digitalWrite(22, LOW);
      if(this->ledsDuration[this->currentLed]==0){
        this->ledsDuration[this->currentLed] = (clockDisplayFlash + 50)%intervalDisplayFlash;
      }
      
      if(this->clockDisplayFlash >= this->ledsDuration[this->currentLed]){
        digitalWrite(22, HIGH);
        this->leds[this->currentLed] = 0;
        this->ledsDuration[this->currentLed] = 0;
      }
    }else if(this->leds[this->currentLed] == 5){
      // Solid low birghtness
      if(clockDisplayFlash%20 > 16){
        digitalWrite(22, LOW);
      }
    }else{
      digitalWrite(22, HIGH);
    }
//  }
}


/**
 * Iterate over the inputs
 */
inline void Motherboard6::iterateInputs(){
  this->currentInput++;
  this->currentInput = this->currentInput % this->ioNumber;
}

/**
 * Read value of current inout
 */
inline void Motherboard6::readCurrentInput(){
  switch(this->inputs[this->currentInput]){
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
inline void Motherboard6::readButton(byte inputIndex){
  this->setMainMuxOnEncoders2();
  
  byte rowNumber = inputIndex / this->columnsNumber;

  for(byte i = 0; i < 3; i++){
    if(i == rowNumber){
      digitalWrite(15 + i, LOW);
    }else{
      digitalWrite(15 + i, HIGH);
    }
  }
  
  byte columnNumber = inputIndex % this->columnsNumber;

  byte r0 = bitRead(columnNumber, 0);   
  byte r1 = bitRead(columnNumber, 1);    
  byte r2 = bitRead(columnNumber, 2);
  digitalWrite(5, r0);
  digitalWrite(9, r1);
  digitalWrite(14, r2);

  this->buttons[inputIndex] = digitalRead(22);
}


/**
 * Get potentiometer value
 * @param byte inputeIndex The index of the input
 */
inline void Motherboard6::readPotentiometer(byte inputIndex){
  this->setMainMuxOnPots();
  
  byte r0 = bitRead(inputIndex, 0);   
  byte r1 = bitRead(inputIndex, 1);    
  byte r2 = bitRead(inputIndex, 2);
  digitalWrite(5, r0);
  digitalWrite(9, r1);
  digitalWrite(14, r2);

  this->potentiometersReadings[inputIndex] = this->potentiometersReadings[inputIndex] + 1;
  this->potentiometersTemp[inputIndex] += analogRead(22);
  
  if(this->potentiometersReadings[inputIndex] == 255){
    this->potentiometers[inputIndex] = this->potentiometersTemp[inputIndex] / 255; 
    this->potentiometers[inputIndex] = map(this->potentiometers[inputIndex], this->getAnalogMinValue(), this->getAnalogMaxValue(), 0, 1023);
    this->potentiometers[inputIndex] = constrain(this->potentiometers[inputIndex], 0, 1023);
    this->potentiometersReadings[inputIndex] = 0;
    this->potentiometersTemp[inputIndex] = 0;
  }
  
}

/**
 * Get encoder value
 * @param byte inputeIndex The index of the input
 */
inline void Motherboard6::readEncoder(byte inputIndex){
  // Activating the right row in the matrix
  byte rowNumber = inputIndex / this->columnsNumber;

  // Setting the main multiplexer on encoders
  if(this->clockInputs < this->intervalInputs / 10){
    for(byte i = 0; i < 3; i++){
      if(i == rowNumber){
        digitalWrite(15 + i, LOW);
      }else{
        digitalWrite(15 + i, HIGH);
      }
    }
    this->setMainMuxOnEncoders1();
  }

  // Getting right pin numbers on the multiplexer
  byte columnNumber = inputIndex % this->columnsNumber;
  byte muxPinA = columnNumber * 2;
  byte muxPinB = columnNumber * 2 + 1;

  // Giving time for the multiplexer to switch to Pin A 
  if(this->clockInputs > this->intervalInputs / 10
  && this->clockInputs < this->intervalInputs / 6) {
    byte r0 = bitRead(muxPinA, 0);   
    byte r1 = bitRead(muxPinA, 1);    
    byte r2 = bitRead(muxPinA, 2);
    digitalWrite(5, r0);
    digitalWrite(9, r1);
    digitalWrite(14, r2);

    this->currentEncPinA = digitalRead(22);
  }
  
   // Giving time for the multiplexer to switch to Pin B
  if(this->clockInputs > this->intervalInputs / 6
  && this->clockInputs < this->intervalInputs / 2){
    int r0 = bitRead(muxPinB, 0);   
    int r1 = bitRead(muxPinB, 1);    
    int r2 = bitRead(muxPinB, 2);
    digitalWrite(5, r0);
    digitalWrite(9, r1);
    digitalWrite(14, r2);
    
    this->currentEncPinB = digitalRead(22);
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
    } else if (result == DIR_CCW) {
      this->encoders[inputIndex]++;
    }

    // Setting the main multiplexer on encoder's buttons
    this->setMainMuxOnEncoders2();
    byte r0 = bitRead(columnNumber, 0);   
    byte r1 = bitRead(columnNumber, 1);    
    byte r2 = bitRead(columnNumber, 2);
    digitalWrite(5, r0);
    digitalWrite(9, r1);
    digitalWrite(14, r2);
  }

  // Giving time for the multiplexer to switch to Pin B
  if (this->clockInputs > this->intervalInputs / 1.5){
    this->encodersSwitch[inputIndex] = digitalRead(22);
  }
}

inline void Motherboard6::readMidiChannel(){
  this->setMainMuxOnChannel();
  delay(50); // Only because this function is used in Init only

  byte midiChannel = 0b00000000;
  for(byte i=0; i<4; i++){
    byte r0 = bitRead(i, 0);   
    byte r1 = bitRead(i, 1);    
    byte r2 = bitRead(i, 2);
    digitalWrite(5, r0);
    digitalWrite(9, r1);
    digitalWrite(14, r2);
    delay(5); // Only because this function is used in Init only
    byte channelBit = !digitalRead(22);
    bitWrite(midiChannel, i, channelBit);
  }
  this->midiChannel = midiChannel;
}

/**
 * Set a led status
 */
inline void Motherboard6::setDisplay(byte ledIndex, byte ledStatus){
  this->leds[ledIndex] = ledStatus;
}


/**
 * Reset LEDs
 */
inline void Motherboard6::resetDisplay(){
  for(byte i = 0; i < this->ioNumber; i++){
    if(this->leds[i] != 4){
      this->leds[i] = 0;
    }
  }
}


/**
 * Get input value
 * @param byte index The index of the input
 */
inline int Motherboard6::getInput(byte index){
  switch(this->inputs[index]){
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
inline int Motherboard6::getEncoderSwitch(byte index){
  return !this->encodersSwitch[index];
}

/**
 * Get max analog value according to resolution
 */
inline int Motherboard6::getAnalogMinValue(){
  return 0;
}

/**
 * Get max analog value according to resolution
 */
inline int Motherboard6::getAnalogMaxValue(){
  return (1 << this->analogResolution) - 1;
}

inline byte Motherboard6::getMidiChannel(){ 
  return this->midiChannel;
}

/**
 * Debug print
 */
inline void Motherboard6::printInputs(){
  Serial.println("Potentiometers:");
  for(byte j = 0; j < this->ioNumber; j++){
    Serial.print(this->potentiometers[j]);
    Serial.print(" ");
  }
  Serial.println("");
  
  Serial.println("Encoders:");
  for(byte j = 0; j < this->ioNumber; j++){
    Serial.print(this->encoders[j]);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println("Encoder switches:");
  for(byte j = 0; j < this->ioNumber; j++){
    Serial.print(this->encodersSwitch[j]);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println("Buttons:");
  for(byte j = 0; j < this->ioNumber; j++){
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
inline void Motherboard6::printLeds(){
  Serial.println("Leds:");
  for(byte j = 0; j < this->ioNumber; j++){
    Serial.print(this->leds[j]);
    Serial.print(" ");
  }
  Serial.println("");
}
#endif
