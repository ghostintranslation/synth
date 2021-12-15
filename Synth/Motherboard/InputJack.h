#ifndef InputJack_h
#define InputJack_h

#include "InputPotentiometer.h"

class InputJack : public InputPotentiometer
{
public:
  InputJack(int index, String name);
};

inline InputJack::InputJack(int index, String name):InputPotentiometer{index, name}{
  this->max = 4025;
}

// Prevents to have to write the namespace when using this class
#define InputJack MotherboardNamespace::InputJack

#endif
