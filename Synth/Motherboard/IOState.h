#ifndef IOState_h
#define IOState_h

class IO; // Forward declaration

/**
 * State pattern
 */
class IOState
{
protected:
  IO* io;
  
public:
    virtual void update() = 0;
    
    void setIO(IO* io);
};

inline void IOState::setIO(IO* io){
  this->io = io;
}
#endif
