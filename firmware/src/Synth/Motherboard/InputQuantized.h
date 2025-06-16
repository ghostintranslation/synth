#ifndef InputQuantized_h
#define InputQuantized_h

#include "Registrar.h"
#include "Input.h"

class InputQuantized : public Input, public Registrar<InputQuantized>
{
public:
    InputQuantized(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    void onNoteChange(void (*onNoteCallback)(byte note));

protected:
    byte note;
    void (*onNoteCallback)(byte) = nullptr;
};

inline InputQuantized::InputQuantized(int8_t index) : Input(index)
{
    this->setLowPassCoeff(0.01);
}

inline void InputQuantized::update(void)
{
    Input::update();
}

inline int16_t *&InputQuantized::updateBefore(int16_t *&blockData)
{
    byte newNote = (int)((float)(blockData[0] + 32768) / 65535.0 * 12 * 5);

    if(newNote != this->note){
        if(this->onNoteCallback){
            this->onNoteCallback(newNote);
        }
    }
    this->note = newNote;

    return blockData;
}


inline void InputQuantized::onNoteChange(void (*onNoteCallback)(byte))
{
    this->onNoteCallback = onNoteCallback;
}
#endif
