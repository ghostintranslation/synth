#ifndef MidiNotesInput_h
#define MidiNotesInput_h

// #include "Setting.h"
#include "MidiInput.h"

// TODO: add callbacks

class MidiNotesInput : public MidiInput, public Registrar<MidiNotesInput>
{
public:
    MidiNotesInput();
    void noteOn(byte note, byte velocity);
    void noteOff(byte note);
    void onNoteOn(void (*onNoteOnCallback)(byte note, byte velocity));
    void onNoteOff(void (*onNoteOffCallback)(byte note));

private:
    void (*onNoteOnCallback)(byte note, byte velocity) = nullptr;
    void (*onNoteOffCallback)(byte note) = nullptr;
};

inline MidiNotesInput::MidiNotesInput() : MidiInput(nullptr)
{
}

inline void MidiNotesInput::noteOn(byte note, byte velocity){
    if(this->onNoteOnCallback){
        this->onNoteOnCallback(note, velocity);
    }
}
inline void MidiNotesInput::noteOff(byte note){
    if(this->onNoteOffCallback){
        this->onNoteOffCallback(note);
    }
}

inline void MidiNotesInput::onNoteOn(void (*onNoteOnCallback)(byte, byte))
{
    this->onNoteOnCallback = onNoteOnCallback;
}

inline void MidiNotesInput::onNoteOff(void (*onNoteOffCallback)(byte))
{
    this->onNoteOffCallback = onNoteOffCallback;
}
#endif