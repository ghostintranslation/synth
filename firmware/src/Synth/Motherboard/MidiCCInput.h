#ifndef MidiCCInput_h
#define MidiCCInput_h

#include "Setting.h"
#include "MidiInput.h"

// TODO: add callbacks

class MidiCCInput : public MidiInput, public Registrar<MidiCCInput>
{
public:
    MidiCCInput(Setting *ccSetting);
    MidiCCInput(byte control);

    void setMin(byte min);
    void setMax(byte max);
    byte getMin();
    byte getMax();
    byte getControl();
    static void handleMidiControlChange(byte channel, byte control, byte value);

private:
    byte min = 0;
    byte max = 127;
    byte control = 0;
};

inline MidiCCInput::MidiCCInput(Setting *ccSetting) : MidiInput(ccSetting)
{
    // TODO: this->control = ccSetting->getValue() ?
    // Could make things easier but would require a reboot after settings change
}

inline MidiCCInput::MidiCCInput(byte control) : MidiInput(nullptr)
{
    this->control = control;
}

inline void MidiCCInput::setMin(byte min)
{
    // Min has to be max - 1;
    if (min > 126)
    {
        min = 126;
    }

    this->min = min;
}

inline void MidiCCInput::setMax(byte max)
{

    if (max > 127)
    {
        max = 127;
    }

    this->max = max;
}

inline byte MidiCCInput::getMin()
{
    return this->min;
}

inline byte MidiCCInput::getMax()
{
    return this->max;
}

inline byte MidiCCInput::getControl()
{
    return this->control;
}

#endif