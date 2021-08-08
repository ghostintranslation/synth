#ifndef Button_h
#define Button_h

#include "Input.h"

class Button : public Input
{
public:
    void read() override;
    bool needsGround() override;
    void setPressDownCallback(PressDownCallback pressDownCallback) override;
    void setLongPressDownCallback(LongPressDownCallback longPressDownCallback) override;
    void setPressUpCallback(PressUpCallback pressUpCallback) override;
    void setLongPressUpCallback(LongPressUpCallback longPressUpCallback) override;
    String getType() override;

private:
    // Time counter for long vs short press
    elapsedMillis pressTime;

    // Has the long press down callback been fired already?
    bool longPressDownFired;

    // Callbacks functions
    PressDownCallback pressDownCallback;
    LongPressDownCallback longPressDownCallback;
    PressUpCallback pressUpCallback;
    LongPressUpCallback longPressUpCallback;
};

inline void Button::read()
{
    // Serial.print("Button: ");
    // Serial.println(this->needsGround());

    pinMode(this->pin, INPUT_PULLUP);
    int newReading = !digitalRead(this->pin);
    
//    Serial.println(newReading);
//    this->value = newReading;
//    this->target = newReading;

    if (this->pressDownCallback != nullptr ||
        this->pressUpCallback != nullptr ||
        this->longPressDownCallback != nullptr ||
        this->longPressUpCallback != nullptr)
    {

        // Inverted logic, 0 = button pushed
        // If previous value is not pushed and now is pushed
        // So if it's pushed
        if (this->value && !newReading)
        {
            // Start the counter of that input
            this->pressTime = 0;
            this->longPressDownFired = false;

            // If there is a short press down callback on that input, and there is no Long Press down
            if (this->longPressDownCallback == nullptr &&
                this->pressDownCallback != nullptr)
            {
                this->pressDownCallback(this->index);
            }
        }

        // If it stayed pressed for 200ms and Long Press Down callback hasn't been fired yet
        if (!this->value && !newReading)
        {
            if (this->pressTime >= 200 && !this->longPressDownFired)
            {

                if (this->longPressDownCallback != nullptr)
                {
                    // Fire the callback
                    this->longPressDownCallback(this->index);
                    this->longPressDownFired = true;
                }
            }
        }

        // If it's released
        if (!this->value && newReading)
        {
            // How long was it pressed
            if (this->pressTime < 200)
            {
                // Short press

                // If there is a short press callback on that input
                if (this->pressUpCallback != nullptr)
                {
                    this->pressUpCallback(this->index);
                }
            }
            else
            {
                // Long press

                // If there is a long press callback on that input
                if (this->longPressUpCallback != nullptr)
                {
                    this->longPressUpCallback(this->index);
                }
                else if (this->pressUpCallback != nullptr)
                {
                    // If the input was pressed for a long time but there is only a short press callback
                    // the short press callback should still be called
                    this->pressUpCallback(this->index);
                }
            }
        }
    }

    this->setTarget(newReading);
}

inline bool Button::needsGround()
{
    return false;
}

/**
 * Set the press down callback
 */
inline void Button::setPressDownCallback(PressDownCallback fptr)
{
    this->pressDownCallback = fptr;
}

/**
 * Set the long press down callback
 */
inline void Button::setLongPressDownCallback(LongPressDownCallback fptr)
{
    this->longPressDownCallback = fptr;
}

/**
 * Set the press up callback
 */
inline void Button::setPressUpCallback(PressUpCallback fptr)
{
    this->pressUpCallback = fptr;
}

/**
 * Set the long press up callback
 */
inline void Button::setLongPressUpCallback(LongPressUpCallback fptr)
{
    this->longPressUpCallback = fptr;
}

inline String Button::getType()
{
    return "Button";
}

// From now on "Button" will be replaced by "new Button()"
#define Button new MotherboardNamespace::Button

#endif
