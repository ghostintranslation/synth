#ifndef Input_h
#define Input_h

#include "IO.h"

class Input : public IO
{
public:
    // Depending on the type of input, the way to read it will difer
    virtual void read() = 0;

    // Depending on the type of input, it will need to be connected to ground or not
    virtual bool needsGround() = 0;

    virtual void setPressDownCallback(PressDownCallback pressDownCallback){};
    virtual void setLongPressDownCallback(LongPressDownCallback longPressDownCallback){};
    virtual void setPressUpCallback(PressUpCallback pressUpCallback){};
    virtual void setLongPressUpCallback(LongPressUpCallback longPressUpCallback){};

    void setPin(byte pin);

    // All inputs have a change callback, but not all inputs have press callbacks
    void setChangeCallback(ChangeCallback changeCallback);

    // On value change callback
    void onValueChange() override;

protected:
    // The pin on which the input can be read
    byte pin;

private:
    // Change callback function
    ChangeCallback changeCallback;
};

/**
 * On value change call the callback.
 * This function is called in the IO class.
 */
inline void Input::onValueChange()
{
    if (this->changeCallback != nullptr)
    {
        this->changeCallback(this->index, this->value, this->value - this->previousValue);
    }
}

/**
 * Set the callback function to call when the value changes
 */
inline void Input::setChangeCallback(ChangeCallback fptr)
{
    this->changeCallback = fptr;
}

inline void Input::setPin(byte pin)
{
    this->pin = pin;
}
#endif
