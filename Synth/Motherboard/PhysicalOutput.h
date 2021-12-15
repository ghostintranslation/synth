#ifndef PhysicalOutput_h
#define PhysicalOutput_h

#include "Registrar.h"
#include "PhysicalIO.h"

// TODO: SET THE NUMBER OF STREAM OUTPUTS to 0
class PhysicalOutput : public PhysicalIO, public Registrar<PhysicalOutput>
{
public:
    //    virtual void write(); // TODO: NEED?
    PhysicalOutput(int index, String name);

    void update() override;
};

inline PhysicalOutput::PhysicalOutput(int index, String name):PhysicalIO{index, name}
{
//  IORegistrar::registerOutput(this);
}

inline void PhysicalOutput::update(){
  IO::update();
}

#endif
