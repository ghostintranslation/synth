#ifndef None_h
#define None_h

#include "Input.h"
#include "Output.h"

class None : public Input, public Output
{
public:
  void read() override;
  bool needsGround() override;
  String getType() override;
};

inline void None::read()
{
}

inline bool None::needsGround()
{
  return false;
}

inline String None::getType()
{
  return "None";
}

// From now on "None" will be replaced by "new None()"
#define None new MotherboardNamespace::None()

#endif
