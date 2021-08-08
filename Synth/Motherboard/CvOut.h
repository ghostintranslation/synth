#ifndef CvOut_h
#define CvOut_h

#include "Output.h"

class CvOut : public Output
{
public:
  String getType() override;
};

inline String CvOut::getType()
{
  return "CvOut";
}

// From now on "CvOut" will be replaced by "new CvOut()"
#define CvOut new MotherboardNamespace::CvOut()
#endif
