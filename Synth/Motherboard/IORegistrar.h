#ifndef IORegistrar_h
#define IORegistrar_h

// Forward declarations
class PhysicalInput;
class PhysicalOutput;
class Led;

class IORegistrar
{

  public:
    static void registerInput(PhysicalInput* input);
    static void registerOutput(PhysicalOutput* output);
    static void registerLed(Led* led);
    static PhysicalInput** getInputs();
    static PhysicalOutput** getOutputs();
    static Led** getLeds();
    static unsigned int inputsSize;
    static unsigned int outputsSize;
    static unsigned int virtualInputsSize;
    static unsigned int virtualOutputsSize;
    static unsigned int midiInputsSize;
    static unsigned int midiOutputsSize;
    static unsigned int ledsSize;

  private:
    static PhysicalInput** inputs;
    static PhysicalOutput** outputs;
    static Led** leds;
};


// Must initialize
unsigned int IORegistrar::inputsSize = 0;
PhysicalInput** IORegistrar::inputs = new PhysicalInput*[0];
    
unsigned int IORegistrar::outputsSize = 0;
PhysicalOutput** IORegistrar::outputs = new PhysicalOutput*[0];
    
unsigned int IORegistrar::ledsSize = 0;
Led** IORegistrar::leds = new Led*[0];


inline void IORegistrar::registerInput(PhysicalInput* input){
  PhysicalInput** newArr = new PhysicalInput*[inputsSize + 1];
  std::copy(inputs, inputs + std::min(inputsSize, inputsSize + 1), newArr); // TODO: Not sure if this is better than the loop
//  for (int i = 0; i < inputsSize; i++)
//  {
//      newArr[i] = inputs[i];
//  }
  delete []inputs;
  inputs = newArr;
  newArr = NULL;

  inputs[inputsSize] = input;
  inputsSize++;
}

inline void IORegistrar::registerOutput(PhysicalOutput* output){
  PhysicalOutput** newArr = new PhysicalOutput*[outputsSize + 1];
  std::copy(outputs, outputs + std::min(outputsSize, outputsSize + 1), newArr);
  delete[] outputs;
  outputs = newArr;

  outputs[outputsSize] = output;
  
  outputsSize++; 
}

inline void IORegistrar::registerLed(Led* led){
  Led** newArr = new Led*[ledsSize + 1];
  std::copy(leds, leds + std::min(ledsSize, ledsSize + 1), newArr);
  delete[] leds;
  leds = newArr;

  leds[ledsSize] = led;
  
  ledsSize++; 
}

inline PhysicalInput** IORegistrar::getInputs(){
  return inputs;
};

inline PhysicalOutput** IORegistrar::getOutputs(){
  return outputs;
};

inline Led** IORegistrar::getLeds(){
  return leds;
};

#endif
