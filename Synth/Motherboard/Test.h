#ifndef Test_h
#define Test_h

#include "Registrar.h"

class Test : public Registrar<Test>
{
public:
  Test(String name);
  String getName();

    
private:
  String name = "";
};

inline Test::Test(String name)
{
  this->name = name;
}

inline String Test::getName()
{
  return this->name;
}

#endif
