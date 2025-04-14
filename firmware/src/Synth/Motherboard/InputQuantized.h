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
    byte cycleCount = 0;
    void (*onNoteCallback)(byte) = nullptr;
};

inline InputQuantized::InputQuantized(int8_t index) : Input(index)
{
    this->setLowPassCoeff(0.1);
}

inline void InputQuantized::update(void)
{
    Input::update();
}

inline int16_t *&InputQuantized::updateBefore(int16_t *&blockData)
{
    // byte newNote = 0;
    // for(int i=0; i<AUDIO_BLOCK_SAMPLES;i++){
    //     byte n = (int)((float)((blockData[i] >> 9) + 32) / 64.0 * 12 * 5);
    //     newNote = (float)n * 0.1 + (float)this->note * 0.9;
    // }
    
    // cycleCount++;
    // if(cycleCount>2){
        cycleCount = 0;
        // byte newNote = (int)((float)((blockData[0] >> 10) + 32) / 64.0 * 12 * 5);
        byte newNote = (int)((float)(blockData[0] + 32768) / 65536.0 * 12 * 5);

        if(newNote != this->note){
            Serial.println(newNote);
            if(this->onNoteCallback){
                this->onNoteCallback(newNote);
            }
        }
        this->note = newNote;
        
    // }

    return blockData;
}


inline void InputQuantized::onNoteChange(void (*onNoteCallback)(byte))
{
    this->onNoteCallback = onNoteCallback;
}
#endif
