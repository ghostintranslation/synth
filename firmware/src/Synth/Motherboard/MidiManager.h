#ifndef MidiManager_h
#define MidiManager_h

#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI); // MIDI library init

#include "MidiCCInput.h"
#include "MidiNoteInput.h"
#include "MidiNotesInput.h"
#include "MidiCCOutput.h"

class MidiManager : public AudioStream
{
public:
    static MidiManager *getInstance();
    void update(void);

private:
    static MidiManager *instance;
    MidiManager();
    static void handleMidiControlChange(byte channel, byte control, byte value);
    static void handleMidiNoteOn(byte channel, byte control, byte velocity);
    static void handleMidiNoteOff(byte channel, byte control, byte velocity);
};

// Singleton pre init
MidiManager *MidiManager::instance = nullptr;

inline MidiManager::MidiManager() : AudioStream(0, NULL)
{
    this->active = true;

    // MIDI init
    MIDI.begin(MIDI_CHANNEL_OMNI);
    Serial1.begin(31250, SERIAL_8N1_TXINV);

    // Callbacks
    MIDI.setHandleControlChange(handleMidiControlChange);
    usbMIDI.setHandleControlChange(handleMidiControlChange);
    MIDI.setHandleNoteOn(handleMidiNoteOn);
    usbMIDI.setHandleNoteOn(handleMidiNoteOn);
    MIDI.setHandleNoteOff(handleMidiNoteOff);
    usbMIDI.setHandleNoteOff(handleMidiNoteOff);
}

/**
   Singleton instance
*/
inline MidiManager *MidiManager::getInstance()
{
    if (!instance)
        instance = new MidiManager;
    return instance;
}

inline void MidiManager::update(void)
{
    usbMIDI.read();
    MIDI.read();
}

inline void MidiManager::handleMidiControlChange(byte channel, byte control, byte value)
{

    for (unsigned int i = 0; i < MidiCCInput::getCount(); i++)
    {
        MidiCCInput *midiInput = MidiCCInput::get(i);
        byte mappedValue = map(value, 0, 127, midiInput->getMin(), midiInput->getMax());

        if (midiInput->getSetting() != nullptr)
        {
            if ((!isnan(midiInput->getSetting()->getValue()) && (byte)midiInput->getSetting()->getValue() == control) || (isnan(midiInput->getSetting()->getValue()) && (byte)midiInput->getSetting()->getDefaultValue() == control))
            {
                midiInput->setValue(mappedValue);
            }
        }
        else if (control == midiInput->getControl())
        {
            midiInput->setValue(mappedValue);
        }
    }
}

inline void MidiManager::handleMidiNoteOn(byte channel, byte note, byte velocity)
{
    // MidiNoteInput reacts to 1 note and value is the velocity
    for (unsigned int i = 0; i < MidiNoteInput::getCount(); i++)
    {
        MidiNoteInput *midiInput = MidiNoteInput::get(i);
        byte mappedValue = map(velocity, 0, 127, midiInput->getMin(), midiInput->getMax());

        if (midiInput->getSetting() != nullptr)
        {
            if ((!isnan(midiInput->getSetting()->getValue()) && (byte)midiInput->getSetting()->getValue() == note) || (isnan(midiInput->getSetting()->getValue()) && (byte)midiInput->getSetting()->getDefaultValue() == note))
            {
                midiInput->setValue(mappedValue);
            }
        }
        else if (note == midiInput->getNote())
        {
            midiInput->setValue(mappedValue);
        }
    }

    // MidiNotesInput reacts to all notes and value is the note
    for (unsigned int i = 0; i < MidiNotesInput::getCount(); i++)
    {
        MidiNotesInput *midiInput = MidiNotesInput::get(i);
        midiInput->setValue(note);
        if(velocity > 0){
            midiInput->noteOn(note, velocity);
        }else{
            midiInput->noteOff(note);
        }
    }
}

inline void MidiManager::handleMidiNoteOff(byte channel, byte note, byte velocity)
{
    MidiManager::handleMidiNoteOn(channel, note, 0);
}

#endif