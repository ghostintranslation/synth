#ifndef OutputJack_h
#define OutputJack_h

#include "PhysicalOutput.h"

class OutputJack : public PhysicalOutput
{

public:
  OutputJack(unsigned int index, String name);
  
  String getClassName() override {return "OutputJack";}
};

inline OutputJack::OutputJack(unsigned int index, String name):PhysicalOutput{index, name}{
}

//#define OutputJack MotherboardNamespace::OutputJack
#endif
