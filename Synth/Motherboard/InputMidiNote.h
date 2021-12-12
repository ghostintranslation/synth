#ifndef InputMidiNote_h
#define InputMidiNote_h

#include "Registrar.h"
#include "VirtualInput.h"

using MidiNoteOnCallback = void (*)(byte channel, byte note, byte velocity);
using MidiNoteOffCallback = void (*)(byte channel, byte note, byte velocity);

class InputMidiNote : public VirtualInput, public Registrar<InputMidiNote>
{
public:
    InputMidiNote(String name, byte channel);
    byte getChannel();
    void onMidiNoteOn(byte note, byte velocity);
    void onMidiNoteOff(byte note, byte velocity);
    void setHandleMidiNoteOn(MidiNoteOnCallback fptr);
    void setHandleMidiNoteOff(MidiNoteOffCallback fptr);
    void update() override;

protected:
    // The MIDI channel starting at 0
    byte channel = 0;

    // Callbacks
    MidiNoteOnCallback midiNoteOnCallback = nullptr;
    MidiNoteOffCallback midiNoteOffCallback = nullptr;
};

inline InputMidiNote::InputMidiNote(String name, byte channel):VirtualInput{name}
{
  this->channel = channel;
  this->smoothing = 1;
}

inline byte InputMidiNote::getChannel()
{
    return this->channel;
}

inline void InputMidiNote::onMidiNoteOn(byte note, byte velocity){
  // Updating value
  this->value = note;
  this->target = note;
  
  // Triggering the callback
  if(this->midiNoteOnCallback != nullptr){
    this->midiNoteOnCallback(this->channel, note, velocity);
  }
}

inline void InputMidiNote::onMidiNoteOff(byte note, byte velocity){
  // Triggering the callback
  if(this->midiNoteOffCallback != nullptr){
    this->midiNoteOffCallback(this->channel, note, velocity);
  }
}

inline void InputMidiNote::setHandleMidiNoteOn(MidiNoteOnCallback fptr){
  this->midiNoteOnCallback = fptr;
}

inline void InputMidiNote::setHandleMidiNoteOff(MidiNoteOffCallback fptr){
  this->midiNoteOffCallback = fptr;
}


inline void InputMidiNote::update()
{ 
  audio_block_t *block;
  uint32_t *p, *end;
  
  block = this->allocateStream();
  if (block){
    p = (uint32_t *)(block->data);
    end = p + AUDIO_BLOCK_SAMPLES/2;
    do {
      *p++ = this->getValue();
      *p++ = this->getValue();
      *p++ = this->getValue();
      *p++ = this->getValue();
      *p++ = this->getValue();
      *p++ = this->getValue();
      *p++ = this->getValue();
      *p++ = this->getValue();
    } while (p < end);
    this->transmitStream(block);
    this->releaseStream(block);
  }
}
 
#define InputMidiNote MotherboardNamespace::InputMidiNote

#endif
