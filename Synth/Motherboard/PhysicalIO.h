#ifndef PhysicalIO_h
#define PhysicalIO_h

#include "IO.h"

class PhysicalIO : public IO
{
public:
    PhysicalIO(unsigned int index, String name);
    unsigned int getIndex();

protected:
    // The index starting at 0
    unsigned int index = 0;
    
};

inline PhysicalIO::PhysicalIO(unsigned int index, String name):IO{name}
{
  this->index = index;
}

inline unsigned int PhysicalIO::getIndex()
{
    return this->index;
}

//#define PhysicalIO MotherboardNamespace::PhysicalIO

#endif
