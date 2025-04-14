#ifndef InputGate_h
#define InputGate_h

#include "Registrar.h"
#include "Input.h"

class InputGate : public Input, public Registrar<InputGate>
{
public:
    InputGate(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    void setThresholds(int16_t openThreshold, int16_t closeThreshold);
    void onOpen(void (*onOpenCallback)());
    void onClose(void (*onCloseCallback)());

protected:
    uint8_t index;
    int16_t openThreshold = 100;
    int16_t closeThreshold = -100;
    bool isOpen = false;
    void (*onOpenCallback)() = nullptr;
    void (*onCloseCallback)() = nullptr;
};

inline InputGate::InputGate(int8_t index) : Input(index)
{
}

inline void InputGate::update(void)
{
    Input::update();
}

inline int16_t *&InputGate::updateBefore(int16_t *&blockData)
{
    bool newIsOpen = false;
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        if (blockData[i] > openThreshold)
        {
            blockData[i] = INT16_MAX;
            newIsOpen = true;
        }
        else if (blockData[i] < closeThreshold)
        {
            blockData[i] = INT16_MIN;
        }
    }

    // Calling callback
    if(!this->isOpen && newIsOpen){
        if(this->onOpenCallback){
            this->onOpenCallback();
        }
    }else if(this->isOpen && !newIsOpen){
        if(this->onCloseCallback){
            this->onCloseCallback();
        }
    }

    this->isOpen = newIsOpen;

    return blockData;
}

inline void InputGate::setThresholds(int16_t openThreshold, int16_t closeThreshold)
{
    this->openThreshold = openThreshold;
    this->closeThreshold = closeThreshold;
}

inline void InputGate::onOpen(void (*onOpenCallback)())
{
    this->onOpenCallback = onOpenCallback;
}

inline void InputGate::onClose(void (*onCloseCallback)())
{
    this->onCloseCallback = onCloseCallback;
}

#endif
