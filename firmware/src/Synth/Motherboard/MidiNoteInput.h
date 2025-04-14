#ifndef MidiNoteInput_h
#define MidiNoteInput_h

#include "Setting.h"
#include "MidiInput.h"

// TODO: add callbacks

class MidiNoteInput : public MidiInput, public Registrar<MidiNoteInput>
{
public:
    MidiNoteInput(Setting *noteSetting);
    MidiNoteInput(byte note);
    void setMin(byte min);
    void setMax(byte max);
    byte getMin();
    byte getMax();
    byte getNote();

private:
    byte min = 0;
    byte max = 127;
    byte note = 0;
};

inline MidiNoteInput::MidiNoteInput(Setting *setting) : MidiInput(setting)
{
    // TODO: this->note = noteSetting->getValue() ?
    // Could make things easier but would require a reboot after settings change
}

inline MidiNoteInput::MidiNoteInput(byte note) : MidiInput(nullptr)
{
    this->note = note;
}

inline void MidiNoteInput::setMin(byte min)
{
    // Min has to be max - 1;
    if (min > 126)
    {
        min = 126;
    }

    this->min = min;
}

inline void MidiNoteInput::setMax(byte max)
{

    if (max > 127)
    {
        max = 127;
    }

    this->max = max;
}

inline byte MidiNoteInput::getMin()
{
    return this->min;
}

inline byte MidiNoteInput::getMax()
{
    return this->max;
}

inline byte MidiNoteInput::getNote()
{
    return this->note;
}

#endif