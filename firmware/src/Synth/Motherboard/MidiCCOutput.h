#ifndef MidiCCOutput_h
#define MidiCCOutput_h

#include "Setting.h"
#include "MidiOutput.h"

class MidiCCOutput : public MidiOutput, public Registrar<MidiCCOutput>
{
public:
    MidiCCOutput(Setting *ccSetting);
    MidiCCOutput(byte control);
    void setMin(byte min);
    void setMax(byte max);
    byte getMin();
    byte getMax();
    byte getControl();
    void send();

private:
    byte min = 0;
    byte max = 127;
    byte control = 0;
};

inline MidiCCOutput::MidiCCOutput(Setting *ccSetting) : MidiOutput(ccSetting)
{
    // TODO: this->control = ccSetting->getValue() ?
    // Could make things easier but would require a reboot after settings change
}

inline MidiCCOutput::MidiCCOutput(byte control) : MidiOutput(nullptr)
{
    this->control = control;
}

inline void MidiCCOutput::setMin(byte min)
{
    // Min has to be max - 1;
    if (min > 126)
    {
        min = 126;
    }

    this->min = min;
}

inline void MidiCCOutput::setMax(byte max)
{

    if (max > 127)
    {
        max = 127;
    }

    this->max = max;
}

inline byte MidiCCOutput::getMin()
{
    return this->min;
}

inline byte MidiCCOutput::getMax()
{
    return this->max;
}

inline byte MidiCCOutput::getControl()
{
    return this->control;
}

inline void MidiCCOutput::send()
{
   usbMIDI.sendControlChange(this->control, this->value, 1);
   MIDI.sendControlChange(this->control, this->value, 1);
}

#endif